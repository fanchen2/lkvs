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

### 1.2 Code-Only Comment Semantics
- In executable files (`.py`, `.cfg`, shell, etc.), comments and docstrings must describe only what the code does and why it is needed at runtime.
- Do not mention migration lineage or source project names in code comments (for example: `vmm_tree`, `XVS`, `legacy` history labels).
- Keep historical provenance in commit messages or agent/migration documents, not in runtime code.

### 2. Naming Normalization
- Convert legacy snake_case names to LKVS conventions
- Document mapping in commits and agent checklists
- Normalize names around stable semantic segments such as VM topology, execution mode, and parameter intent.
- When units are normalized during renaming, record the old and new units in the mapping table so the name change remains traceable.

### 3. Project-Aware Configuration
- **vmm_tree perspective**: legacy, infrastructure-coupled, minimal disruption
- **LKVS perspective**: modern, clean separation, strict structure
- Use **bridge layer** (mapping document) to clarify equivalence
- Keep case runtime variables in cfg: file names, source/exec binaries, paths, command arguments, and mode switches should be configurable parameters instead of Python literals.
- Python should read these values from `params`; avoid hardcoding case-specific values in test scripts unless they are true constants shared by all variants.

### 4. Multi-Layer Verification
- **Cases are not guest-only tests** — they validate the entire stack: guest behavior, host/hypervisor behavior, and cross-layer interactions.
- **Always inspect legacy case for all verification points** — not just "main" behavior:
  - Guest side: dmesg logs, MSR values, command outputs, process exit codes
  - Host side: kernel trace events, debugfs logs, hypervisor exit counts, syscall patterns
  - Cross-layer: boot timing, device initialization order, interrupt delivery, exception handling
- **Do not omit any legacy verification step during migration** unless explicitly confirmed by the user.
  - If a legacy step appears non-obvious or optional, **ask the user for clarification** before skipping it.
  - Default assumption: all legacy checks are intentional and required for fidelity.
- **Implement host-side verification** in Python using `process.run()` (host commands) alongside guest session commands.
- **Parameterize verification switches** in cfg so non-essential checks can be conditionally enabled/disabled per variant without losing the capability.

### 5. Traceability
- Every case migration must have:
  - Source reference (vmm_tree path/name)
  - Target reference (LKVS cfg path + Python test)
  - Parameter mapping table
  - Semantic equivalence statement
  - Verification scope statement (what gets verified: guest, host, cross-layer)
  - Commit message with explicit migration note

## Migration Workflow (Per Case Group)

### Phase 1: Case Analysis & Mapping
1. **Identify legacy case group**
   - Find all related vmm_tree cfg lines
   - Extract parameters (iterations, memory ranges, VM types)
   - Document legacy naming convention
   - Identify execution model (cfg type, test handler)
   - **Identify all verification points** (both guest-side and host-side checks)

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
   - **Add verification mapping row**: List all legacy checks (guest dmesg, MSR, host trace, etc.) with target implementation status

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
   - Define case file/path variables in cfg (for example source files, executable names, and helper tool names) so each variant can override without Python edits

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
   - Read case-specific file/path/command variables from `params`; do not inline per-case filenames or paths in Python
   - For guest/host package installation in tests, use common helpers (for example `utils_package.package_install`) instead of hand-written distro command chains (`dnf/yum/apt-get` branches)
   - **Implement all verification points from legacy case**:
     - Guest-side: dmesg checks, session commands, file reads
     - Host-side: use `process.run()` for `/sys/kernel/debug/tracing`, debugfs, or kernel commands
     - Parameterize verification flags in cfg (e.g., `check_exception_nmi`, `verify_trace`) to allow customization without removing capability

3. **Preserve legacy logic**
   - If legacy test does something non-obvious (workaround, hardware quirk), document in comment
   - Keep function names consistent with migration story

### Phase 4: Validation & Commit
1. **Pre-commit local validation** (before push to avoid GitHub CI failures):
   - **Python code lint**: Run inspekt on modified test files
     ```bash
     inspekt checkall ./KVM/qemu/tests/<test_file>.py --disable-style E501,E265,W601,E402,E722,E741 --no-license-check
     ```
   - **CFG file validation**: Ensure proper syntax and indentation
     ```bash
     # Manual check: 4-space indent, no trailing whitespace, proper nesting
     # Or use: python3 -c "import yaml; yaml.safe_load(open('./KVM/qemu/<test_name>.cfg'))"
     ```
   - **Shell scripts**: Run shellcheck if modifying `.sh` files
     ```bash
     shellcheck ./.github/scripts/*.sh
     ```
   - **Code style**: Check for unnecessary duplication, dead code, redundant logic

