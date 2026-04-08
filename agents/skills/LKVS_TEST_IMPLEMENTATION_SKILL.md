# LKVS Test Implementation Skill

Skill for implementing or verifying Python test handlers in LKVS to support migrated cases.

## When to Use

Use this skill when:
- Migrating a case requires cfg-only translation + Python verification (most common)
- Creating a new Python test handler for unsupported execution model
- Modifying existing test to add new behavior or parameters
- Validating that handler correctly processes cfg parameters

## Inputs

1. **Case Migration Context** (from CASE_MIGRATION_FRAMEWORK_AGENT)
   - Legacy test behavior and entry point
   - Target LKVS execution model (cfg type)
   - Parameter mapping table (from LKVS_PARAMETER_MAPPING_SKILL)

2. **Existing LKVS Test** (target Python file or template)
   - Current implementation (if updating)
   - Parameter expectations from handler code
   - Entry point signature: `def run(test, params, env):`

3. **Behavioral Requirements**
   - Test-observable behavior (sequence of actions, pass/fail criteria)
   - Iteration model (single run, repeated cycles, swept parameters)
   - VM creation/destruction flow

## Process

### Step 1: Select or Verify Execution Model
- **Cfg-only migration** (most cases):
  - Use existing test handler (no Python changes)
  - Verify handler reads all migrated parameters
  - Add cfg comments linking to legacy source
  - **Action**: Jump to Step 3 (verification)

- **New execution model required**:
  - Handler doesn't exist or semantics don't match
  - Implement new Python test or modify existing
  - **Action**: Proceed to Step 2 (implementation)

Decision tree:
```
Does existing handler match legacy behavior?
  ├─ YES: Use cfg-only
  │   └─ Verify: handler reads all params from mapping table
  │       └─ Add cfg comment linking to vmm_tree
  │       └─ Done!
  └─ NO: Need Python changes
      ├─ Is it a parameter addition (no behavior change)?
      │   └─ Add params to cfg; verify handler reads them
      │   └─ Done! (minimal change)
      └─ Is it a new algorithm/flow?
          └─ Implement new test or refactor existing
          └─ Proceed to Step 2
```

### Step 2: Implement Python Test Handler

#### 2.1 Anatomy of an LKVS Test

```python
"""
Test module docstring describing what the test does.
Key behaviors, parameters, and VM behavior explained here.
"""

import logging
from avocado.utils import process
from virttest import error_context
from virttest import env_process
from virttest import utils_misc
from provider import dmesg_router  # Side-effect: logging routing (keep even if unused)

LOG = logging.getLogger(__name__)


@error_context.context_aware
def run(test, params, env):
    """
    Test entry point.
    
    Docstring format (reStructuredText for Sphinx):
    - Describe test purpose and key behaviors
    - List main parameters used
    - Describe iteration model (1 run, repeated, swept, etc.)
    """
    # Implementation follows...
```

#### 2.2 Structure Template: Memory Sweep Test

For migrated memory-sweep cases, follow this structure:

```python
@error_context.context_aware
def run(test, params, env):
    """
    Sweep VM memory from start_mem to max available, boot/destroy cycles.
    
    Parameters:
        iterations: repeat count per memory value (int)
        mem_generator: sweep algorithm (e.g., 'random_32g_window', 'linear')
        start_mem: baseline memory in MiB (int)
        vms: comma-separated VM names (str, e.g., 'vm' or 'td td td')
        [random_min, random_max, random_unit, etc.]: generator-specific params
    """
    error_context.context("Preparing test environment", LOG.info)
    
    # 1. Extract parameters
    mem_generator = params.get("mem_generator")
    iterations = params.get_numeric("iterations", 1)
    start_mem = params.get_numeric("start_mem")
    vm_names = params.objects("vms")
    
    # Validate expected parameters exist
    if not mem_generator:
        test.error("Parameter 'mem_generator' must be specified")
    
    # 2. Initialize generator (if not built-in)
    memory_sequence = _get_memory_sequence(
        gen_type=mem_generator,
        start_mem=start_mem,
        params=params,
        test=test
    )
    
    vms_created = []
    try:
        for mem_value in memory_sequence:
            error_context.context(
                f"Booting {len(vm_names)} VMs with {mem_value} MiB each",
                LOG.info
            )
            
            # 3. Boot VMs with this memory value
            for vm_name in vm_names:
                # Use object_params to get per-VM configuration
                vm_params = params.object_params(vm_name)
                vm_params["mem"] = mem_value  # Override memory for sweep
                
                # Preprocess (boot) the VM
                env_process.preprocess_vm(test, vm_params, env, vm_name)
                vm = env.get_vm(vm_name)
                vms_created.append(vm)
            
            # 4. Run iteration cycles for this memory value
            for i in range(iterations):
                error_context.context(
                    f"Iteration {i+1}/{iterations} at {mem_value} MiB",
                    LOG.info
                )
                
                # Optional: verify VM is responsive
                try:
                    session = vm.wait_for_login()
                    session.close()
                    LOG.info(f"  VM responsive at memory {mem_value} MiB")
                except Exception as e:
                    test.fail(f"VM not responsive: {e}")
            
            # 5. Destroy VMs (prepare for next memory value)
            for vm in vms_created:
                vm.destroy(gracefully=False)
            vms_created = []
    
    finally:
        # 6. Cleanup: ensure all VMs destroyed
        for vm in vms_created:
            try:
                vm.destroy(gracefully=False)
            except Exception as e:
                LOG.warning(f"Cleanup failed for {vm.name}: {e}")


def _get_memory_sequence(gen_type, start_mem, params, test):
    """
    Generate memory sequence for sweep based on generator type.
    
    Yields:
        int: memory value in MiB
    """
    if gen_type == "random_32g_window":
        yield from _random_window_sequence(start_mem, params, test)
    elif gen_type == "linear":
        yield from _linear_sequence(start_mem, params)
    else:
        test.error(f"Unknown memory generator: {gen_type}")


def _random_window_sequence(start_mem, params, test):
    """Random 32GB window memory sweep."""
    import random
    
    # Extract window parameters
    window_size = params.get_numeric("window_size")  # MiB
    random_min = params.get_numeric("random_min")  # Percentage of window
    random_max = params.get_numeric("random_max")  # Percentage of window
    random_unit = params.get_numeric("random_unit")  # MiB step size
    samples_per_window = params.get_numeric("samples_per_window", 1)
    
    # Compute host memory limit
    host_mem = utils_misc.get_usable_memory_size() * 1024  # Convert GiB to MiB
    
    # Generate random values within window
    for sample in range(samples_per_window):
        # Random offset within [random_min%, random_max%] of window
        offset_pct = random.randint(random_min, random_max)
        offset_mib = int(window_size * offset_pct / 100)
        mem_value = start_mem + offset_mib
        
        # Clamp to host memory
        mem_value = min(mem_value, host_mem)
        mem_value = (mem_value // random_unit) * random_unit  # Align to step
        
        LOG.info(f"Generated memory value: {mem_value} MiB (sample {sample+1})")
        yield mem_value


def _linear_sequence(start_mem, params):
    """Linear memory sweep (fallback)."""
    step = params.get_numeric("mem_step", 1024)
    max_mem = params.get_numeric("mem_max")
    
    mem = start_mem
    while mem <= max_mem:
        yield mem
        mem += step
```

#### 2.3 Key Patterns in LKVS Test Code

**Pattern 1: Parameter Extraction with Type Safety**
```python
# Good: explicit type conversion
iterations = params.get_numeric("iterations", 1)
vm_names = params.objects("vms")
config_str = params.get("config", "default")

# Avoid: implicit string-to-int
iterations = int(params.get("iterations"))  # Fails if param missing
```

**Pattern 2: Per-VM Parameter Overrides**
```python
# Get base params for VM
vm_params = params.object_params(vm_name)

# Override specific param for this sweep value
vm_params["mem"] = new_memory_value

# Boot VM with overridden params
env_process.preprocess_vm(test, vm_params, env, vm_name)
```

**Pattern 3: Proper Cleanup in Finally Block**
```python
vms_created = []
try:
    # ... create and use VMs ...
    vms_created.append(vm)
finally:
    # Always cleanup, even if test fails
    for vm in vms_created:
        try:
            vm.destroy(gracefully=False)
        except Exception as e:
            LOG.warning(f"Cleanup error: {e}")
```

**Pattern 4: Error Context for Step Logging**
```python
error_context.context("Step description", LOG.info)
# Do work that might fail
# If it fails, error context adds "Step description" to failure log
```

### Step 3: Verify Python Test Reads All Migrated Parameters

Use grep/semantic search to confirm:

