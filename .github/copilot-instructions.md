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
