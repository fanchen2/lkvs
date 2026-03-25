---
name: Case Migration Quick Start
description: Hands-on guide to migrate test cases from vmm_tree to LKVS
related_skills:
  - ../skills/VMWARE_LEGACY_CASE_ANALYSIS_SKILL.md
  - ../skills/LKVS_PARAMETER_MAPPING_SKILL.md
  - ../skills/NAMING_NORMALIZATION_SKILL.md
  - ../skills/LKVS_CFG_TRANSLATION_SKILL.md
  - ../skills/LKVS_TEST_IMPLEMENTATION_SKILL.md
related_agents:
  - ../agents/case_migration_framework.agent.md
related_docs:
  - CASE_MIGRATION_ARCHITECTURE.md
---

# Case Migration Framework - Quick Start Guide

## Framework Overview

This framework provides a systematic approach to migrate virtualization test cases from `vmm_tree` (legacy infrastructure) to LKVS (modern test provider), maintaining behavior fidelity while adopting LKVS conventions.

### Key Components

```
agents/case_migration_framework.agent.md       ← Main coordination agent
└── Orchestrates the full migration workflow
    ├── Phase 1: Case Analysis & Mapping
    ├── Phase 2: Configuration Translation
    ├── Phase 3: Python Implementation/Routing
    └── Phase 4: Validation & Commit

Supported Skills (specializations):
├── skills/VMWARE_LEGACY_CASE_ANALYSIS_SKILL        ← Decode legacy case semantics
├── skills/LKVS_PARAMETER_MAPPING_SKILL             ← Build parameter equivalence table
├── skills/NAMING_NORMALIZATION_SKILL               ← Standardize case names
├── skills/LKVS_CFG_TRANSLATION_SKILL               ← Translate to cartograph format
└── skills/LKVS_TEST_IMPLEMENTATION_SKILL           ← Implement/verify Python tests
```

## Quick Start: Migrating a Single Case

### Scenario: Migrate legacy case `tdx_vmx2_from1024m_toall` to LKVS

**Time estimate**: 1-2 hours per case (includes research + implementation)

### Step 1: Analyze the Legacy Case (30 min)
**Use Skill**: [VMWARE_LEGACY_CASE_ANALYSIS_SKILL](../skills/VMWARE_LEGACY_CASE_ANALYSIS_SKILL.md)

```bash
# Find legacy case in vmm_tree
grep -r "tdx_vmx2_from1024m_toall" vmm_tree/

# Read and understand legacy cfg structure
cat vmm_tree/validation/kvm_pts/boot_repeat.cfg | grep -A 20 "tdx_vmx2_from1024m_toall"

# Document findings:
# - What parameters does this case set?
# - What's the VM type and count?
# - What's the behavior (memory sweep, iterations, etc.)?
# - What's the execution model (cfg type)?
```

**Output**: Legacy case semantics document (saved locally or in agent notes)

### Step 2: Map Parameters to LKVS (30 min)
**Use Skill**: [LKVS_PARAMETER_MAPPING_SKILL](../skills/LKVS_PARAMETER_MAPPING_SKILL.md)

```bash
# Locate LKVS test handler (target test file)
# E.g., for multi_vms_multi_boot execution type:
cat lkvs/KVM/qemu/tests/multi_vms_multi_boot.py | grep "params.get"

# Create mapping table:
# legacy_param → lkvs_param, with unit conversion if needed
# E.g.:
#   start_mem (1024 MiB) → start_mem (1024 MiB)  [no change]
#   from1024m → naming convention shows unit already in name
```

**Output**: Parameter mapping table (with conversions documented)

### Step 3: Decide Implementation Path (10 min)
**Question**: Does existing Python handler support all migrated parameters?

- **YES** → Proceed to cfg-only migration (Step 4A)
- **NO** → Need to add/update Python handler (Step 4B)

Most cases follow **cfg-only** path: new cases are handled by existing handlers.

### Step 4A: Cfg-Only Migration (20 min)
**Use Skill**: [LKVS_CFG_TRANSLATION_SKILL](../skills/LKVS_CFG_TRANSLATION_SKILL.md)

