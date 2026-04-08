# Naming Normalization Skill

Skill for standardizing test case names from vmm_tree legacy conventions to LKVS naming patterns.

## When to Use

Use this skill when:
- Multiple cases are being migrated (batch naming)
- Need to establish consistent naming for a case family
- Defining naming schema for recurring case patterns
- Validating case names don't conflict with existing LKVS cases

## Inputs

1. **Legacy Case Names** (from vmm_tree)
   - List of cases to migrate
   - Case family/group (memory sweep, repeat boot, functional, etc.)
   - Extracted semantics (from VMM_TREE_LEGACY_CASE_ANALYSIS_SKILL)

2. **LKVS Naming Conventions** (existing patterns)
   - Review existing KVM/qemu case names
   - Identify convention per case family
   - Document explicit rules or implicit patterns

3. **Naming Schema Goals**
   - Clarity: names clearly indicate test type and parameters
   - Stability: names are unlikely to change
   - Predictability: new cases follow same schema
   - Searchability: grep/search work intuitively

## Process

### Step 1: Analyze LKVS Naming Patterns
- Review existing cases in target cfg file
- Document naming conventions currently used
- Identify patterns by test family

Example LKVS patterns (existing in KVM/qemu/boot_repeat.cfg):
```
Existing naming in boot_repeat.cfg:

  one_vm_repeat:
    - 10times:
        @default: ...    # Baseline functional, 10 cycles
    - 20times:
        @default: ...    # Baseline functional, 20 cycles
    - memory_sweep:
        - 1td:
            - from1G_toall: ...       # Single TDX VM, 1GB-to-all sweep
        - 1vm:
            - from1G_toall: ...       # Single normal VM, 1GB-to-all sweep
  
  multi_vms_repeat:
    - 20times:
        - 4td: ...                    # 4 TDX VMs, 20 iterations
        - 4vm: ...                    # 4 normal VMs, 20 iterations
    - memory_sweep:
        - 1td_1vm:
            - from1G_toall: ...       # Mixed 1 TDX + 1 VM, sweep
        - 2td:
            - from1G_toall: ...       # 2 TDX VMs, sweep
        - 4td:
            - from1G_toall: ...       # 4 TDX VMs, sweep
        - 2vm:
            - from1G_toall: ...       # 2 normal VMs, sweep
        - 4vm:
            - from1G_toall: ...       # 4 normal VMs, sweep

Naming patterns observed:
    - Iteration counts: 10times, 20times (singular "times")
    - VM counts: 1td, 1vm, 2td, 4td, 2vm, 4vm (count+type)
    - Memory sweeps: from1G_toall (unit: GB for human readability)
    - Hierarchy levels: operation (one_vm_repeat) → subtype (memory_sweep) → config (1td) → variant (from1G_toall)
```

### Step 2: Create Legacy→LKVS Mapping Table
- List all legacy cases to migrate
- Break down each legacy name components (VM type, VM count, parameters)
- Map to proposed LKVS name components
- Validate no name collisions

Example mapping table:
```
Legacy Name | VM Type | VM Count | Memory Range | Iterations | LKVS Proposed | Notes
------------|---------|----------|--------------|------------|---------------|-------
tdx_vmx2_from1024m_toall | tdx | 1 | 1024m-all | 20 | 1td.from1G_toall | Name simplified; memory unit changed MiB→GB
vm_single_from1024m_toall | vm | 1 | 1024m-all | 20 | 1vm.from1G_toall | Clarified "single" → "1vm"
vmm2_from2048m_toall | vm | 2 | 2048m-all | 20 | 2vm.from2G_toall | Memory unit changed; vm count explicit
4td_from512m_toall | td | 4 | 512m-all | 20 | 4td.from512M_toall | Note: starting below 1GB (512MB)
4td_20times_no_sweep | td | 4 | N/A | 20 | 4td (under @default) | Functional test; no sweep suffix
1td_1vm_mixed_from1024m | td+vm | 1+1 | 1024m-all | 20 | 1td_1vm.from1G_toall | Explicit mixed type naming
```