```bash
# List all parameters used in test
grep -E "params\.(get|get_numeric|objects)" KVM/qemu/tests/<handler>.py

# Check for specific migrated params
grep -E "(mem_generator|start_mem|random_min|random_max)" KVM/qemu/tests/<handler>.py
```

Create verification table:
```
Parameter | Legacy | Cfg Mapping | Python Handler | Used By | Status
----------|--------|--------|--------|--------|--------
iterations | 20 | ✓ (iterations=20) | params.get_numeric("iterations") | boot loop | ✓ OK
mem_generator | random_32g_window | ✓ | params.get("mem_generator") | _get_memory_sequence() | ✓ OK
start_mem | 1024 | ✓ | params.get_numeric("start_mem") | _random_window_sequence() | ✓ OK
random_min | 0 | ✓ | params.get_numeric("random_min") | _random_window_sequence() | ✓ OK
... | ... | ... | ... | ... | ...
```

If parameter is in cfg but not read by handler:
- **Acceptable if**: parameter is for a different test type (different cfg `type`)
- **Action required if**: parameter is required for this test (add to handler or document why unused)

### Step 4: Add Licensing & Documentation

For new files (or new functions), add LKVS-required headers:

```python
# SPDX-License-Identifier: GPL-2.0-only
# Copyright (c) 2024 Intel Corporation

"""
Test module for memory sweep validation.

Migrated from vmm_tree/validation/kvm_pts/boot_repeat.cfg (case: tdx_vmx2_from1024m_toall)

This test repeatedly boots VMs with varying memory sizes, validating that:
- VM boot succeeds at each memory value
- Memory limits are respected (not exceeding available host memory)
- TDX confidential VMs boot with required kernel-irqchip=split settings
"""
```

### Step 5: Validate & Add Comments

Add cfg comment linking to legacy source:

```
KVM/qemu/boot_repeat.cfg:

    - 1td:
        - from1G_toall:
            # Migrated from: vmm_tree/validation/kvm_pts/boot_repeat.cfg::tdx_vmx2_from1024m_toall
            # Behavior: TDX VM, repeated boot from 1GB to all available memory
            # Implementation: multi_vms_multi_boot.py with random window sweep
            type = multi_vms_multi_boot
            # ... parameters ...
```

## Output

**Modified or New Python Test** with:
- [ ] Entry point `def run(test, params, env)` with `@error_context.context_aware`
- [ ] All cfg parameters extracted and used
- [ ] Proper error handling and cleanup (finally blocks)
- [ ] SPDX + Copyright header
- [ ] Docstring explaining test purpose
- [ ] Links to legacy source (in comments)
- [ ] No lint errors (pre-commit should pass)

## Common Pitfalls

### Pitfall 1: Missing Parameter Extraction
```python
# ✗ Bad: reads parameter but doesn't handle missing case
mem = int(params["mem"])  # Crashes if param missing

# ✓ Good: uses default or validates existence
mem = params.get_numeric("mem", 1024)  # Uses default1024
```

### Pitfall 2: Not Using per-VM Parameter Overrides
```python
# ✗ Bad: modifies global params dict
params["mem"] = new_value
vm = env.get_vm("vm")  # Impacts all subsequent VMs

# ✓ Good: per-VM override
vm_params = params.object_params("vm")
vm_params["mem"] = new_value
env_process.preprocess_vm(test, vm_params, env, "vm")
```

### Pitfall 3: Incomplete Cleanup
```python
# ✗ Bad: VMs not cleaned if loop breaks
for vm_name in vm_names:
    vm = env.get_vm(vm_name)
    # ... use VM ...
    vm.destroy()  # Never reached if exception thrown

# ✓ Good: cleanup guaranteed
vms = []
try:
    for vm_name in vm_names:
        vm = env.get_vm(vm_name)
        vms.append(vm)
        # ... use VM ...
finally:
    for vm in vms:
        vm.destroy(gracefully=False)
```

### Pitfall 4: Side-Effect Import Removal
```python
# ✗ Bad: removing side-effect import breaks logging routing
- from provider import dmesg_router

# ✓ Good: keep if it's active in test environment
from provider import dmesg_router  # pylint: disable=unused-import
```

## Related Skills
- **LKVS_PARAMETER_MAPPING_SKILL**: Provides parameter mapping table
- **LKVS_CFG_TRANSLATION_SKILL**: Produces cfg that drives test execution
- **VMM_TREE_LEGACY_CASE_ANALYSIS_SKILL**: Explains legacy behavior to replicate
