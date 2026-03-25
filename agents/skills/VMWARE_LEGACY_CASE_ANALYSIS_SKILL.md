# vmm_tree Legacy Case Analysis Skill

Skill for reverse-engineering legacy test case semantics from vmm_tree source.

## When to Use

Use this skill when:
- First analyzing a vmm_tree test case for migration
- Parameter intent is unclear or implicit
- Comparing legacy behavior with LKVS implementation
- Documenting case purpose and assumptions

## Inputs

1. **Legacy Case Reference** (name or location)
   - vmm_tree cfg file path
   - Case name or group identifier
   - Any available documentation

2. **vmm_tree Directory Structure** (for context)
   - Access to cfg files and test handlers
   - Related configuration files
   - Test output logs (if available)

## Process

### Step 1: Locate Legacy Case Definition
- Find vmm_tree cfg file (typically under `validation/kvm_pts/` or `validation/vmm-pts/`)
- Identify case name and its enclosing variant blocks
- Extract raw parameter definitions
- Note file path and line numbers for traceability

Example search:
```bash
# Find case in vmm_tree
grep -r "tdx_vmx2_from1024m_toall" vmm_tree/validation/

# Likely found in:
# vmm_tree/validation/kvm_pts/boot_repeat.cfg
```

Output location:
```
vmm_tree/validation/kvm_pts/boot_repeat.cfg, lines 42-70
```

### Step 2: Extract Raw Parameters
- Copy all cfg lines for this case (from `variants:` entry point down)
- Include parent variant blocks (for inherited params)
- Document parameter sources:
  - Explicitly set at this level
  - Inherited from parent variant blocks
  - Derived from case name (implicit)

Example extraction:
```
Case: tdx_vmx2_from1024m_toall
Location: vmm_tree/validation/kvm_pts/boot_repeat.cfg, line 42

Raw cfg block:
    - one_vm_repeat:
        - 20times:
            iterations = 20
            type = multi_vms_multi_boot
            vm_secure_guest_type = tdx
            machine_type_extra_params = "kernel-irqchip=split"
        - memory_sweep:
            iterations = 20
            mem_generator = random_32g_window
            start_mem = 1024
            random_min = 0
            random_max = 64
            random_unit = 511
            samples_per_window = 2
            window_size = 32768

Extracted parameters (for one_vm_repeat.memory_sweep):
  Explicit in this case:
    - iterations = 20
    - mem_generator = random_32g_window
    - start_mem = 1024
    - random_min = 0
    - random_max = 64
    - random_unit = 511
    - samples_per_window = 2
    - window_size = 32768
    
  Inherited from ancestors:
    - type = multi_vms_multi_boot
    - vm_secure_guest_type = tdx
    - machine_type_extra_params = "kernel-irqchip=split"
    
  Implicit in case name "tdx_vmx2_from1024m_toall":
    - VM count: 1 (implied by "vmx2" = 1 VMX VM? Or variant nesting?)
    - VM type: TDX (implied by "tdx" prefix)
    - Memory sweep start: 1024 MiB (from "from1024m")
    - Memory sweep end: all available (from "toall")
```

### Step 3: Decode Case Name Convention
- Analyze naming pattern: `<prefix>_<vm_type><vm_count>_<memory_strategy>_<naming_suffix>`
- Document naming scheme for this case family

Example decode for vmm_tree boot_repeat cases:
```
Case name: tdx_vmx2_from1024m_toall

Naming breakdown:
  prefix: "tdx" (TDX VM type flag)
  vm_type_count: "vmx2" (unclear: 1 TDX? 2 VMX VMs? Variant of TDX?)
  memory_start: "from1024m" (memory starts at 1024 MiB)
  memory_end: "toall" (memory sweep extends to all available)
  full_name: "tdx_vmx2_from1024m_toall"

Interpretation issues:
  ? "vmx2" is ambiguous - check cfg structure or test code
  ? "1024m" - is this the minimum each VM gets, or total system?
  ? "from...toall" - does it respect host memory limits?

Clarification (from cfg inspection):
  ✓ cfg nesting: one_vm_repeat.memory_sweep → indicates 1 VM
  ✓ vm_secure_guest_type = tdx → VM type is TDX
  ✓ start_mem = 1024 → per-VM baseline in MiB
  ✓ random window expands from 1024 within 32GB window
```

