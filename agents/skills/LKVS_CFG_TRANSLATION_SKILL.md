# LKVS CFG Translation Skill

Skill for translating test case configurations from vmm_tree to LKVS cartograph format.

## When to Use

Use this skill when:

- Converting legacy vmm_tree test matrix to LKVS `KVM/qemu/*.cfg`
- Adding new variant groups to existing cfg files
- Restructuring cfg hierarchy while preserving behavior
- Need to validate cfg syntax and structure integrity

## Inputs

1. **Legacy CFG Block** (from vmm_tree or documentation)

- Raw cfg text or parameter list
- Execution model (test type, handler)
- Variant hierarchy (if structured)

2. **Target LKVS CFG File**

- Destination cfg file path (`boot_repeat.cfg`, `multi_vms.cfg`, etc.)
- Existing structure to match
- Indent style (confirm 4 spaces)

3. **Parameter Mapping Table**

- Produced by LKVS_PARAMETER_MAPPING_SKILL
- Maps legacy -> LKVS parameter names and values

## Process

### Step 1: Identify Cartograph Target Location

- Review target cfg file structure (variants, @default nesting, case groups)
- Determine where new/modified cases belong
- Same feature family -> existing variant block
- New test family -> new variant group at appropriate nesting level
- Note indentation level (typically 4 spaces per level)

### Step 2: Translate Variant Hierarchy

- Legacy structure: `group.subgroup.case`
- LKVS structure: `variants: \n    group: \n        - subgroup: ...`
- Convert flat naming to nested blocks
- Preserve semantic grouping (for example, VM types and memory strategies)

Example translation:

```text
Legacy: tdx_vmx2_from1024m_toall
  |- tdx_vmx2 (VM type: TDX x VMX2)
  `- from1024m_toall (sweep from 1GB to max)

LKVS cartograph:
  variants:
    one_vm_repeat:
      - memory_sweep:
          - 1td:
              - from1G_toall:
                  # parameters here
```

### Step 3: Inline Parameters

- Use mapping table to translate each legacy parameter
- Group related parameters logically:
- VM configuration (mem, vcpu, machine_type_extra_params)
- Sweep behavior (mem_generator, start_mem, random window params)
- Execution (iterations, type)
- Add cfg comments for non-obvious transformations or custom logic

Example:

```properties
# Migrated from vmm_tree/validation/kvm_pts/boot_repeat.cfg::tdx_vmx2_from1024m_toall
# Sweep 1 TDX VM from 1GB to all available memory
type = multi_vms_multi_boot
iterations = 20
mem_generator = random_32g_window
start_mem = 1024
random_min = 0
random_max = 64
random_unit = 511
samples_per_window = 2
window_size = 32768
machine_type_extra_params = "kernel-irqchip=split"
vm_secure_guest_type = tdx
```

### Step 4: Validate Structure And Syntax

- **Indentation**: Confirm all nested levels use 4-space increments
- **No trailing whitespace**: Check each line
- **Nesting balance**: `variants:` must have at least one child entry
- **Param keys**: All keys must be valid (no typos)
- **Required params present**: Check that `type` is set; verify handler needs other params
- **Quote style**: When a cfg value needs quotes, use **double quotes** to match LKVS coding style
- Preferred: `machine_type_extra_params = "kernel-irqchip=split"`
- Avoid: single-quoted string values unless file-local style explicitly requires otherwise

Validation checks:

```bash
# Check indentation integrity (4-space levels)
grep -n "^  [^ ]" lkvs/KVM/qemu/*.cfg  # Should find no tabs/odd indents

# Check for trailing spaces
grep -n " $" lkvs/KVM/qemu/*.cfg

# Verify variant nesting (basic structure)
# Look for orphaned params (not under any variant)
```

### Step 5: Cross-Reference Parameter Usage

- Confirm that cfg `type = <handler>` exists in Python tests
- Verify handler Python test reads all required cfg parameters
- If new parameters are added, ensure Python test can parse them safely

File structure:

```text
KVM/qemu/boot_repeat.cfg         # cfg variant matrix
KVM/qemu/tests/boot_repeat.py    # check whether it uses new params
```

## Output

Translated cfg block ready to insert into target file:

- Correct nesting and indentation
- All parameters inlined with comments
- No syntax errors
- Structure validated against target cfg style

## Common Patterns

### Pattern 1: Single-VM Repeat With Memory Sweep

```properties
# Legacy: tdx_vmx2_from1024m_toall
- one_vm_repeat:
    - memory_sweep:
        - 1td:
            - from1G_toall:
                type = multi_vms_multi_boot
                iterations = 20
                # ... params
```

### Pattern 2: Multi-VM Repeat With Memory Sweep

```properties
# Legacy: 4td_from1024m_toall
- multi_vms_repeat:
    - memory_sweep:
        - 4td:
            - from1G_toall:
                type = multi_vms_multi_boot
                iterations = 20
                # ... params
```

### Pattern 3: Baseline Functional (No Sweep)

```properties
# Legacy: 4td_20times
- multi_vms_repeat:  # or @default
    - 20times:
        vms = td td td td
        iterations = 20
        # ... params
```

## Risk Areas

1. **Unit Mismatches**: Legacy memory param in MiB, LKVS in GiB

- Mitigation: Mapping table must show conversion; add cfg comment

2. **Deeply Nested Variants**: Creating more than 3-4 levels confuses case naming

- Mitigation: Keep nesting shallow; use `@default` to avoid cross-product explosion

3. **Parameter Duplication**: Same param set in multiple variant paths

- Mitigation: Use `@default` block to centralize common params; override only specifics

4. **Tab vs Space**: Mixing indentation types breaks cartograph parsing

- Mitigation: Always use spaces (4 per level); avoid tabs entirely

5. **Quote Inconsistency**: Mixing single and double quotes in cfg strings increases review churn

- Mitigation: For quoted values, standardize on double quotes across translated blocks

6. **TDX-Only Parameters in Common Scope**: Placing TDX-specific parameters (e.g. `bus_lock_source_file_tdx`, `vm_secure_guest_type`) outside any `variants:` block makes them visible to **all** variants, including `vm`.

- When the Python side uses `params.get("key_tdx", params["key"])` for fallback, the fallback will **never** trigger if `key_tdx` is in the common scope, causing non-TDX variants to silently use the TDX value.
- Mitigation: Always scope TDX-only parameters inside the `td:` variant block, never in the common section.

```properties
# Wrong: bus_lock_source_file_tdx visible to vm variant too
bus_lock_source_file = bus_lock.c
bus_lock_source_file_tdx = bus_lock_64.c   # <- common scope, affects vm!
variants:
    - vm:
    - td:
        vm_secure_guest_type = tdx

# Correct: TDX parameter scoped inside td: only
bus_lock_source_file = bus_lock.c
variants:
    - vm:
    - td:
        vm_secure_guest_type = tdx
        bus_lock_source_file_tdx = bus_lock_64.c   # <- td scope only
```

## Related Skills

- **LKVS_PARAMETER_MAPPING_SKILL**: Provides parameter translation table input
- **NAMING_NORMALIZATION_SKILL**: Ensures variant names are consistent
- **VMM_TREE_LEGACY_CASE_ANALYSIS_SKILL**: Extracts legacy cfg semantics to translate