2. **GitHub CI checks that will run on PR**:
   - **CodeCheck**: Running `./.github/scripts/pr_check` on modified code (shellcheck, checkpatch, codespell)
   - **BuildCheck**: Docker build validation for BM artifacts
   - **python-style-check**: inspekt lint on KVM test files
   - **cfg-lint-check**: CFG file linting for `/KVM/qemu/*.cfg`
   - **commitlint** (if using avocado-vt): Validates commit message format

3. **Pre-commit validation checklist**:
   - No lint errors from inspekt
   - CFG structure integrity: no corruption, correct indentation (4 spaces)
   - Parameter alignment: all references match cfg
   - Code review pass: no unnecessary duplication, no dead/redundant logic
   - Commit message format valid (Scope: message, body with references)
   - All modified files are in scope (test, cfg, agent, skill)

4. **Behavioral validation**:
   - Document expected test behavior in observable terms such as iteration count, VM type, variable under test, and cleanup expectations.
   - Note any differences from legacy (if unavoidable)
   - Add links to legacy case for future reference

5. **Commit with full context**:
   - Use migration-focused scope: e.g., `KVM/qemu: migrate <case_family> cases from vmm_tree`
   - Include mapping table or summary in body
   - Reference vmm_tree location (e.g., `Migrated from vmm_tree/validation/...`)
   - Document any behavior changes or approximations
   - Verify commit message will pass GitHub CI commitlint checks

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

### Common Pitfall 6: Package Installation Logic Drift
- **Risk**: Copying legacy shell install snippets (`dnf/yum/apt-get`) into Python tests causes inconsistent behavior and maintenance overhead
- **Mitigation**: Use Avocado/virttest common install helpers (for example `utils_package.package_install`) for package installation paths
- **Validation**: Check touched test files for hardcoded package-manager command chains; keep install handling through shared helper APIs

### Common Pitfall 7: Incomplete Verification Layer Migration
- **Risk**: Migrating guest-side checks but omitting host-side trace/debugfs verification, resulting in incomplete test coverage and failure to catch hypervisor bugs
- **Real Example**: vmdos_buslock_de originally checked **both** guest dmesg (`split lock detection: #DB` traps) **and** host KVM trace (`EXCEPTION_NMI` with exit reason `0x80000301`). First migration omitted the host trace check.
- **Why It Happens**: 
  - Mental model narrowing: "this is a guest test" → only verify guest state
  - Unfamiliarity with host command execution from Python (`process.run()`)
  - Legacy shell script mixes host and guest commands seamlessly, but Python test structure feels more guest-focused
  - Incomplete source inspection (did not cross-reference original case logic)
- **Mitigation**: 
  - **Line-by-line case comparison**: For each operation in legacy test, classify as host or guest, then ensure Python implements both
  - **Ask before omitting**: If a verification step seems non-essential, explicitly list it and ask user for confirmation before removing
  - **Use process.run()** for host-side checks (debugfs, trace, sysfs, kernel commands)
  - **Parameterize with defaults**: Add cfg flags (e.g., `expect_exception_nmi`, `ratelimit_retry`) so checks are enabled by default and can be customized per variant
- **Validation**: 
  - Document all verification points from legacy case in mapping table
  - For each point, confirm it is either:
    1. Implemented in Python (guest session or host process.run())
    2. Explicitly user-approved as not applicable (with reason)
  - Trace event checks, debugfs reads, and cross-layer timing must be present unless justification is documented

### Common Pitfall 5: Parameter Inheritance in cfg
- **Risk**: Custom parameter names (e.g., `guest_mem`) don't apply to all auto-generated VMs
- **Mitigation**: Use standard LKVS parameters (e.g., `mem`) and ensure runtime per-VM overrides are built from resolved VM list
- **Example**: In dynamic multi-VM flows, prefer inherited parameters such as `mem` for shared boot settings and generate per-VM overrides only when the case truly needs them
- **Pattern**: VM-specific params like `mem_vm1`, `mem_vm2` require explicit cfg entries; generic params like `mem` auto-inherit
- **Validation**: For dynamic VM list, test that all VMs (vm1, vm2, vm3+) receive intended memory value

