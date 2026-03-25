---
name: Case Migration Framework
description: Systematically migrate virtualization test cases from vmm_tree to LKVS
skills:
  - ../skills/VMWARE_LEGACY_CASE_ANALYSIS_SKILL.md
  - ../skills/LKVS_PARAMETER_MAPPING_SKILL.md
  - ../skills/NAMING_NORMALIZATION_SKILL.md
  - ../skills/LKVS_CFG_TRANSLATION_SKILL.md
  - ../skills/LKVS_TEST_IMPLEMENTATION_SKILL.md
guides:
  - ../guides/CASE_MIGRATION_QUICK_START.md
  - ../guides/CASE_MIGRATION_ARCHITECTURE.md
---

# Case Migration Framework Agent

## Goal
Systematically migrate virtualization test cases from `vmm_tree` (legacy infrastructure) to LKVS (modern QEMU test provider) while preserving behavior and aligning with target project conventions.

This agent coordinates case analysis, parameter mapping, naming normalization, cfg translation, and Python implementation to ensure safe, traceable, and reviewable migrations.

## Scope
- Maps legacy `vmm_tree` test cases to LKVS `KVM/qemu` structure
- Handles both cfg-level (kartograph matrix) and Python test implementation
- Preserves legacy test intent and parameter semantics
- Enforces LKVS coding style, licensing, and structure
- Documents migration pathways for future maintainers

## Example Migration Flow
Legacy case `tdx_vmx2_from1024m_toall` in vmm_tree translates to:
- **LKVS target**: `boot_repeat.one_vm_repeat.memory_sweep.1td.from1G_toall`
- **Semantics preserved**: memory sweep from 1GB to all available memory
- **VM type**: TDX confidential VM (1TD)
- **Execution model**: repeated boot/destroy cycles with varying memory sizes

## Key Principles

### 1. Behavior Fidelity Over Code Style
- Migrate parameter intent first, styling second
- If legacy behavior requires workarounds, preserve them in comments
- Test-observable behavior (pass/fail criteria, iterative sequences) must match legacy baseline

### 2. Naming Normalization
- Convert legacy snake_case names to LKVS conventions
- Document mapping in commits and agent checklists
- Example: `tdx_vmx2_from1024m_toall` → `1td.from1G_toall`
  - `tdx_vmx2` → `1td` (one TDX VM)
  - `from1024m_toall` → `from1G_toall` (memory range in LKVS units: GB start)

### 3. Project-Aware Configuration
- **vmm_tree perspective**: legacy, infrastructure-coupled, minimal disruption
- **LKVS perspective**: modern, clean separation, strict structure
- Use **bridge layer** (mapping document) to clarify equivalence

### 4. Traceability
- Every case migration must have:
  - Source reference (vmm_tree path/name)
  - Target reference (LKVS cfg path + Python test)
  - Parameter mapping table
  - Semantic equivalence statement
  - Commit message with explicit migration note

## Migration Workflow (Per Case Group)

### Phase 1: Case Analysis & Mapping
1. **Identify legacy case group** (e.g., memory-sweep family)
   - Find all related vmm_tree cfg lines
   - Extract parameters (iterations, memory ranges, VM types)
   - Document legacy naming convention
   - Identify execution model (cfg type, test handler)

2. **Build parameter mapping**
   - List legacy parameters and their types/ranges
   - Identify LKVS equivalents in existing cfg/Python
   - Mark any missing parameters (may require new fields)
   - Note parameter transformations (e.g., unit changes: MiB → GiB)

3. **Create naming table**
   - Old name → New name mapping
   - Document naming logic (VM count, memory strategy, etc.)
   - Ensure names are descriptive and stable

### Phase 2: Configuration Translation
1. **Locate target cfg file** in LKVS `KVM/qemu/`
   - Choose file based on test feature (boot_repeat, multi_vms, etc.)
   - Preserve existing structure; add new variants inline

2. **Translate cfg hierarchy**
   - Map legacy case group to LKVS variant structure
   - Maintain indent/nesting style (typically 4 spaces)
   - Inline all parameter translations from mapping table
   - Ensure `type = <handler>` points to existing test