```bash
# 1. Choose target cfg file (usually boot_repeat.cfg)
# 2. Decide where case fits in hierarchy
#    (e.g., boot_repeat.one_vm_repeat.memory_sweep.1td)
# 3. Inline parameters from mapping table
# 4. Validate formatting (indentation, no trailing whitespace)
# 5. Add cfg comment linking to legacy source

# Result in lkvs/KVM/qemu/boot_repeat.cfg:
    - one_vm_repeat:
        - memory_sweep:
            - 1td:
                - from1G_toall:
                    # Migrated from: vmm_tree/validation/kvm_pts/boot_repeat.cfg
                    type = multi_vms_multi_boot
                    iterations = 20
                    mem_generator = random_32g_window
                    start_mem = 1024
                    # ... other params ...
```

**Verification**:
```bash
# Check cfg syntax
grep -A 10 "from1G_toall" lkvs/KVM/qemu/boot_repeat.cfg

# Check for indentation issues
grep "^  [^ ]" lkvs/KVM/qemu/boot_repeat.cfg  # Should find nothing (no tab indents)

# Verify handler exists and reads required params
grep "mem_generator" lkvs/KVM/qemu/tests/multi_vms_multi_boot.py
```

### Step 4B: Python Implementation (if needed)
**Use Skill**: [LKVS_TEST_IMPLEMENTATION_SKILL](../skills/LKVS_TEST_IMPLEMENTATION_SKILL.md)