### Step 4: Infer Behavior & Intent
- Trace through test handler to understand execution:
  1. What VMs are created?
  2. In what sequence?
  3. What's measured/validated?
  4. What's the success criterion?

Example behavior inference:
```
Case: tdx_vmx2_from1024m_toall

Execution flow (inferred from cfg + test handler):
  1. Create 1 TDX CM with machine_type_extra_params="kernel-irqchip=split"
  2. Generate random memory sequence:
     - Start at 1024 MiB
     - Random offsets within a 32GB window ([0%-64%] of window)
     - Rounded to 511 MiB steps
     - Generate 2 samples per window
  3. For each memory value:
     - Override VM memory to this value
     - Boot VM (via env_process.preprocess_vm)
     - Run 20 iterations (repeating boot might mean here: boot, wait, destroy, repeat)
     - Destroy VM
  4. Final result: success if all boots completed without crash/hang

Test intent:
  - Validate TDX VM can boot across range of memory sizes
  - Ensure memory sweep generator produces valid sequences
  - Check VM stability under varying memory allocation
  - Catch memory-related bugs early (OOM, allocation failure, etc.)
```

### Step 5: Document Parameter Semantics
- For each parameter, explain:
  - What it controls
  - Expected value range
  - Unit (if numeric)
  - Interaction with other parameters
  - Rationale (why this parameter exists)

Example semantics doc:
```
Legacy Case: tdx_vmx2_from1024m_toall
Parameter Semantics:

Parameter: iterations
  - Meaning: Per-memory-value boot repeat count
  - Value in case: 20
  - Unit: count (iterations)
  - Range: 1-? (no explicit upper limit)
  - Interaction: Each memory sweep value tested 20 times
  - Rationale: Catch transient memory-related failures or race conditions

Parameter: mem_generator
  - Meaning: Algorithm for generating memory sequence
  - Value in case: random_32g_window
  - Unit: algorithm name (enum: random_32g_window, linear, etc.)
  - Range: handler-dependent
  - Interaction: Drives which parameters are used (random_* params vs. linear_*)
  - Rationale: Test system across diverse memory scenarios, not just fixed sizes

Parameter: start_mem
  - Meaning: Baseline memory (minimum value in sweep)
  - Value in case: 1024
  - Unit: MiB
  - Range: > 0 (must boot with some memory)
  - Interaction: Seed for random window; lower bound of sweep
  - Rationale: 1GB baseline matches common VM memory minimum

Parameter: random_min, random_max
  - Meaning: Window boundaries for random offset (% of window_size)
  - Value in case: 0, 64
  - Unit: percentage of window_size
  - Range: 0-100 (but random_min <= random_max)
  - Interaction: Define random offset range; combined with window_size
  - Rationale: Concentrate random samples in lower half of 32GB window
                (0-64% = up to ~20GB additional offset from 1GB baseline)

Parameter: window_size
  - Meaning: Size of random window (MiB)
  - Value in case: 32768
  - Unit: MiB (32GB)
  - Range: typically 32GB standard
  - Interaction: Entire random offset confined to this window
  - Rationale: 32GB is typical host memory budget; capped to prevent OOM

Parameter: machine_type_extra_params
  - Meaning: Extra QEMU machine type config (for VM architecture-specific needs)
  - Value in case: "kernel-irqchip=split"
  - Unit: string (QEMU machine arg)
  - Range: QEMU machine type options
  - Interaction: Required for TDX VMs; enables split IRQ chip handling
  - Rationale: TDX VMs need specialized IRQ chip config for security guarantees
```

### Step 6: Identify Assumptions & Constraints
- Document environmental or operational assumptions:
  - Host memory requirement
  - CPU features required (nested virtualization, TDX support, etc.)
  - Test duration/timeout expectations
  - Failure modes and how they're detected

