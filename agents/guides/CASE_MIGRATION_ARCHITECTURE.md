---
name: Case Migration Framework Architecture
description: Overall architecture and structure of the case migration framework
related_agents:
  - ../agents/case_migration_framework.agent.md
related_guides:
  - CASE_MIGRATION_QUICK_START.md
---

# Case Migration Framework - Architecture & Structure

## Framework Philosophy

**Goal**: Systematize the process of migrating test cases from vmm_tree (legacy, infrastructure-heavy) to LKVS (modern, clean architecture) while preserving test intent and behavioral fidelity.

**Approach**: 
1. Decompose migration into 5 distinct phases
2. Create reusable "skills" for specialized tasks
3. Provide orchestration via main agent
4. Emphasize traceability and documentation

**Outcome**: 
- Reviewable, traceable migrations
- Reduced risk of behavioral drift
- Reusable playbooks for future migrations
- Knowledge capture for team learning

---

## Framework Structure

```
┌─────────────────────────────────────────────────────────────┐
│         CASE MIGRATION FRAMEWORK AGENT (Main Orchestrator)  │
│                                                              │
│  • Defines 4-phase workflow (Analysis → Translation →     │
│    Implementation → Validation)                             │
│  • Provides risk mitigation guidance                        │
│  • Coordinates task execution across skills                │
│  • Produces final migration checklist                       │
│                                                              │
│  Entry point: Use when starting migration project          │
└──────────────────────────────────┬──────────────────────────┘
                                   │
          ┌────────────────────────┼────────────────────────┐
          │                        │                        │
          ▼                        ▼                        ▼
    ┌──────────────────┐   ┌──────────────────┐   ┌──────────────────┐
    │   PHASE 1:       │   │   REQUIRED SKILLS │   │   PHASE 2-4:     │
    │   Case Analysis  │   │   (Specializations)  │   │   Translation &  │
    │                  │   │                  │   │   Implementation │
    │ ↓ Uses:          │   │ 1. Legacy Case   │   │                  │
    │ Skill-1          │   │    Analysis      │   │ ↓ Uses:          │
    │                  │   │                  │   │ Skills 2-5       │
    └──────────────────┘   │ 2. Parameter     │   └──────────────────┘
                           │    Mapping       │
                           │                  │
                           │ 3. Naming        │
                           │    Normalization │
                           │                  │
                           │ 4. Cfg           │
                           │    Translation   │
                           │                  │
                           │ 5. Test          │
                           │    Implementation│
                           │                  │
                           └──────────────────┘
```

---

## Component Details

### 1. Main Agent: `case_migration_framework.agent.md`

**Purpose**: Orchestrate full migration workflow from start to finish

**Responsibilities**:
- Define 4-phase workflow (Analysis, Translation, Implementation, Validation)
- Provide risk mitigation strategies
- Create validation checklist
- Document commit message template
- Coordinate skill invocation

**When to use**:
- Starting a migration project
- Need clear workflow guidance
- Want end-to-end picture

**Output type**:
- Migration plan (what to do)
- Risk assessment
- Validation checklist

---

### 2-6. Reusable Skills

Each skill is a **specialized workflow** focused on one migration aspect. They are:
- **Self-contained** (can be used independently)
- **Composable** (combined by main agent)
- **Referenced in documentation** (agent points to relevant skill)

#### Skill 1: `VMM_TREE_LEGACY_CASE_ANALYSIS_SKILL.md`

**Purpose**: Reverse-engineer legacy case semantics

**Input**: Legacy case name + location

**Process**:
1. Extract raw parameters from vmm_tree cfg
2. Decode naming convention patterns
3. Infer execution behavior and flow
4. Document parameter meanings
5. Identify host assumptions and constraints

**Output**: Legacy case analysis report (YAML-structured)

**Example**:
```yaml
legacy_case: tdx_vmx2_from1024m_toall
parameters: [iterations: 20, mem_generator: random_32g_window, ...]
behavior: "Boot 1 TDX VM across memory sweep from 1GB to host max"
intent: "Validate TDX VM stability under varying memory allocation"
```

---

#### Skill 2: `LKVS_PARAMETER_MAPPING_SKILL.md`

**Purpose**: Map legacy parameters to LKVS equivalents

**Input**: Legacy parameters + target LKVS test handler

**Process**:
1. Extract legacy parameter list
2. Identify LKVS handler expectations
3. Build mapping table (unit conversions, type changes)
4. Identify gaps (missing/new/dropped parameters)
5. Create semantic validation checklist

**Output**: Parameter mapping document (table + validation rules)

**Example**:
```
Legacy param    │ Value    │ Unit  │ LKVS Param    │ Value   │ Conversion
─────────────────┼──────────┼───────┼───────────────┼─────────┼────────────
start_mem       │ 1024     │ MiB   │ start_mem     │ 1024    │ 1:1
mem_generator   │ random_… │ name  │ mem_generator │ random_…│ 1:1
```

---

#### Skill 3: `NAMING_NORMALIZATION_SKILL.md`

