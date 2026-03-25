# Multi VMs Memory Sweep Agent

## Goal
Implement and maintain LKVS memory sweep cases for `KVM/qemu` with behavior aligned to legacy `vmm_tree` memory cases and stable cfg compatibility.

## Scope
- `KVM/qemu/boot_repeat.cfg`
- `KVM/qemu/multi_vms.cfg`
- `KVM/qemu/tests/multi_vms_multi_boot.py`

## Current Requirement Summary (Current Codebase)
1. `multi_vms.cfg` is baseline functional matrix only and currently has no memory-sweep variants.
2. Active memory-sweep variants are currently centralized in `boot_repeat.cfg` under:
   - `boot_repeat.one_vm_repeat.memory_sweep`
   - `boot_repeat.multi_vms_repeat.memory_sweep`
3. Current sweep naming in cfg is `from1G_toall` (both VM and TD branches).
4. Sweep execution uses `type = multi_vms_multi_boot`.
5. Current sweep generator parameters in cfg:
   - `mem_generator = random_32g_window`
   - `start_mem = 1024`
   - `random_min = 0`, `random_max = 64`
   - `random_unit = 511`
   - `samples_per_window = 2`
   - `window_size = 32768`
   - `divide_host_mem_limit_by_vm_count = yes` for multi-vm sweep matrix

## Python Rules for `multi_vms_multi_boot.py`
- Keep entrypoint `run(test, params, env)` and `@error_context.context_aware`.
- Keep `from provider import dmesg_router  # pylint: disable=unused-import` as side-effect import.
- Keep imports aligned with current code (`logging`, `random`, `env_process`, `error_context`, `utils_misc`, `dmesg_router`).
- Use `utils_misc.get_usable_memory_size()` as host memory source.
- Keep generator support aligned with implementation:
  - `random_32g_window` (active)
  - `linear` fallback (currently supported in code)
- Use `env_process.preprocess_vm(...)` and `params.object_params(vm_name)`.
- Current implementation applies same per-iteration `mem` to all VMs in the iteration.
- Always clean up in `finally` with `vm.destroy(gracefully=False)`.
- Keep concise iteration logs (`Iteration X/Y` with effective mem values).

## CFG Rules
- Preserve indentation and kartograph structure.
- Do not remove or rename historical baseline variants in `multi_vms.cfg` unless explicitly requested.
- For sweep work, prefer changing `boot_repeat.cfg` first because sweep is currently hosted there.
- Keep sweep parameters explicit in cfg (generator type, random range/unit, start_mem, and host-memory divide policy).

## Review Checklist
- No cfg nesting or indentation breakage.
- No unexpected movement of sweep variants back into `multi_vms.cfg` unless explicitly requested.
- Script and cfg are aligned on generator mode and parameters.
- No lint/compile errors in touched Python files.
- `grep` checks are clean and match expectation:
   - `from1G_toall` present in sweep branches
   - `mem_generator = random_32g_window` in active sweep variants
   - no accidental unsupported generator names in active cfg

## Known Pitfalls
- Using legacy patch content directly may introduce malformed imports or stale logic.
- Mixing `MemAvailable` and `MemFree` semantics causes inconsistent sweep range.
- Updating only docs without checking active cfg host file (`boot_repeat.cfg` vs `multi_vms.cfg`) causes drift.
- Case naming drift (`from...`) and actual `start_mem` mismatch causes confusion.

## Commit Message Template
KVM/qemu: keep memory sweep aligned with boot_repeat matrix

Update `boot_repeat.cfg` and/or `multi_vms_multi_boot.py` to keep active
memory-sweep cases (`from1G_toall`) aligned with current random window
generator behavior and host memory limit policy.

Signed-off-by: Fan Chen <fan.chen@intel.com>

## Current Baseline Notes
- Active sweep behavior is currently cfg-driven by `boot_repeat.cfg`.
- `multi_vms.cfg` remains baseline functional matrix without sweep cases.
- `multi_vms_multi_boot.py` currently supports random-window flow plus linear fallback.
