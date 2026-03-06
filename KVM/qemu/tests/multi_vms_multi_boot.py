#!/usr/bin/python3

# SPDX-License-Identifier: GPL-2.0-only
# Copyright (c) 2026 Intel Corporation

import random

from provider import dmesg_router  # pylint: disable=unused-import
from virttest import env_process
from virttest import error_context


def _host_available_mem_mb():
    with open("/proc/meminfo", "r", encoding="utf-8") as meminfo:
        for line in meminfo:
            if line.startswith("MemAvailable:"):
                return int(line.split()[1]) // 1024
    raise RuntimeError("Cannot read host available memory from /proc/meminfo")


def _build_iteration_mem_plan(params, vm_names):
    mem_generator = params.get("mem_generator", "random_32g_window")
    start_mem = params.get_numeric("start_mem", 512)
    vm_mem_step = params.get_numeric("vm_mem_step", 0)
    use_half_host_mem_limit = params.get("use_half_host_mem_limit", "yes") == "yes"
    if vm_mem_step < 0:
        raise ValueError("vm_mem_step must be >= 0")

    host_available_mem = _host_available_mem_mb()
    memory_limit = host_available_mem
    if use_half_host_mem_limit:
        memory_limit = host_available_mem // max(1, len(vm_names))

    if start_mem > memory_limit:
        return []

    if mem_generator != "random_32g_window":
        raise ValueError(
            "Unsupported mem_generator '%s'. Only 'random_32g_window' is supported"
            % mem_generator
        )

    random_min = params.get_numeric("random_min", 0)
    random_max = params.get_numeric("random_max", 64)
    random_unit = params.get_numeric("random_unit", 511)
    samples_per_window = params.get_numeric("samples_per_window", 2)
    window_size = params.get_numeric("window_size", 32768)
    random_seed = params.get("random_seed")
    if random_seed is not None:
        random.seed(int(random_seed))

    plan = []
    base = start_mem
    while True:
        for _ in range(int(samples_per_window)):
            base_mem = int(base + random.randint(int(random_min), int(random_max)) * int(random_unit))
            # tp8 semantics: once the sampled value is out of limit, stop trying
            # the current window and move to the next 32G window.
            if base_mem > memory_limit:
                break

            vm_mem_map = {}
            for index, vm_name in enumerate(vm_names):
                vm_mem_map[vm_name] = int(base_mem + index * vm_mem_step)
            if max(vm_mem_map.values()) <= memory_limit:
                plan.append(vm_mem_map)
            else:
                break

        base += int(window_size)
        if base > memory_limit:
            break

    return plan


@error_context.context_aware
def run(test, params, env):
    """
    Boot multiple VMs with per-iteration parameter overrides:
    1) Boot all VMs with current iteration parameters
    2) Verify all guests can login
    3) Destroy all VMs
    4) Repeat for all iterations

    :param test: QEMU test object
    :param params: Dictionary with the test parameters
    :param env: Dictionary with test environment
    """

    timeout = params.get_numeric("login_timeout", 240)
    serial_login = params.get("serial_login", "no") == "yes"

    vm_names = params.objects("vms")
    if not vm_names:
        test.cancel("No VMs configured for multi_vms_multi_boot")

    try:
        mem_plan = _build_iteration_mem_plan(params, vm_names)
    except ValueError as error:
        test.cancel(str(error))

    if not mem_plan:
        test.cancel("No valid iterations resolved for multi_vms_multi_boot")

    for iteration, vm_mem_map in enumerate(mem_plan, start=1):
        started_vms = []
        try:
            error_context.context(
                "Iteration %s: boot %s VMs with stepped memory values"
                % (iteration, len(vm_names)),
                test.log.info,
            )

            for vm_name in vm_names:
                vm_params = params.object_params(vm_name)
                vm_params["start_vm"] = "yes"
                vm_params["mem"] = str(vm_mem_map[vm_name])
                env_process.preprocess_vm(test, vm_params, env, vm_name)
                started_vms.append(env.get_vm(vm_name))

            for vm in started_vms:
                vm.verify_alive()
                if serial_login:
                    session = vm.wait_for_serial_login(timeout=timeout)
                else:
                    session = vm.wait_for_login(timeout=timeout)
                session.close()
        finally:
            for vm in started_vms:
                vm.destroy(gracefully=False)