(Rare case; most migrations don't require this)

```python
# Example: If you need to add custom parameter handling
# 1. Add entry point function with @error_context.context_aware
# 2. Extract cfg parameters
# 3. Implement test logic
# 4. Ensure cleanup in finally block
# 5. Add SPDX + Copyright header
```

### Step 5: Validate & Commit (15 min)

```bash
# Pre-commit validation
cd lkvs
pre-commit run --files KVM/qemu/boot_repeat.cfg KVM/qemu/tests/multi_vms_multi_boot.py

# Commit with migration reference
git add KVM/qemu/boot_repeat.cfg
git commit -s -m "KVM/qemu: migrate memory sweep cases from vmm_tree

Migrate legacy boot_repeat cases to LKVS boot_repeat.cfg:
  vmm_tree::tdx_vmx2_from1024m_toall → lkvs::boot_repeat.one_vm_repeat.memory_sweep.1td.from1G_toall

Parameter mapping:
  iterations: 20 → 20 (1:1)
  start_mem: 1024 MiB → 1024 (1:1)
  mem_generator: random_32g_window → random_32g_window (1:1)
  [other params...]

Behavior preserved: TDX VM boot with random memory sweep (1GB baseline, 32GB window).

Migrated from: vmm_tree/validation/kvm_pts/boot_repeat.cfg"
```

## Batch Migration: Multiple Cases

**Scenario**: Migrate 5 related cases to standardize memory sweep family

**Time estimate**: 3-4 hours total (reduced per-case overhead)

### Strategy: Establish Naming Schema Once, Apply Consistently

1. **Analyze all 5 cases together** → Identify common pattern
2. **Define naming schema** with `NAMING_NORMALIZATION_SKILL`
3. **Create mapping table for all 5** → Document in single reference
4. **Translate all cfg at once** → Efficient bulk update
5. **Single validation pass** → Pre-commit runs once on full updated file
6. **Single commit** → Reference all 5 migrations

Example batch mapping table:
```
Legacy Name            | LKVS Target                              | Status
-----------------------|------------------------------------------|-------
tdx_vmx2_from1024m..   | boot_repeat.one_vm_repeat.memory_sweep.1td.from1G_toall | ✓ mapped
vm_single_from1024m..  | boot_repeat.one_vm_repeat.memory_sweep.1vm.from1G_toall | ✓ mapped
4td_from1024m_toall    | boot_repeat.multi_vms_repeat.memory_sweep.4td.from1G_toall | ✓ mapped
[2 more...]            | [mapped]                                 | ✓ mapped
```

## Risk Mitigation Checklist

Before committing any migration, verify:

- [ ] **Parameter fidelity**: Every legacy parameter has LKVS equivalent; no silent drops
- [ ] **Behavior preservation**: Test observable behavior matches legacy (same memory range, iteration count, VM type)
- [ ] **Naming stability**: Case names won't change again (well-established schema)
- [ ] **No conflicts**: New names don't clash with existing LKVS cases
- [ ] **Cfg syntax**: No indentation errors, no trailing whitespace
- [ ] **Handler verification**: Confirmed Python test reads all cfg parameters
- [ ] **Licensing**: New files have GPL-2.0-only header + Intel copyright
- [ ] **Pre-commit pass**: All linters/validators pass
- [ ] **Traceability**: Commit message and cfg comments link to vmm_tree source

## Troubleshooting

### Issue: "Parameter not found in LKVS handler"
- **Cause**: Mapped parameter not in target test
- **Solution**: Check if parameter should be in different test; or add to handler
- **Decision**: Cfg-only won't work; need Python changes

### Issue: "Cfg indentation mismatch with existing file"
- **Cause**: Mixed tabs/spaces or wrong indent level
- **Solution**: Use `sed` or editor to fix (prefer 4-space indent)
- **Validation**: `grep "^  [^ ]"` should find nothing

### Issue: "Legacy case name too long or ambiguous"
- **Cause**: Naming doesn't follow established schema
- **Solution**: Refactor with `NAMING_NORMALIZATION_SKILL`
- **Risk**: Longer migrations; more validation needed

### Issue: "Memory units differ (MiB vs GiB)"
- **Cause**: Legacy uses MiB; LKVS cfg uses different units in certain contexts
- **Solution**: Document unit conversion in cfg comment
- **Validation**: Cross-check with test output logs

### Issue: "No test handler for cfg type"
- **Cause**: Cfg references non-existent test file
- **Solution**: Use existing handler or implement new one
- **Timeline**: Add 1-2 hours for new test implementation

## When to Use This Framework

✓ **Good fit**:
- Migrating specific test cases from vmm_tree to LKVS
- Establishing naming conventions for LKVS case families
- Batch migrations with clear semantics (e.g., "all memory sweep cases")

✗ **Not ideal for**:
- Porting test cases between unrelated test frameworks
- Rewriting test logic (choose direct Python dev instead)
- Creating entirely new test suites (use LKVS guides directly)

## Integration with Existing Workflows

### How This Relates to Other Agents

- **boot_repeat_case_refactor.agent.md**: Focuses on cfg hierarchy within LKVS (this framework feeds into it)
- **multi_vms_memory_sweep.agent.md**: Aligns specific sweep implementations (this framework may reference it)

### Commit Message Style

All migrations should reference:
1. Source: `vmm_tree/validation/.../filename`
2. Target: `lkvs/KVM/qemu/boot_repeat.cfg` (or relevant file)
3. Semantics: Brief description of what test does
4. Changes: List of legacy→LKVS mappings

## Documentation & Future Reference

- **Mapping tables** stored in commit messages and cfg comments
- **Naming schemas** documented in framework (reuse for future cases)
- **Parameter conversions** captured in LKVS_PARAMETER_MAPPING_SKILL examples
- **Common pitfalls** updated as new cases reveal issues

## Key Takeaways

1. **Standardized workflow** reduces case-to-case variance and validation cycles
2. **Skills handle specialized tasks** (parameter mapping, cfg translation), freeing agent for orchestration
3. **Cfg-first approach** (when possible) minimizes Python changes and risk
4. **Traceability** (commits linking to vmm_tree) aids future maintenance
5. **Batch operations** more efficient than single-case migrations

---

For detailed guidance on specific skills or phases:
- Refer to skill files in [`../skills/`](../skills/)
- Review [Architecture & Structure](CASE_MIGRATION_ARCHITECTURE.md) for framework overview
- Check [Main Agent](../agents/case_migration_framework.agent.md) for orchestration details

Questions or issues? Consult the troubleshooting section or update this guide with new patterns discovered.