### Step 3: Define Naming Schema Rules
- Document explicit rules for each name component
- Handle edge cases and exceptions
- Ensure consistency across case family

Example schema rules for memory sweep cases:
```
NAMING SCHEMA: boot_repeat.one_vm_repeat/multi_vms_repeat.memory_sweep

Format: <vm_config>.<memory_range>

Component 1: vm_config (required)
  - Single VM: <count><type>
    ○ 1td (1 TDX VM)
    ○ 1vm (1 normal VM)
  - Multi VM: <count><type>[_<count><type>]*
    ○ 4td (4 TDX VMs, all same type)
    ○ 4vm (4 normal VMs, all same type)
    ○ 1td_1vm (1 TDX + 1 normal VM, mixed)
    ○ 2td_2vm (2 TDX + 2 normal, mixed)

  Type abbreviations:
    ○ td = TDX confidential VM
    ○ vm = normal VM
  
  Count:
    ○ 1, 2, 4, 8, ... (explicit count prefix)
    ○ Total VM count must match multi_vms.cfg baseline or tested scenario

Component 2: memory_range (required for sweep variants)
  - Format: from<start>_to<end>
  - Start value (unit in GiB for clarity):
    ○ from512M (less common; minimum viable VM memory)
    ○ from1G (standard baseline; 1024 MiB)
    ○ from2G (higher baseline; 2048 MiB)
  - End value:
    ○ toall (extend to all available host memory)
    ○ to<size>G (fixed upper bound; rare)

Full example names:
  - 1td.from1G_toall
  - 1vm.from1G_toall
  - 4td.from1G_toall
  - 4vm.from1G_toall
  - 1td_1vm.from1G_toall
  - 2td_2vm.from1G_toall

Non-sweep examples (functional baseline):
  - 1td (under @default or 10times/20times variants)
  - 4vm (baseline functional, no sweep)
```

### Step 4: Handle Edge Cases & Exceptions
- Document any names that break the pattern
- Explain why (backward compatibility, future-proof, etc.)
- Mark as stable or subject to future change

Example edge cases:
```
Edge Case 1: Memory starts below 1GB (e.g., from512m_toall)
  Rationale: Validate minimum memory scenarios (resource constrained)
  Naming: from512M (uppercase M for non-1GB unit)
  Risk: Uncommon; may confuse users
  Action: Document in cfg comment; keep if legacy precedent exists

Edge Case 2: Fixed upper memory bound (e.g., from1G_to16G)
  Rationale: Test specific range without maxing out host memory
  Naming: from1G_to16G (explicit upper bound)
  Risk: Rare; not currently used
  Action: Reserve naming if future use appears; don't use yet

Edge Case 3: Non-standard VM count (e.g., 6td)
  Rationale: Unusual but possible future extension
  Naming: 6td (follows <count><type> pattern)
  Risk: Not currently tested; may have resource/scaling issues
  Action: Document; reserve pattern; validate if deployed
```

### Step 5: Batch Validation
- Check for naming conflicts with existing LKVS cases
- Verify no 1:many or many:1 mappings (each legacy should map to 1 LKVS name)
- Ensure names are URL/filesystem safe (no special chars)
- Check length (keep reasonable; <60 chars preferred)

Validation checks:
```bash
# 1. Extract existing case names from LKVS cfg
grep -oP "^    - \K[\w]+" KVM/qemu/boot_repeat.cfg | sort | uniq

# 2. Check proposed names don't exist
proposed_names="1td.from1G_toall 4td.from1G_toall ..."
for name in $proposed_names; do
    grep -q "$name" KVM/qemu/boot_repeat.cfg && echo "CONFLICT: $name already exists"
done

# 3. Check names are filesystem safe
for name in $proposed_names; do
    [[ "$name" =~ [^a-zA-Z0-9._-] ]] && echo "INVALID CHARS: $name"
done

# 4. Report name lengths
for name in $proposed_names; do
    echo "${#name}: $name"
done | sort -rn
```