Example assumptions doc:
```
Legacy Case: tdx_vmx2_from1024m_toall
Assumptions & Constraints:

Host Environment:
  - Host must have >= 32GB RAM available (to accommodate 32GB window)
  - CPU must support TDX (if any TDX VM variant in family)
  - Nested virtualization enabled on host (if testing nested scenarios)

Test Duration:
  - ~20 memory values (random sweep across 32GB window)
  - ~20 boot iterations per value
  - ~400 total VMs booted per test run
  - Expected runtime: 30-60 minutes (estimated)
  - Timeout: not explicitly documented; assume 2-3 hours per run

Success Criteria:
  - All 400 VMs boots succeed (no crash/hang)
  - No memory allocation failures
  - All VMs reach "ready" state (responsive to command)

Failure Modes:
  - "Out of memory" (host memory exhausted)
  - "VM hangs" (doesn't reach ready state)
  - "Kernel panic" (guest kernel error)
  - "QEMU crash" (VM process dies unexpectedly)

Known Limitations:
  - Memory sweep uses pseudo-random values (may not cover all edge cases)
  - Only tests boot; doesn't run workloads on booted VMs
  - Only tests TDX; doesn't test other VM types with large memory
```

### Step 7: Create Legacy Case Summary
- Output structured document summarizing all findings

Example summary:
```yaml
---
legacy_case:
  name: tdx_vmx2_from1024m_toall
  group: one_vm_repeat.memory_sweep
  location: vmm_tree/validation/kvm_pts/boot_repeat.cfg
  lines: 42-70

naming_breakdown:
  convention: "<vm_type><vm_count>_from<start_mem>_toall"
  tdx: TDX confidential VM type flag
  vmx2: unclear; likely variant indicator
  from1024m: memory sweep base of 1024 MiB
  toall: expand to maximum available memory

parameters:
  explicit_in_case:
    iterations: 20
    mem_generator: random_32g_window
    start_mem: 1024  # MiB
    random_min: 0  # % of window
    random_max: 64  # % of window
    random_unit: 511  # MiB step
    samples_per_window: 2
    window_size: 32768  # MiB (32GB)
  
  inherited_from_parent:
    type: multi_vms_multi_boot
    vm_secure_guest_type: tdx
    machine_type_extra_params: kernel-irqchip=split

behavior_summary: |
  Repeatedly boot a single TDX VM across a range of memory sizes
  (1GB baseline + random offsets within 32GB window, 2 samples, 511MiB steps).
  For each memory value, boot the VM 20 times and verify success.
  Validates TDX VM stability under varying memory allocation.

test_intent: |
  Catch memory-related bugs in TDX VM boot path:
  - Memory allocation failures
  - OOM handling
  - Memory request/response cycles
  - Transient failures under stress

host_requirements:
  min_free_memory: 32GB
  cpu_features: [TDX, nested_virtualization]
  
expected_duration: 30-60 minutes
expected_vm_boots: 400

risk_areas:
  - Memory resource contention (if host near limit)
  - Timing-dependent failures (race conditions in memory mgmt)
  - Platform-specific TDX support gaps
```

## Output

**Legacy Case Analysis Report** with:
1. Case location and naming breakdown
2. Extracted parameters (explicit + inherited)
3. Decoded naming convention
4. Inferred execution behavior
5. Parameter semantics (meaning, unit, range, interaction)
6. Host assumptions and constraints
7. Test intent and success criteria
8. Structured YAML summary for reference

## Common Patterns in vmm_tree Cases

### Pattern 1: Memory Sweep Cases
- **Naming**: `<vm_type>_from<start_size>_to<end_size>`
- **Intent**: Test VM boot across memory range
- **Execution**: Generate memory sequence, boot/destroy cycle per value
- **Duration**: Typically 30-60 min, 100+ boots per run

### Pattern 2: Repeat Iteration Cases
- **Naming**: `<vm_type>_<iteration_count>times`
- **Intent**: Catch transient/timing-dependent bugs
- **Execution**: Boot same config N times sequentially
- **Duration**: Typically 10-30 min per test

### Pattern 3: Multi-VM Scenario Cases
- **Naming**: `<vm_count><vm_type>_<operation>`
- **Intent**: Test coexistence/interaction of multiple VMs
- **Execution**: Create N VMs, run operations, verify interaction
- **Duration**: Varies (30 min - 3 hours)

### Pattern 4: Feature-Specific Cases
- **Naming**: `<feature_name>_<variant>`
- **Intent**: Validate specific CPU/hypervisor feature
- **Execution**: Enable feature, run workload, verify effect
- **Duration**: Typically 15-45 min per variant

## Related Skills
- **LKVS_PARAMETER_MAPPING_SKILL**: Uses findings to build mapping table
- **LKVS_CFG_TRANSLATION_SKILL**: Uses behavior understanding to write cfg
- **NAMING_NORMALIZATION_SKILL**: Applies naming conventions systematically
