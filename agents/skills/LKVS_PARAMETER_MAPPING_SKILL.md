# LKVS Parameter Mapping Skill

Skill for identifying and translating test parameters from vmm_tree to LKVS equivalents.

## When to Use

Use this skill when:
- Starting a new case migration (build parameter equivalence table)
- Parameter semantics are unclear or implicit in legacy code
- Need to identify missing or new parameters required by LKVS test
- Validating that cfg parameters match Python test expectations

## Inputs

1. **Legacy Parameter List** (from vmm_tree cfg or test)
   - Parameter names and values
   - Parameter types (int, string, list, etc.)
   - Any comments explaining semantics
   - Documentation (README, test docstrings)

2. **LKVS Test Handler** (target Python test file)
   - Parameters it reads via `params.get()`, `params.get_numeric()`, etc.
   - Default values if present
   - Type expectations (parsed range, unit)

3. **Domain Context** (from agent or case group)
   - Test type (memory sweep, repeat boot, functional)
   - VM types involved (vm, td, etc.)
   - Execution model (cfg-driven behavior)

## Process

### Step 1: Extract Legacy Parameters
- Locate legacy case definition (vmm_tree cfg file)
- Extract all parameters:
  - Explicit (directly set in cfg or test)
  - Implicit (derived from case name or group context)
  - Conditional (set only for certain VM types)
- Document source file location and line numbers for traceability

Example:
```
vmm_tree source: /validation/kvm_pts/boot_repeat.cfg, line 42-55
Legacy case: tdx_vmx2_from1024m_toall

Extracted parameters:
    iterations = 20
    start_mem = 1024  # MiB (inferred from name "from1024m")
    mem_generator = random_32g_window
    random_min = 0
    random_max = 64
    random_unit = 511
    samples_per_window = 2
    window_size = 32768
    vm_type = td  # inferred from "tdx" in case name
    vms = td
    machine_type_extra_params = "kernel-irqchip=split"
    vm_secure_guest_type = tdx
```

### Step 2: Identify LKVS Handler & Its Parameter Expectations
- Locate target LKVS Python test (e.g., `multi_vms_multi_boot.py`)
- Extract parameters the test actually reads:
  - Search for `params.get(...)`, `params.get_numeric(...)`, `params.objects()...`
  - Note default values
  - Identify required vs. optional parameters

Example analysis for `multi_vms_multi_boot.py`:
```
Handler: KVM/qemu/tests/multi_vms_multi_boot.py

Parameters read by handler:
    mem_generator (required, no default)
        → controls sweep algorithm (random_32g_window, linear, etc.)
    
    start_mem (default: auto-computed if missing)
        → memory value (unit: MiB) to start sweep from
        → used as baseline; sweep expands from this point
    
    random_min, random_max, random_unit, samples_per_window, window_size
        → required if mem_generator = random_32g_window
        → optional if mem_generator = linear
    
    iterations (default: 1)
        → repeat boot/destroy cycle N times at each memory point
    
    vms (required)
        → list of VM names to create (e.g., "vm", "td", "td td td td")
        → parsed by params.objects()
    
    mem (per-VM via params.object_params(vm))
        → memory in MiB for this VM
        → set dynamically during sweep
```

### Step 3: Build Parameter Mapping Table
- Create table: Legacy Param → LKVS Equivalent
- Columns: 
  - Legacy Name
  - Legacy Value/Range
  - Legacy Unit
  - LKVS Name
  - LKVS Value/Range
  - LKVS Unit
  - Conversion Formula (if any)
  - Notes

Example table:
```
| Legacy | Value | Unit | LKVS | Value | Unit | Conversion | Notes |
|--------|-------|------|------|-------|------|------------|-------|
| iterations | 20 | count | iterations | 20 | count | 1:1 | Direct match |
| start_mem | 1024 | MiB | start_mem | 1024 | MiB | 1:1 | Already in MiB |
| mem_generator | random_32g_window | name | mem_generator | random_32g_window | name | 1:1 | Generator name preserved |
| random_min | 0 | pct | random_min | 0 | pct | 1:1 | Window min boundary % |
| random_max | 64 | pct | random_max | 64 | pct | 1:1 | Window max boundary % |
| random_unit | 511 | MiB | random_unit | 511 | MiB | 1:1 | Step size within window |
| window_size | 32768 | MiB | window_size | 32768 | MiB | 1:1 | 32GB window |
| vm_type (inferred) | "tdx" | name | vm_secure_guest_type + machine_type_extra_params | "tdx" + "kernel-irqchip=split" | name | Split into 2 params | TDX requires kernel-irqchip=split |
| case name (inferred) | "from1024m_toall" | name | case suffix | "from1G_toall" | name | MiB→GB renaming | Unit changed in LKVS naming |
```

### Step 4: Identify Gaps & Transformations
- **Missing parameters** in LKVS:
  - Are they genuinely unused, or must be added?
  - If unused: acceptable? Document why.
  - If must-have: who sets them? (cfg vs. Python)