**Purpose**: Standardize case names from legacy to LKVS style

**Input**: Legacy case names + LKVS naming patterns (existing)

**Process**:
1. Analyze existing LKVS naming conventions
2. Create legacy→LKVS mapping table
3. Define naming schema rules
4. Handle edge cases
5. Batch validation (no collisions, filesystem safety)

**Output**: Naming mapping document + validation results

**Example**:
```
Legacy: tdx_vmx2_from1024m_toall  →  LKVS: 1td.from1G_toall
Legacy: 4td_from1024m_toall       →  LKVS: 4td.from1G_toall
Naming rules: <vm_config>.<memory_range>
```

---

#### Skill 4: `LKVS_CFG_TRANSLATION_SKILL.md`

**Purpose**: Translate case cfg from vmm_tree to LKVS cartograph

**Input**: Legacy cfg block + parameter mapping table + LKVS target file

**Process**:
1. Identify cartograph target location
2. Translate variant hierarchy to nested structure
3. Inline parameters from mapping table
4. Validate cfg syntax (indentation, nesting)
5. Cross-reference parameter usage in Python handler

**Output**: Translated cfg block ready to insert

**Example**:
```
- 1td:
    - from1G_toall:
        type = multi_vms_multi_boot
        iterations = 20
        mem_generator = random_32g_window
        # ... parameters ...
```

---

#### Skill 5: `LKVS_TEST_IMPLEMENTATION_SKILL.md`

**Purpose**: Implement or verify Python test handlers

**Input**: Case migration context + target test file + parameter mapping

**Process**:
1. Decide if cfg-only or Python changes needed
2. If Python needed:
   - Implement test with proper entry point
   - Extract cfg parameters with type safety
   - Implement core logic (boot/destroy, iteration, etc.)
   - Ensure cleanup in finally block
3. Verify handler reads all migrated parameters
4. Add licensing headers and documentation

**Output**: Modified or verified Python test + verification table

**Example**:
```python
@error_context.context_aware
def run(test, params, env):
    """Test entry point."""
    iterations = params.get_numeric("iterations", 1)
    # ... implementation ...
    finally:
        # Cleanup VMs
```

---

## Execution Workflow

### Typical Single-Case Migration

```
Start (have legacy case name)
    │
    ├─→ Run SKILL 1: Legacy Case Analysis
    │   Output: Legacy semantics
    │
    ├─→ Run SKILL 2: Parameter Mapping
    │   Output: Mapping table + validation
    │
    ├─→ Run SKILL 3: Naming Normalization
    │   Output: LKVS target name + schema
    │
    ├─→ Decision: Python changes needed?
    │   ├─ NO → Cfg-only path
    │   │       └─→ Run SKILL 4: Cfg Translation
    │   │           Output: Ready to commit
    │   │
    │   └─ YES → Implementation path
    │           └─→ Run SKILL 5: Test Implementation
    │               Output: Python + cfg ready to commit
    │
    └─→ Run SKILL 4: Cfg Translation (if not done)
        Output: Final cfg block
        
    └─→ Validate & Commit
```

### Batch Migration (Multiple Cases)

```
Start (have case family to migrate)
    │
    ├─→ Run SKILL 1: Analyze ALL cases together
    │   Output: Common patterns → LKVS schema
    │
    ├─→ Run SKILL 3: Define naming schema (once)
    │   Output: Unified naming for entire family
    │
    ├─→ Run SKILL 2: Build mapping table for all
    │   Output: Single reference table
    │
    ├─→ Batch SKILL 4: Translate all cfg at once
    │   Output: Single cfg update with all cases
    │
    ├─→ Verify via SKILL 5 (single pass)
    │   Output: Verification table
    │
    └─→ Single validation & commit (all cases)
        Output: Traceable batch migration
```

---

## Skill Interdependencies

```
SKILL 1 (Legacy Analysis)
    │
    ├─→ Feeds into: SKILL 2 (Parameter Mapping)
    │               SKILL 3 (Naming Normalization)
    │
SKILL 2 (Parameter Mapping)
    │
    ├─→ Feeds into: SKILL 4 (Cfg Translation)
    │               SKILL 5 (Test Implementation)
    │
SKILL 3 (Naming Normalization)
    │
    ├─→ Feeds into: SKILL 4 (Cfg Translation)
    │
SKILL 4 (Cfg Translation)
    │
    ├─→ Input to: Commit message, validation checklist
    │
SKILL 5 (Test Implementation)
    │
    ├─→ Input to: Validation checklist, commit message
    │
    └─→ Verifies: Parameter usage from SKILL 2
```

---

## Reusability Patterns

### Pattern 1: Repeatable Case Family (Most Common)
When migrating cases with same structure (e.g., all memory sweeps):
1. **Run SKILL 1 once** for family: Extract common patterns
2. **Run SKILL 3 once**: Define naming schema
3. **Run SKILL 2 once for all**: Build unified mapping
4. **Run SKILL 4 in batch**: Apply to all at once
5. **Run SKILL 5 once**: Verify handler

