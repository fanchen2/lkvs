# Boot Repeat Case Refactor Agent

## Goal
Implement and maintain `boot_repeat`-related cfg/test refactors in `KVM/qemu` with stable case naming and minimal behavioral risk.

## Scope
- `KVM/qemu/boot_repeat.cfg`
- `KVM/qemu/multi_vms.cfg`
- `KVM/qemu/tests/boot_repeat.py`

## Requirement Summary (Current)
1. Repeated-boot scenarios are centralized in `boot_repeat.cfg`.
2. `multi_vms.cfg` currently keeps baseline multi-vm functional matrix only (`1td_1vm`, `2td.*`, `4td`, `2vm.*`, `4vm`) and does not carry repeat/memory_sweep variants.
3. `boot_repeat.cfg` currently contains:
  - `one_vm_repeat.@default` with `10times`/`20times` + `one_socket`/`two_socket` + `vm`/`td`
  - `one_vm_repeat.specific_memory_size` (`memory_sizes_list` flow)
  - `one_vm_repeat.memory_sweep` (`type = multi_vms_multi_boot`, `1vm`/`1td`, `from1G_toall`)
  - `multi_vms_repeat.@default` with `20times` for `4td` and `4vm`
  - `multi_vms_repeat.memory_sweep` (`type = multi_vms_multi_boot`, `1td_1vm`/`2td`/`4td`/`2vm`/`4vm`, `from1G_toall`)
4. Preserve semantic compatibility unless explicitly asked to rename/remove old paths.
5. Prefer cfg-only migration first; change Python only when behavior truly requires it.

## Case Handling Rules
- When changing cfg hierarchy, preserve parameter payloads (`iterations`, `memory_sizes_list`, `start_mem`, random window params, TDX params, `vms`).
- Remove no-op cfg nodes when safe:
  - empty `variants:` blocks
  - empty `@default` entries
- Do not silently change generator semantics (`random_32g_window`, window sizing, unit).

## Python Rules (`boot_repeat.py`)
- Keep entrypoint: `run(test, params, env)` with `@error_context.context_aware`.
- Keep existing mode split stable:
  - `memory_sizes_list` flow: boot once per listed memory size
  - iterations flow: boot/destroy cycle repeated by `iterations`
- Keep side-effect import: `from provider import dmesg_router  # pylint: disable=unused-import` in test scripts when required by local style.
- Only add memory-sweep execution flow to `boot_repeat.py` when cfg type is actually routed to `boot_repeat` and behavior requires it.
- Always keep VM cleanup in `finally`.

## Validation Checklist
- Case names after refactor are explicit and predictable.
- No corrupt cfg nesting or indentation drift.
- No unexpected case removals.
- `grep` check for moved keys:
  - `memory_sweep`
  - `iterations = 20`
- Ensure each moved case still has a valid `type` target.
- Ensure `multi_vms_repeat.memory_sweep` and `one_vm_repeat.memory_sweep` still route to `type = multi_vms_multi_boot`.
- Ensure baseline `multi_vms.cfg` does not accidentally re-introduce repeat/memory_sweep variants.

## Commit Message Guidance
Use concise scope + intent, for example:

`KVM/qemu: refine boot_repeat case hierarchy`

Body should mention:
- what moved from `multi_vms.cfg`
- what was centralized in `boot_repeat.cfg`
- whether behavior changed or only hierarchy changed

## Current Baseline Notes
- `boot_repeat.cfg` is now the primary home for repeat and memory-sweep matrices.
- `multi_vms.cfg` remains the baseline functional matrix without repeat wrappers.
- `boot_repeat.py` currently supports `iterations` and `memory_sizes_list`; memory_sweep execution is delegated via cfg `type = multi_vms_multi_boot`.