Output:
```
Validation Results:
  ✓ No existing names conflict with proposed set
  ✓ All proposed names use only [a-zA-Z0-9._-]
  ✓ Name lengths: max 22 chars (well under limit)
  ✓ All proposed names follow established pattern
```

### Step 6: Create Migration Mapping Document
- Final comprehensive table for commit reference
- Link legacy → LKVS names
- Document any divergences from standard schema
- Note case counts and groups

Example final document:
```yaml
---
migration_id: vmm_tree_boot_repeat_batch_1
date: 2024-03-25
source: vmm_tree/validation/kvm_pts/boot_repeat.cfg

naming_schema_version: boot_repeat_v1

cases_migrated:
  one_vm_repeat:
    memory_sweep:
      - legacy: tdx_vmx2_from1024m_toall
        lkvs: 1td.from1G_toall
        status: ready_for_migration
      - legacy: vm_single_from1024m_toall
        lkvs: 1vm.from1G_toall
        status: ready_for_migration
      - legacy: vmm2_mixed_from1024m_toall
        lkvs: 1td_1vm.from1G_toall
        status: ready_for_migration
  
  multi_vms_repeat:
    memory_sweep:
      - legacy: 4td_from1024m_toall
        lkvs: 4td.from1G_toall
        status: ready_for_migration
      - legacy: 4vm_from1024m_toall
        lkvs: 4vm.from1G_toall
        status: ready_for_migration
      - legacy: 2td_2vm_mixed_from1024m_toall
        lkvs: 2td_2vm.from1G_toall
        status: ready_for_migration

total_cases: 6
total_case_families: 2

divergences: none
exceptions: none

validation:
  - "All names follow boot_repeat_v1 schema"
  - "No naming conflicts with existing LKVS cases"
  - "All names validated against filesystem/URL safety rules"
  - "Legacy semantics preserved in LKVS target"
```

## Output

**Naming Mapping Document** containing:
1. LKVS naming patterns analysis (existing conventions)
2. Legacy→LKVS mapping table (all cases)
3. Defined naming schema with rules
4. Edge cases and exceptions (documented or excluded)
5. Validation results (conflicts, safety checks)
6. Final YAML/table for commit inclusion
7. Migration readiness confirmation

## Common Naming Pitfalls

### Pitfall 1: Abbreviations Too Aggressive
```
✗ Bad: "1t" (unclear if "t" = TDX or generic "t"?)
✓ Good: "1td" (explicit: 1 TDX VM)

✗ Bad: "f1g" (too abbreviated; unclear)
✓ Good: "from1G" (readable: from 1 GB)
```

### Pitfall 2: Naming Inconsistency Across Cases
```
✗ Bad: Some use "from1G", others "1gb", others "1024m"
✓ Good: All memory sweeps use "from1G_toall" format

✗ Bad: Some use "td" for TDX; others use "tdx"
✓ Good: Always use "td" (convention locked)
```

### Pitfall 3: Missing Information in Name
```
✗ Bad: "memory_sweep" (doesn't tell what's swept or what VMs)
✓ Good: "4td.from1G_toall" (4 TDX VMs, 1GB-to-all sweep)

✗ Bad: "1vm" (doesn't indicate if sweep or functional)
✓ Good: "1vm.from1G_toall" (sweep variant; clarity from context)
```

### Pitfall 4: Overly Generic Names
```
✗ Bad: "test_1" (not descriptive; could be anything)
✓ Good: "1td.from1G_toall" (intent obvious; predictable)

✗ Bad: "memory" (too vague; multiple meanings)
✓ Good: "from1G_toall" (specific sweep strategy)
```

## Related Skills
- **VMM_TREE_LEGACY_CASE_ANALYSIS_SKILL**: Provides case semantics for naming
- **LKVS_CFG_TRANSLATION_SKILL**: Uses normalized names in cfg structure
- **LKVS_PARAMETER_MAPPING_SKILL**: Works with named variants to map parameters