- **New parameters** required by LKVS but absent in legacy:
  - Are they derivable from legacy params?
  - Must cfg provide them explicitly?
  - Check handler code for fallback defaults

- **Unit transformations**:
  - E.g., MiB → GiB, seconds → milliseconds
  - Document formula and validation method

Example gaps:
```
Missing in LKVS config:
  ✗ no explicit "samples_per_window" in old cfg
    → Found in multi_vms_multi_boot.py: def = 2
    → Acceptable: handler has default

New in LKVS config:
  ✓ divide_host_mem_limit_by_vm_count (for multi-vm fairness)
    → Legacy: implicit (single VM only?) or default behavior
    → LKVS: explicit cfg param, must set for multi-vm case
    → Transformation: set = yes for multi-vm; leave unset for single-vm
```

### Step 5: Semantic Validation
- Trace parameter through entire flow:
  1. **CFG**:  Parameter defined in cartograph
  2. **Python**: Handler reads parameter via `params.get(...)`
  3. **Behavior**: Parameter controls observable test behavior (memory ranges, iteration count, VM setup)
  
- Validate alignment:
  - Naming: legacy `foo_bar` → LKVS `foo_bar` (unchanged) or renamed + documented?
  - Types: legacy "0-64" range → LKVS numeric `int` with same range?
  - Semantics: legacy "random sweep 1GB-all" → LKVS `mem_generator=random_32g_window` with matching window?

### Step 6: Document Transformation Rules
- For each parameter group (e.g., "memory sweep generator"), write a transformation rule:

```
RULE: Memory Sweep Generator Parameters
  Input (legacy):
    iterations = 20, start_mem = 1024 (MiB), mem_generator = random_32g_window,
    random_min = 0, random_max = 64, random_unit = 511, 
    samples_per_window = 2, window_size = 32768
    
  Output (LKVS cfg):
    iterations = 20
    mem_generator = random_32g_window
    start_mem = 1024
    random_min = 0
    random_max = 64
    random_unit = 511
    samples_per_window = 2
    window_size = 32768
    
  Validation:
    - Assert start_mem >= 1024 (never below 1GB baseline)
    - Assert window_size = 32768 (32GB standard window)
    - Assert random_min + random_max <= 100 (valid window boundaries)
    - Verify multi_vms_multi_boot.py reads all these params
```

## Output

**Parameter Mapping Document** with sections:

1. **Mapping Table** (as shown above)
2. **Transformation Rules** (by parameter group)
3. **Gap Analysis** (missing/new/dropped params)
4. **Semantic Validation** (cross-check against handler code)
5. **Cfg Template** (ready-to-use parameter block)

Example final output:
```yaml
---
migration_id: vmm_tree.tdx_vmx2_from1024m_toall
target: lkvs.boot_repeat.one_vm_repeat.memory_sweep.1td.from1G_toall

parameters:
  iterations:
    legacy: 20
    lkvs: 20
    note: Direct pass-through

  mem_generator:
    legacy: random_32g_window
    lkvs: random_32g_window
    note: Name preserved; algorithm unchanged

  start_mem:
    legacy: 1024 MiB
    lkvs: 1024
    unit_change: "MiB (in both)"
    note: Same baseline in both systems

  # ... rest of parameters ...

validation:
  - "Handler multi_vms_multi_boot.py reads all parameters"
  - "Memory sweep bounds: 1GB baseline, 32GB window, random within random_min-random_max%"
  - "Semantics: repeated boot/destroy at each memory point, 20 iterations per value"
  - "VM type: TDX with kernel-irqchip=split"

cfg_template: |
  # From vmm_tree: tdx_vmx2_from1024m_toall
  - 1td:
      - from1G_toall:
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
          divide_host_mem_limit_by_vm_count = no
```

## Common Patterns

### Pattern 1: Direct Parameter Pass-Through
```
Legacy param → LKVS param (same name, same value)
Example: iterations=20 → iterations=20
Action: Copy directly; validate type compatibility
```

### Pattern 2: Unit Conversion
```
Legacy: memory_size = 2048 (MiB)
LKVS: memory_size = 2 (GiB)
Action: Apply conversion formula; add cfg comment
```

### Pattern 3: Implicit to Explicit
```
Legacy: case name "tdx_vmx2" implies {vms=td, vm_secure_guest_type=tdx}
LKVS: must set explicitly in cfg
Action: Extract intent; set both parameters
```

### Pattern 4: Parameter Splitting
```
Legacy: single param "tdx_config"
LKVS: split into machine_type_extra_params + vm_secure_guest_type
Action: Parse legacy; create 2 LKVS params
```

## Related Skills
- **VMWARE_LEGACY_CASE_ANALYSIS_SKILL**: Extracts legacy parameter semantics
- **LKVS_CFG_TRANSLATION_SKILL**: Uses mapping table to translate cfg
- **LKVS_TEST_IMPLEMENTATION_SKILL**: Verifies Python test reads mapped parameters
