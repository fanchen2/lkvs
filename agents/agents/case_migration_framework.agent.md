---
name: Case Migration Framework
description: Systematically migrate virtualization test cases from vmm_tree to LKVS
---

# Case Migration Framework Agent

## Linked Resources
- Skills:
   - [VMM_TREE_LEGACY_CASE_ANALYSIS_SKILL](../skills/VMM_TREE_LEGACY_CASE_ANALYSIS_SKILL.md)
   - [LKVS_PARAMETER_MAPPING_SKILL](../skills/LKVS_PARAMETER_MAPPING_SKILL.md)
   - [NAMING_NORMALIZATION_SKILL](../skills/NAMING_NORMALIZATION_SKILL.md)
   - [LKVS_CFG_TRANSLATION_SKILL](../skills/LKVS_CFG_TRANSLATION_SKILL.md)
   - [LKVS_TEST_IMPLEMENTATION_SKILL](../skills/LKVS_TEST_IMPLEMENTATION_SKILL.md)
- Guides:
   - [CASE_MIGRATION_QUICK_START](../guides/CASE_MIGRATION_QUICK_START.md)
   - [CASE_MIGRATION_ARCHITECTURE](../guides/CASE_MIGRATION_ARCHITECTURE.md)
   - [CASE_MIGRATION_CASE_PATTERNS](../guides/CASE_MIGRATION_CASE_PATTERNS.md)

## Goal
Systematically migrate virtualization test cases from `vmm_tree` (legacy infrastructure) to LKVS (modern QEMU test provider) while preserving behavior and aligning with target project conventions.

This agent coordinates case analysis, parameter mapping, naming normalization, cfg translation, and Python implementation to ensure safe, traceable, and reviewable migrations.

## Scope
- Maps legacy `vmm_tree` test cases to LKVS `KVM/qemu` structure
- Handles both cfg-level (kartograph matrix) and Python test implementation
- Preserves legacy test intent and parameter semantics
- Enforces LKVS coding style, licensing, and structure
- Documents migration pathways for future maintainers

## Case-Specific References
- Keep case-family routing, naming quirks, and handler-specific constraints in companion guides instead of the core framework.
- Promote a lesson back into this framework only after it has been validated across multiple migrations or clearly applies beyond one case family.
- Use [CASE_MIGRATION_CASE_PATTERNS](../guides/CASE_MIGRATION_CASE_PATTERNS.md) for extracted notes such as boot-repeat memory sweeps and large multi-VM scaling cases.
- Read TD-extension guidance only when the task explicitly asks to add a TD variant for an existing VM case; do not pull TD-specific rules into unrelated migrations by default.

## Key Principles

### 1. Behavior Fidelity Over Code Style
- Migrate parameter intent first, styling second
- If legacy behavior requires workarounds, preserve them in comments
- Test-observable behavior (pass/fail criteria, iterative sequences) must match legacy baseline

### 1.1 Mandatory Post-Change Review
- Every code change must be reviewed immediately after editing.
- Review focus: behavior correctness, parameter usage consistency, and side-effect scope.
- Review must explicitly check for duplicated logic and redundant branches.
- If duplicate/redundant code is found, refactor to a single clear path before finalizing.
- Default preference is concise code: keep implementation minimal while preserving readability.

### 2. Naming Normalization
- Convert legacy snake_case names to LKVS conventions
- Document mapping in commits and agent checklists
- Normalize names around stable semantic segments such as VM topology, execution mode, and parameter intent.
- When units are normalized during renaming, record the old and new units in the mapping table so the name change remains traceable.

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
1. **Identify legacy case group**
   - Find all related vmm_tree cfg lines
   - Extract parameters (iterations, memory ranges, VM types)
   - Document legacy naming convention
   - Identify execution model (cfg type, test handler)

2. **Locate case definition by case name (do not start with repo-wide grep)**
   - Use `validation/xvs/src/src/TestSuites/<suite>/tet_scen` as the source of truth.
   - Search exact case name in `tet_scen`, then read the next non-comment line to get `/suite/ts1.sh{N}`.
   - Open `validation/xvs/src/src/TestSuites/<suite>/<suite>/ts1.sh`, map `{N}` to `icN`, then map `icN` to `tpN`.
   - If the case has per-case inputs, check `validation/xvs/src/src/TestSuites/<suite>/<suite>/param.json` with the same case key.
   - Use repo-wide grep only as a fallback when suite information is genuinely unknown.

3. **Build parameter mapping**
   - List legacy parameters and their types/ranges
   - Identify LKVS equivalents in existing cfg/Python
   - Mark any missing parameters (may require new fields)
   - Note parameter transformations (e.g., unit changes: MiB → GiB)