**Benefit**: 5+ cases migrated with high efficiency; consistent naming; single validation pass

### Pattern 2: Single Outlier Case
When migrating a case that doesn't fit family pattern:
1. **Run SKILL 1**: Understand this specific case
2. **Run SKILL 2**: Single-case mapping
3. **Run SKILL 3**: Document exception/divergence
4. **Run SKILL 4**: Translate with notes
5. **Run SKILL 5**: Verify

**Benefit**: Deliberate exception; documented divergence; reviewable outlier

### Pattern 3: Incremental Long-Term Migration
When migrating entire vmm_tree test suite over months:
1. **Establish LKVS naming schema** (SKILL 3, once at start)
2. **For each case group**: Repeat SKILLS 1-2-4 (SKILL 3 already done)
3. **Batch SKILL 5** (verify handler, running monthly)
4. **Commit by case family**: Linked to master schema

**Benefit**: Consistent naming across entire suite; schema reuse; incremental progress

---

## Risk Mitigation Strategy

### By Skill:
- **SKILL 1**: Risk of misunderstanding legacy intent → Document thoroughly, add comments
- **SKILL 2**: Risk of unit/type mismatch → Create validation table, check both codebases
- **SKILL 3**: Risk of name conflicts/confusion → Batch validation before applying
- **SKILL 4**: Risk of cfg syntax errors → Validate indentation, test against existing files
- **SKILL 5**: Risk of parameter drift → Grep for all params in handler, verify read

### By Phase:
- **Phase 1 (Analysis)**: Reduce by documenting assumptions and edge cases
- **Phase 2 (Translation)**: Reduce by cfg-first approach (Python only if necessary)
- **Phase 3 (Implementation)**: Reduce by reusing existing handlers when possible
- **Phase 4 (Validation)**: Reduce by comprehensive checklist + pre-commit validation

---

## Documentation & Knowledge Base

### What Gets Documented
1. **Mapping tables** → In commit messages and cfg comments
2. **Naming schemas** → In SKILL 3 output, reused for follow-up migrations
3. **Parameter conversions** → In SKILL 2, part of permanent record
4. **Common pitfalls** → Updated in skills as new issues found
5. **Handler requirements** → Captured in SKILL 5 verification table

### Where It's Stored
- **Instant reference**: Cfg comments, commit messages
- **Future reference**: Framework skills, agent guides
- **Indexed**: Search by case name, parameter, or family group

---

## Team Onboarding

**For new team members migrating cases**:
1. Read this architecture document (overview)
2. Read `CASE_MIGRATION_QUICK_START.md` (hands-on example)
3. Refer to specific skills as needed during work
4. Ask questions; update framework based on learnings

**For reviewers** reviewing a migration PR:
1. Check reference in commit message to vmm_tree source
2. Review mapping table (SKILL 2 output) for correctness
3. Verify cfg syntax (SKILL 4)
4. Check Python handler for parameter usage (SKILL 5)
5. Confirm validation checklist is complete

---

## Version & Maintenance

### Framework Version History
- **v1.0** (2024-03-25): Initial framework release
  - 5 core skills
  - Single-case and batch migration workflows
  - Risk mitigation and validation guidance

### Future Enhancements
- Guidance for VM type-specific migrations (vm→td, etc.)
- Automation scripts for parameter mapping
- Pre-migration compatibility checker
- Legacy→LKVS cfg diff visualization

---

## Quick Reference: When to Use Each Skill

| Situation | Skill | Why |
|-----------|-------|-----|
| "What does this legacy case do?" | [VMM_TREE_LEGACY_CASE_ANALYSIS](../skills/VMM_TREE_LEGACY_CASE_ANALYSIS_SKILL.md) | Decode intent + semantics |
| "How do I map legacy params to LKVS?" | [LKVS_PARAMETER_MAPPING](../skills/LKVS_PARAMETER_MAPPING_SKILL.md) | Build equivalence table |
| "What should I name this in LKVS?" | [NAMING_NORMALIZATION](../skills/NAMING_NORMALIZATION_SKILL.md) | Standardize; avoid conflicts |
| "How do I write LKVS cfg for this?" | [LKVS_CFG_TRANSLATION](../skills/LKVS_CFG_TRANSLATION_SKILL.md) | Translate structure + params |
| "Does the Python handler read all params?" | [LKVS_TEST_IMPLEMENTATION](../skills/LKVS_TEST_IMPLEMENTATION_SKILL.md) | Verify + implement if needed |

---

## Summary

This framework turns ad-hoc case migration into a **systematic, repeatable process**:
- ✓ Clear phases with defined inputs/outputs
- ✓ Reusable skills for specialized tasks
- ✓ Risk guidance at each step
- ✓ Traceability by commit message + documentation
- ✓ Efficient batch processing for case families
- ✓ Knowledge capture for team learning

**Expected outcome**: 
- Predictable migration timeline (1-2 hours per case, 30 min overhead)
- High confidence in behavior fidelity
- Reviewable, traceable changes
- Reusable playbooks for future work
