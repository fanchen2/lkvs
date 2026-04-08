# Case Migration Case Patterns

This guide holds migration notes that are intentionally scoped to specific case families or previously migrated cases.

Use these notes only when the target case matches the same execution model or naming pattern. If a rule proves reusable across multiple migrations, move the generalized lesson back into the framework agent.

## Boot Repeat And Memory Sweep Family

### Consolidation Rules
- `boot_repeat.cfg` is the primary home for repeat and memory-sweep matrices.
- `multi_vms.cfg` remains the baseline functional matrix only; do not move repeat or memory-sweep variants back there unless explicitly requested.
- `one_vm_repeat.specific_memory_size` uses the `memory_sizes_list` flow.
- `one_vm_repeat.memory_sweep` and `multi_vms_repeat.memory_sweep` route to `type = multi_vms_multi_boot`.
- Preserve cfg payloads for these flows: `iterations`, `memory_sizes_list`, `start_mem`, `random_min`, `random_max`, `random_unit`, `samples_per_window`, `window_size`, `divide_host_mem_limit_by_vm_count`, and `vms`.
- Keep sweep names stable as `from1G_toall` across VM and TD branches unless a naming migration is explicitly requested.
- `multi_vms_multi_boot.py` keeps `random_32g_window` as the active generator, `linear` as fallback, applies the same `mem` to all VMs in an iteration, uses `utils_misc.get_usable_memory_size()` as host memory source, and cleans up with `vm.destroy(gracefully=False)` in `finally`.
- For high guest-count cases, tune host nofile limits in process scope only and restore the original limits in `finally`.
- Keep iteration logs concise and include the effective memory value per iteration.

### Example Migration Flow
Legacy case `tdx_vmx2_from1024m_toall` in vmm_tree translates to:
- **LKVS target**: `boot_repeat.one_vm_repeat.memory_sweep.1td.from1G_toall`
- **Semantics preserved**: memory sweep from 1GB to all available memory
- **VM type**: TDX confidential VM (1TD)
- **Execution model**: repeated boot/destroy cycles with varying memory sizes

### Naming Example
- `tdx_vmx2_from1024m_toall` -> `1td.from1G_toall`
- `tdx_vmx2` -> `1td` for one TDX guest
- `from1024m_toall` -> `from1G_toall` after unit normalization

## Large Multi-VM Auto-Scaling Cases

### Case Notes: `many_vm_1G` (April 2026)

**Context**: `many_vm_1G` adds large-scale VM boot coverage in `multi_vms` with auto guest-count calculation.

**Finalized Behavior**:
- Guest runtime memory remains fixed by cfg (`mem = 1024`, `max_mem = 1024`).
- Guest count is auto-calculated from host usable memory and then capped by `max_guest_count`.
- Guest-count calculation can include per-guest overhead via cfg parameter `guest_count_mem_overhead`.

**Finalized Implementation Rules**:

1. **Separate Runtime Memory From Count Memory**
   - Keep `mem` for actual VM boot memory.
   - Use a distinct computed value (`guest_count_mem`) for guest-count sizing.
   - Do not overload one variable for both purposes.

2. **Use Existing Helper, Avoid Duplicate Count Logic**
   - Centralize auto guest-count logic in `_prepare_auto_calc_guest_count()`.
   - `run()` should consume returned values for validation instead of re-implementing the same math.

3. **Bounded Auto Scaling**
   - Guest count formula: `guest_num = host_usable_mem // guest_count_mem`.
   - Always apply `max_guest_count` cap for stability and lab safety.

4. **Process-Scoped Resource Tuning**
   - For high guest-count cases, tune host nofile limit in test runtime only.
   - Save original RLIMIT values and restore in `finally`.
   - Avoid persistent host config writes in cfg hooks for this scenario.

5. **Naming Clarity For Params**
   - Prefer explicit names such as `guest_count_mem_overhead`.
   - Avoid ambiguous names like `host_mem_*` for per-guest sizing terms.

### Commit Message Example

```
KVM: add many_vm_1G case to multi_vms tests

Add many_vm_1G variant that auto-calculates VM count from available host memory.
Each VM uses fixed 1024MB; guest_num = usable_memory // 1024.

Extended multi_vms_multi_boot handler with auto_calc_guest_count mode.

Signed-off-by: <name> <email>
```

### When To Reuse These Notes
- Reuse the boot-repeat section only for repeat or memory-sweep migrations that share the same cfg routing and handler model.
- Reuse the large multi-VM section only when the case auto-derives guest count or applies temporary host resource tuning.
- If a new migration differs materially, treat these notes as reference material instead of policy.