4. **Create naming table**
   - Old name → New name mapping
   - Document naming logic (VM count, memory strategy, etc.)
   - Ensure names are descriptive and stable

### Phase 2: Configuration Translation
1. **Locate target cfg file** in LKVS `KVM/qemu/`
   - Choose file based on test feature (boot_repeat, multi_vms, etc.)
   - Preserve existing structure; add new variants inline
   - If the request explicitly includes adding a TD version for a VM case, consult the LKVS repository instruction for VM-to-TD extension rules before editing cfg.

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
   - Code review pass: no unnecessary duplication, no dead/redundant logic

2. **Behavioral validation**:
   - Document expected test behavior in observable terms such as iteration count, VM type, variable under test, and cleanup expectations.
   - Note any differences from legacy (if unavoidable)
   - Add links to legacy case for future reference

3. **Commit with full context**:
   - Use migration-focused scope: e.g., `KVM/qemu: migrate <case_family> cases from vmm_tree`
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

### Common Pitfall 5: Parameter Inheritance in cfg
- **Risk**: Custom parameter names (e.g., `guest_mem`) don't apply to all auto-generated VMs
- **Mitigation**: Use standard LKVS parameters (e.g., `mem`) and ensure runtime per-VM overrides are built from resolved VM list
- **Example**: In dynamic multi-VM flows, prefer inherited parameters such as `mem` for shared boot settings and generate per-VM overrides only when the case truly needs them
- **Pattern**: VM-specific params like `mem_vm1`, `mem_vm2` require explicit cfg entries; generic params like `mem` auto-inherit
- **Validation**: For dynamic VM list, test that all VMs (vm1, vm2, vm3+) receive intended memory value

### Reusable Lessons From Scaled Multi-VM Cases

1. **Separate Runtime Values From Sizing Values**
   - Keep the boot-time parameter distinct from any derived value used for capacity planning or guest-count calculation.

2. **Centralize Derived-Value Logic**
   - Put sizing or auto-calculation logic in one helper path and let the main run flow consume its result instead of repeating the math.

3. **Apply Safety Bounds Explicitly**
   - Cap automatically derived counts or sizes with explicit limits so the case stays predictable on large hosts.

4. **Keep Host Tuning Process-Scoped**
   - Apply temporary host resource tuning only inside the test process and always restore it in `finally`.

5. **Use Explicit Parameter Names**
   - Prefer names that distinguish actual runtime settings from derived planning inputs.

## Validation Checklist

- [ ] **Source case identified** in vmm_tree with full path
- [ ] **Parameter mapping table** created (legacy → LKVS)
- [ ] **Naming convention** documented and applied consistently
- [ ] **CFG translated** and structure validated (indentation, nesting)
- [ ] **Python handler** verified (if new) or existing one checked for completeness
- [ ] **Post-edit code review done** (correctness + side effects + readability)
- [ ] **Duplication minimized** (no repeated logic blocks when one shared path is enough)
- [ ] **Code kept concise** (remove redundant branches/variables/comments)
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
KVM: add <case_name> case to <test_family>

Brief description of new case behavior and variants.

Implementation details (if applicable):
  - Extended handler: <handler_name> with <mode_flag>
  - Supports: vm (standard), td (TDX), etc.

Signed-off-by: <name> <email>
```

**Template Notes**:
- Use "KVM:" scope for KVM/qemu test additions
- Focus on "add" (new capability) rather than "migrate" or "fix"
- Keep body brief; emphasize new feature, not legacy lineage
- Mention handler extension or implementation pattern used
- List supported variant types (vm, td, etc.)

## Agent Coordination

This agent works in tandem with:
- **[LKVS_CFG_TRANSLATION_SKILL](../skills/LKVS_CFG_TRANSLATION_SKILL.md)**: Handles cartograph translation rules
- **[LKVS_PARAMETER_MAPPING_SKILL](../skills/LKVS_PARAMETER_MAPPING_SKILL.md)**: Documents parameter equivalence
- **[LKVS_TEST_IMPLEMENTATION_SKILL](../skills/LKVS_TEST_IMPLEMENTATION_SKILL.md)**: Guides Python test creation/modification
- **[VMM_TREE_LEGACY_CASE_ANALYSIS_SKILL](../skills/VMM_TREE_LEGACY_CASE_ANALYSIS_SKILL.md)**: Extracts legacy case semantics
- **[NAMING_NORMALIZATION_SKILL](../skills/NAMING_NORMALIZATION_SKILL.md)**: Standardizes case names across projects

See [Quick Start Guide](../guides/CASE_MIGRATION_QUICK_START.md) and [Architecture Doc](../guides/CASE_MIGRATION_ARCHITECTURE.md) for detailed workflows.