### Common Pitfall 8: Incomplete Conditional Branch Porting
- **Risk**: Logic in `if...then...fi` branches (especially VM-type or environment-specific) gets omitted during migration, causing divergent behavior between VM/TD variants or conditional paths
- **Real Example**: vmdos_buslock_de TDX logic omitted:
  - Original vmm_tree had: `if [ $? -eq 0 ]; then tool=bus_lock_64.out` (select 64-bit tool for TDX)
  - Also had: `if [ "$ISTDX" = "0" ]; then update_guest_kernel_cmdline ...` (skip cmdline update for TDX)
  - Migration created cfg-only parameters without conditional selection, and called `_update_split_lock_mode()` unconditionally
- **Why It Happens**:
  - Focus on "main execution path" (non-branching code) over branching logic
  - Shell script structure `if...then...fi` is visually clear, but mental translation to Python/cfg parameterization is non-obvious
  - Branching logic often embedded at function start or scattered throughout, not centralized
  - Assumption that "all variants execute the same code path" without verifying all conditional branches
- **Mitigation**:
  - **Build a conditional branch table**: For each legacy case, list all `if...then` and `case...esac` statements with their conditions, branches, and affected parameters
  - **Parametrize branch conditions**: Map legacy `if [ "$ISTDX" = "0" ]` to cfg-driven detection (e.g., `is_tdx = params.get("vm_secure_guest_type") == "tdx"`)
  - **Use cfg-driven tool selection**: For parameters that differ by VM type/environment (tool versions, timeouts, etc.), add suffixed parameters (e.g., `bus_lock_source_file_tdx`)
  - **Guard operations with types**: Wrap conditional operations with explicit checks in Python (e.g., `if not is_tdx: _update_split_lock_mode(...)`)
  - **Document all branches**: Add comments explaining which branches apply to which variants
- **Validation**:
  - Create a table: Legacy Condition → LKVS Implementation → Affected VM Types
  - For each condition:
    1. Confirm parameter/flag is exposed in cfg (or computed from cfg)
    2. Confirm Python reads and uses this parameter/flag
    3. Confirm skip/conditional logic is applied correctly for each variant
  - Example validation row:
    ```
    Condition: VM is TDX (ISTDX == 1)
    LKVS: vm_secure_guest_type == "tdx" → is_tdx parameter
    Implementation: 
      - cfg: machine_type_extra_params, vm_secure_guest_type
      - Python: is_tdx = params.get("vm_secure_guest_type") == "tdx"
      - Uses: tool selection, kernel cmdline skip, EXCEPTION_NMI check skip
    Affected VM Types: td variant only
    ```

### Common Pitfall 9: CFG Parameter Redundancy From Copy-Paste
- **Risk**: Case cfg grows with redundant parameter definitions, increasing maintenance burden and mutation risk (changing one parameter requires multiple edits)
- **Real Example**: vmdos_buslock_de originally had 12 lines of duplicate tool file parameters:
  - Cases 01, 02, 03 each defined: `bus_lock_source_file = bus_lock.c`, `bus_lock_exec_file = bus_lock.out`, `bus_lock_source_file_tdx = bus_lock_64.c`, `bus_lock_exec_file_tdx = bus_lock_64.out`
  - Case 04 overrode tool versions: `bus_lock_source_file = bus_lock_10.c`, `bus_lock_exec_file = bus_lock_10.out`
  - Discovery: After adding TDX variant layer, realized 4 out of 8 parameter definitions were identical
- **Why It Happens**:
  - Case-by-case copy-paste: create case 01, duplicate to case 02, modify only intent params (split_lock_mode, expected_*), leave tool params as-is
  - Feature additions (e.g., TDX support) layer on top: add TDX tool params to all cases individually instead of auditing global scope first
  - Lack of post-completion review: complete cfg, submit, realize redundancy only later during maintenance
  - Mental model: cases are independent units, not members of a shared parameter hierarchy