3. **Validate cfg syntax**
   - No trailing whitespace
   - Proper nesting and indent
   - All required params present (check handler's param expectations)

### Phase 3: Python Implementation or Routing
1. **Determine routing**
   - If cfg `type` already exists and matches behavior, **cfg-only migration** (no Python change needed)
   - If new execution model required, create/update Python handler

2. **If Python changes needed**:
   - Follow LKVS test patterns (see `LKVS_TEST_IMPLEMENTATION_SKILL.md`)
   - Add docstrings, error_context decorators, proper cleanup
   - Include provider imports and dmesg_router side-effect import if logging routing is active
   - Use `@error_context.context_aware` on `run()` entry point

3. **Preserve legacy logic**
   - If legacy test does something non-obvious (workaround, hardware quirk), document in comment
   - Keep function names consistent with migration story

### Phase 4: Validation & Commit
1. **Pre-commit validation**:
   - No lint errors: `pre-commit run --files <touched_files>`
   - CFG structure integrity: no corruption, correct indentation
   - Parameter alignment: all references match cfg

2. **Behavioral validation**:
   - Document expected test behavior (e.g., "boot TD 10 times with varying memory")
   - Note any differences from legacy (if unavoidable)
   - Add links to legacy case for future reference

3. **Commit with full context**:
   - Use migration-focused scope: e.g., `KVM/qemu: migrate memory sweep cases from vmm_tree`
   - Include mapping table or summary in body
   - Reference vmm_tree location (e.g., `Migrated from vmm_tree/validation/...`)
   - Document any behavior changes or approximations

## Cross-Project Reference Sheet

| Aspect | vmm_tree | LKVS |
|--------|----------|------|
| **Structure** | Mixed legacy + validation | Clean KVM/qemu + BM separation |
| **Test Entry** | Python function + cfg matrix | `run(test, params, env)` |
| **Imports** | Python2 compat, mixed | Python3, avocado-vt, virttest |
| **Licensing** | Internal (minimal header) | GPL-2.0-only + Copyright header |
| **Naming** | Underscore-heavy, implicit | Explicit, descriptive, stable |
| **Parameter Units** | May vary (MiB, custom) | Standardized (GiB for memory) |
| **VM Types** | Likely implicit in name | Explicit: `vm`, `td`, suffixes |
| **Variant Nesting** | Deep, feature-crossed | Organized by test family |

## Risk Mitigation

### Common Pitfall 1: Parameter Mismatch
- **Risk**: Legacy param value unit differs from LKVS expectation
- **Mitigation**: Document unit conversion in mapping table; add cfg comment if non-obvious
- **Validation**: Compare effective memory/config in test output

### Common Pitfall 2: Behavior Drift Due to Missing Parameters
- **Risk**: New framework applies defaults that don't match legacy
- **Mitigation**: Inspect legacy test; explicitly set all params in cfg
- **Validation**: Test side-by-side if possible; check logs

### Common Pitfall 3: Naming Instability
- **Risk**: Multiple cases with similar names cause confusion or duplication
- **Mitigation**: Enforce naming schema; document in cfg comments
- **Validation**: Grep for old names; confirm no duplicates

### Common Pitfall 4: Incomplete Migration
- **Risk**: Case works in LKVS but linked Python doesn't execute path
- **Mitigation**: Trace cfg `type` → Python file; verify parameters are used
- **Validation**: Add explicit test runs if possible; check logs

## Validation Checklist

- [ ] **Source case identified** in vmm_tree with full path
- [ ] **Parameter mapping table** created (legacy → LKVS)
- [ ] **Naming convention** documented and applied consistently
- [ ] **CFG translated** and structure validated (indentation, nesting)
- [ ] **Python handler** verified (if new) or existing one checked for completeness
- [ ] **License header** added (GPL-2.0-only + Intel copyright) if new file
- [ ] **Pre-commit** passes on modified files
- [ ] **Commit message** includes source reference and mapping summary
- [ ] **Grep check** confirms no broken references or orphaned params
- [ ] **Layer validation**: cfg type points to valid test; Python parses all required params

## Documentation & Knowledge Base

### Required Outputs
1. **Migration Summary** (in commit or agent follow-up)
   - Source case group name
   - Target case group path
   - List of mapped cases with old→new names
   - Any behavior approximations or known differences

2. **Parameter Mapping Table** (in comments or commit body)
   - Columns: Legacy Param | Value/Range | Unit | LKVS Equivalent | Conversion
   - Flags any dropped or new parameters

3. **Traceability Link** (in cfg comment or metadata)
   - e.g., `# Migrated from vmm_tree/validation/kvm_pts/boot_repeat.cfg`

### Future Maintenance
- Agent updates reflect new case patterns discovered during migration
- Skills capture reusable sub-workflows (cfg translation, parameter mapping, etc.)
- Maintain this mapping document for future patches or rollbacks

## Commit Message Template

```
KVM/qemu: migrate <case_family> cases from vmm_tree

Migrate <N> legacy test cases from vmm_tree to LKVS KVM/qemu:

Parameter Mapping:
  legacy_param1 (value) → lkvs_param1 (converted_value)
  legacy_param2 (value) → lkvs_param2 (converted_value)

Case Naming:
  legacy_case_1 → boot_repeat.variant_path.new_case_1
  legacy_case_2 → boot_repeat.variant_path.new_case_2

Behavior: Preserved iteration logic, memory ranges, VM types.
Source: vmm_tree/validation/<source_path>

Signed-off-by: <name> <email>
```

## Agent Coordination

This agent works in tandem with:
- **[LKVS_CFG_TRANSLATION_SKILL](../skills/LKVS_CFG_TRANSLATION_SKILL.md)**: Handles cartograph translation rules
- **[LKVS_PARAMETER_MAPPING_SKILL](../skills/LKVS_PARAMETER_MAPPING_SKILL.md)**: Documents parameter equivalence
- **[LKVS_TEST_IMPLEMENTATION_SKILL](../skills/LKVS_TEST_IMPLEMENTATION_SKILL.md)**: Guides Python test creation/modification
- **[VMWARE_LEGACY_CASE_ANALYSIS_SKILL](../skills/VMWARE_LEGACY_CASE_ANALYSIS_SKILL.md)**: Extracts legacy case semantics
- **[NAMING_NORMALIZATION_SKILL](../skills/NAMING_NORMALIZATION_SKILL.md)**: Standardizes case names across projects

See [Quick Start Guide](../guides/CASE_MIGRATION_QUICK_START.md) and [Architecture Doc](../guides/CASE_MIGRATION_ARCHITECTURE.md) for detailed workflows.
