# GitHub Copilot Instructions for LKVS

## Project Overview

LKVS (Linux Kernel Validation Suite) is an Intel open-source project for testing Linux kernel features on x86 platforms. It contains two major test areas:

- **BM/**: Bare-metal feature tests (C/shell scripts, test by feature directory)
- **KVM/**: Virtualization tests using avocado-vt + QEMU framework

## License and Copyright

- License: GPL-2.0-only
- All new files must include the SPDX header and Intel copyright:
  ```python
  # SPDX-License-Identifier: GPL-2.0-only
  # Copyright (c) <year> Intel Corporation
  ```
- For KVM Python tests, keep the existing `#!/usr/bin/python3` shebang style when adding new executable test files.

## Coding Style

### Python (KVM tests)
- Follow avocado-vt conventions: `run(test, params, env)` entry point
- Use `@error_context.context_aware` decorator on `run()`
- Use `env_process.preprocess_vm()` before `env.get_vm()` when overriding VM params
- Use `params.object_params(vm_name)` to get per-VM parameter dict
- Use `test.fail()` for test failures, `test.cancel()` for skips
- Use `error_context.context("...", test.log.info)` for step logging
- Import order: match local file style first; commonly provider imports (including side-effect imports such as `dmesg_router`) appear before virttest imports
- Private helpers should be prefixed with `_`

### Shell scripts
- Follow Linux kernel coding style
- Use Shellcheck-compatible syntax
- Print format: `[PASS|FAIL|INFO|SKIP] case_name: message`

### CFG files (avocado-vt kartograph format)
- Indent with 4 spaces
- No trailing whitespace
- Use `variants:` blocks to compose test matrix
- Use `no <variant_name>` to exclude specific variants from a case
- Independent test cases should be at the same level as `@default`, not inside it, to avoid unintended cross-product combinations

## KVM Test Structure

```
KVM/qemu/
  *.cfg          # Test configuration files (top-level in qemu/)
  tests/         # Python test scripts
  provider/      # Shared utilities
  hooks/         # avocado hooks (optional)
```

### CFG conventions
- Each `.cfg` file corresponds to a test feature
- Top-level variants: `one_vm_repeat`, `multi_vms_repeat`, etc.
- VM types: `vm` (normal VM), `td` (TDX confidential VM)
- TDX VMs require: `machine_type_extra_params = "kernel-irqchip=split"` and `vm_secure_guest_type = tdx`

### Adding a TD version for an existing VM case
- Default approach: keep the same Python `type = <handler>` and add a TD cfg variant unless the test logic is explicitly incompatible with TDX.
- Match the local cfg naming style instead of inventing a new one:
  - single-guest cases often use `vm` and `tdvm` or `td`
  - count-based matrices often use `1vm` and `1td`
  - mixed topologies use explicit combinations such as `1td_1vm`
- Keep the VM case payload unchanged where possible and add only the TDX-specific delta in the TD branch.
- For a TD branch, set at least:
  ```
  machine_type_extra_params = "kernel-irqchip=split"
  vm_secure_guest_type = tdx
  ```
- When the surrounding file already pins TDX CPU handling, preserve the same pattern in the new TD branch, commonly:
  ```
  auto_cpu_model = "no"
  cpu_model = host
  ```
- For mixed VM/TD cases, use per-VM suffix parameters instead of changing shared params for all guests. Example:
  ```
  machine_type_extra_params_vm2 = "kernel-irqchip=split"
  vm_secure_guest_type_vm2 = tdx
  ```
- Preserve the original case semantics: same iterations, memory, vcpu layout, and handler routing unless the TD path requires an explicit limitation.
- If the TD path is unsupported for a subcase, keep the VM case intact and exclude or constrain only the TD variant with a clear cfg note.
- Prefer cfg-only extension first. Only change Python when the handler truly needs TDX-specific setup, validation, or skip logic.

### TD extension review checklist
- Confirm the new TD variant stays in the same cfg family as the VM source case.
- Confirm the handler already works with `vm_secure_guest_type = tdx`; if not, add the smallest required Python change.
- Confirm all TD-specific params are applied at the correct scope: global for all guests, or suffixed per VM for mixed topologies.
- Confirm any VM-only assumptions in the test are still valid for TD, especially CPU model handling, guest boot flow, and unsupported feature checks.
- Confirm naming stays aligned with nearby LKVS cases so VM and TD variants are easy to pair during review.

### Test Python conventions
- Entry: `def run(test, params, env):`
- Always check required params exist before use
- Always destroy VMs in `finally` block: `vm.destroy(gracefully=False)`
- Use `params.get()` with defaults, `params.get_numeric()` for numbers, `params.objects()` for lists
- Keep VM lifecycle consistent across loops/iterations: collect created VMs and clean all of them in `finally`
- When dynamically setting VM memory or other params before boot:
  ```python
  vm_params = params.object_params(vm_name)
  vm_params["mem"] = mem_size
  env_process.preprocess_vm(test, vm_params, env, vm_name)
  vm = env.get_vm(vm_name)
  ```
- Prefer per-VM suffix params in cfg (for example `machine_type_extra_params_vm2`) when mixed VM types are needed in one case.
- When extending a VM test to TD, keep the handler guest-type-agnostic where possible and branch only on true TDX-specific requirements.

## Git Commit Message Format

```
<scope>: <short summary>

<optional body>

Signed-off-by: Name <email>
```

- Scope examples: `boot_repeat`, `KVM/qemu`, `BM/tdx-guest`
- Summary: concise, imperative, lowercase after scope
- Always include `Signed-off-by` (use `git commit -s`)
- Amend commits only when the user explicitly requests it (for example, `git commit --amend --no-edit` while fixing up the same logical change)

## Common Pitfalls

- Do not set `params['mem']` directly - it won't affect VM boot. Use `params.object_params()` + `env_process.preprocess_vm()`.
- Do not place feature-specific test variants inside `@default` variants block if they should not combine with timing/smp/socket matrix.
- `test.fail()` raises an exception and stops execution; use it to report hard failures with descriptive messages.
- Do not remove side-effect imports like `from provider import dmesg_router  # pylint: disable=unused-import` unless you verified logging routing behavior is unchanged.
- Do not create a separate Python test just because a VM case needs a TD variant; in LKVS this is usually a cfg split on top of the same handler.
- Do not apply TDX parameters globally when only one guest in a mixed topology should be TD; use suffixed params such as `vm_secure_guest_type_vm2`.
- Do not rename the VM branch while adding TD support unless the surrounding cfg already uses paired naming that requires it.