- **Mitigation**:
  - **After defining all case variants**: audit cfg for "same value across N variants"
  - **Parameter promotion checklist**:
    - If parameter X appears in 80%+ of variants with identical value → move to case-family level (above `variants:`)
    - If exception (variant Y differs), use case-level override instead
    - Document why exception exists (e.g., "case 04 uses different tool version")
  - **CFG hierarchy discipline**: Establish clear scoping:
    ```
    <case_family>:
      <global params>        # Shared by all variants (tool names, test_dir, deps_subdir, etc.)
      <variant_base_params>  # Common to all variants in this family
      variants:
        - variant_a:
            <param_unique_to_a>
        - variant_b:
            <param_unique_to_b>
            <param_override_from_global>  # Only when exception needed
    ```
  - **Use cfg inheritance patterns**: kartograph supports parameter inheritance; layer shared params at family level, case level contains only deltas
  - **Post-migration cfg audit**: Before finalizing cfg, grep for duplicate strings and consolidate redundancy
- **Validation**:
  - Scan all variants in case cfg: `grep -E "^\s+(param_name|foo|bar) =" <case>.cfg | sort | uniq -c`
  - If count matches variant count with identical value → promotion candidate
  - Document rationale for any non-promoted duplicates (e.g., "intentional case 04 override")

  ### Common Pitfall 10: VM Startup Ownership Conflict (`start_vm` vs `preprocess_vm`)
  - **Risk**: Test framework and test script both try to manage VM lifecycle, causing either:
    - VM not started when expected (`start_vm` is not `yes`, but script assumes `preprocess_vm()` will always boot)
    - VM destroyed and recreated unexpectedly (framework already started VM, then script calls `preprocess_vm()` again and triggers restart on parameter diff such as network model)
  - **Real Example**: In `vmdos_buslock_de`, adding `start_vm = yes` while still calling `env_process.preprocess_vm()` in Python caused "network differs, restarting" and a second VM creation.
  - **Why It Happens**:
    - Misunderstanding `preprocess_vm()` as unconditional "create/start VM" API
    - Overlap between cfg lifecycle control (`start_vm`, `kill_vm`) and script lifecycle control
    - Runtime parameter drift (`needs_restart`) between current VM and expected params triggers implicit destroy/recreate
  - **Mitigation**:
    - **Choose one owner for initial boot** per case family:
      - If cfg uses `start_vm = yes`: Python should generally use `env.get_vm()` and avoid an extra `preprocess_vm()` call.
      - If cfg does not auto-start: Python may call `preprocess_vm()` or explicit `vm.create()` based on the case design.
    - **Use `preprocess_vm()` only when you intentionally need parameter reconciliation/restart behavior**.
    - Keep VM lifecycle intent explicit in cfg comments and test `run()` flow.
  - **Validation**:
    - During first boot logs, confirm only one expected create path is used.
    - Check for restart signatures such as `VM params in env match, but network differs, restarting`.
    - Verify `start_vm` policy and Python lifecycle calls are consistent.

### Common Pitfall 11: Fragile Host Cmdline Matching (`bytes/str` and token checks)
- **Risk**: Host precheck fails even when kernel cmdline is correct, caused by incorrect output type handling or brittle string matching logic.
- **Real Example**: In `vmdos_buslock_de`, `/proc/cmdline` contained `split_lock_detect=off` but check still failed because of a buggy expression (`host_cmdline()` call) and inconsistent handling of command output type.
- **Why It Happens**:
  - `process.run(...).stdout` may be `bytes` in some runtime paths.
  - Mixing callable/variable forms by accident during quick edits.
  - Overly strict tokenization logic can be fragile when cmdline formatting changes.
- **Mitigation**:
  - Normalize command output before matching:
    - Decode when value is `bytes`, then strip whitespace.
    - Keep matching logic simple and explicit (`"split_lock_detect=off" in host_cmdline`).
  - Add a debug log with the normalized host cmdline when diagnosing precheck mismatches.
  - Prefer helper-style encapsulation for host prechecks so parsing logic is centralized and reused.
- **Validation**:
  - Verify precheck passes on a host known to contain `split_lock_detect=off`.
  - Add a negative test path (or dry-run reasoning) for a host cmdline without that key.
  - Confirm no callable misuse pattern such as `host_cmdline()` exists in check code.

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

6. **Use splitlines() for Text Output Parsing**
   - When processing multi-line command output, use `output.splitlines()` instead of `split('\n')` to reliably handle different line endings (LF, CRLF)
   - Combine with line filtering to remove empty lines: `[line for line in output.splitlines() if line.strip()]`
   - This pattern is more robust than manual split operations and handles edge cases like trailing newlines

## Validation Checklist

- [ ] **Source case identified** in vmm_tree with full path
- [ ] **Parameter mapping table** created (legacy → LKVS)
- [ ] **Naming convention** documented and applied consistently
- [ ] **CFG translated** and structure validated (indentation, nesting)
- [ ] **Python handler** verified (if new) or existing one checked for completeness
- [ ] **Cfg-driven variables**: case file names, paths, command args, and tool names are defined in cfg and consumed via `params` (no per-case hardcoded literals in Python)
- [ ] **Common install helper used**: package installation uses shared APIs (for example `utils_package.package_install`), not ad-hoc distro-specific shell chains
- [ ] **Post-edit code review done** (correctness + side effects + readability)
- [ ] **Duplication minimized** (no repeated logic blocks when one shared path is enough)
- [ ] **Code kept concise** (remove redundant branches/variables/comments)
- [ ] **Comment semantics clean**: In non-markdown code/config files, comments/docstrings describe runtime behavior only; no migration/source-history labels (e.g., `vmm_tree`, `XVS`)
- [ ] **License header** added (GPL-2.0-only + Intel copyright) if new file
- [ ] **Pre-commit passes locally** on modified files
  - [ ] Python lint: `inspekt checkall ./KVM/qemu/tests/<test_file>.py --disable-style E501,E265,W601,E402,E722,E741 --no-license-check`
  - [ ] CFG lint: Proper 4-space indentation, no trailing whitespace, valid nesting
  - [ ] **CFG redundancy audit**: Check for duplicate parameter definitions across variants and promote common parameters to case-family level (reference Common Pitfall 9)
  - [ ] Shell scripts: `shellcheck` passes if modifying scripts
- [ ] **GitHub CI checks will verify** (automated on PR):
  - [ ] CodeCheck: Code linting (shellcheck, checkpatch, codespell)
  - [ ] BuildCheck: Docker build validation
  - [ ] python-style-check: inspekt on KVM test files
  - [ ] cfg-lint-check: CFG file syntax validation
  - [ ] commitlint: Commit message format (if applicable)
- [ ] **Commit message** includes source reference and mapping summary
- [ ] **Grep check** confirms no broken references or orphaned params
- [ ] **Layer validation**: cfg type points to valid test; Python parses all required params
- [ ] **Multi-layer verification complete**: All guest-side checks ✓ + all host-side checks ✓ + cross-layer checks ✓ implemented or explicitly user-approved as excluded
- [ ] **Verification mapping**: Legacy case checks documented in mapping table with implementation status (implemented in Python / approved for omission / user pending)
- [ ] **Host-side trace/debugfs**: If legacy case uses `/sys/kernel/debug/` or `process.run()` for event inspection, Python implements equivalent `process.run()` calls
- [ ] **VM startup ownership aligned**: `start_vm` policy in cfg and Python lifecycle calls (`preprocess_vm`, `vm.create`, `env.get_vm`) are consistent; no unintended double-boot/restart path
- [ ] **Host cmdline precheck robustness**: command output type is normalized (`bytes` vs `str`) and key matching logic is stable for expected kernel args
- [ ] **No verification step silently dropped**: All legacy verification points ported unless user explicitly confirmed discontinuation
- [ ] **Conditional branches mapped**: All `if...then` / `case...esac` branches in legacy code are accounted for:
  - [ ] Build conditional branch table (condition → operation → affected variant)
  - [ ] For each condition: verify parametrization in cfg (or computed in Python)
  - [ ] For each operation: verify conditional execution in Python code
  - [ ] VM-type specific operations (vm vs td branches) are guarded explicitly

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

## Strict Migration Policy: No Silent Omissions

**Core Rule**: When migrating from vmm_tree, every operation, check, and verification point in the legacy case must be faithfully ported to LKVS.

**When Omission Seems OK**:
1. **Document why** — e.g., "infrastructure not available in LKVS, can skip CPU speed sweep"
2. **Propose explicitly** to user: "Found X in legacy case but it seems N/A in LKVS for reason Y. Should I skip it?"
3. **Wait for confirmation** — do not assume omission is acceptable
4. **Record decision** in cfg comments or commit message if approved

**Example**: If legacy test reads `/sys/kernel/debug/tracing/trace` for EXCEPTION_NMI validation:
- **Do not skip** with reasoning "trace is optional"
- **Instead ask**: "Legacy case checks host KVM trace for EXCEPTION_NMI events. Should this be ported? Can I add it?"
- **Port the check** unless user explicitly says "no, this detail is not needed in LKVS"

## Commit Message Template

```
KVM: add <case_name> case to <test_family>

Brief description of new case behavior and variants.

Implementation details (if applicable):
  - Extended handler: <handler_name> with <mode_flag>
  - Supports: vm (standard), td (TDX), etc.
  - Verification: guest dmesg + host trace validation, MSR checks, etc.

Migrated from: vmm_tree/validation/xvs/src/src/TestSuites/<suite>/ts1.sh

Signed-off-by: <name> <email>
```

**Template Notes**:
- Use "KVM:" scope for KVM/qemu test additions
- Focus on "add" (new capability) rather than "migrate" or "fix"
- Keep body brief; emphasize new feature, not legacy lineage
- Mention handler extension or implementation pattern used
- List supported variant types (vm, td, etc.)
- **Add verification scope** to commit body so reviewers know what validation was ported

## Commit Message Guardrails (Mandatory)

Before any commit, run this preflight in order:

1. Determine scope from staged file paths:
  - Any non-`.md` file under `KVM/qemu/` -> scope MUST be `KVM:`
  - Markdown-only changes follow default `<scope>: <summary>` rule (no forced `KVM:`)
  - Any file under `BM/` -> scope SHOULD be `BM:` or `BM/<feature>:`
2. Validate subject line format:
  - Required pattern: `<scope>: <lowercase imperative summary>`
  - Reject file-name scopes for KVM/qemu changes (for example `vmdos_buslock_de:`)
3. Ensure sign-off is present:
  - Use `git commit -s`
4. If any check fails:
  - Do not commit
  - Rewrite the message and re-validate

Recommended automation:
- Install the repository hook template at `.githooks/commit-msg`
- Configure once per clone:
  - `git config core.hooksPath .githooks`

Hook behavior expected by this agent:
- Block commit if staged files include non-`.md` files under `KVM/qemu/` but subject does not start with `KVM:`
- Block commit if subject does not match generic `<scope>: <summary>` format
- Block commit if `Signed-off-by:` is missing

## How to Use This Framework

### For Each Migrated Case:
1. Read **Phase 1** to understand scope and build verification mapping
2. Reference **Multi-Layer Verification** principle: if unsure whether a check matters, ask the user
3. Use **Strict Migration Policy** as a guardrail: never silently drop a legacy check without user approval
4. Check the **Validation Checklist** before committing — ensure multi-layer verification completeness is marked off
5. Reference **Common Pitfall 7** if you find yourself tempted to skip host-side checks

### For Future Reference:
- When discovering a pattern that affects multiple case migrations, promote it to this framework
- When adding new cases, reference past pitfalls in this document
- If multi-layer verification fails in a case, document the lesson in a new Pitfall entry

## Agent Coordination

This agent works in tandem with:
- **[LKVS_CFG_TRANSLATION_SKILL](../skills/LKVS_CFG_TRANSLATION_SKILL.md)**: Handles cartograph translation rules
- **[LKVS_PARAMETER_MAPPING_SKILL](../skills/LKVS_PARAMETER_MAPPING_SKILL.md)**: Documents parameter equivalence
- **[LKVS_TEST_IMPLEMENTATION_SKILL](../skills/LKVS_TEST_IMPLEMENTATION_SKILL.md)**: Guides Python test creation/modification
- **[VMM_TREE_LEGACY_CASE_ANALYSIS_SKILL](../skills/VMM_TREE_LEGACY_CASE_ANALYSIS_SKILL.md)**: Extracts legacy case semantics
- **[NAMING_NORMALIZATION_SKILL](../skills/NAMING_NORMALIZATION_SKILL.md)**: Standardizes case names across projects

See [Quick Start Guide](../guides/CASE_MIGRATION_QUICK_START.md) and [Architecture Doc](../guides/CASE_MIGRATION_ARCHITECTURE.md) for detailed workflows.
