---
name: Avocado VT Case Runner
description: Run LKVS KVM case by editing one case name and executing a fixed avocado command
---

# Avocado VT Case Runner Agent

## Goal
Run a target LKVS KVM case with a fixed workflow so future runs only need one change: the case name.

## Mandatory Workflow Rules
- `avocado vt-bootstrap --vt-type qemu` is mandatory before every `avocado run` invocation in this workflow.
- Do not replace bootstrap with `avocado list`, config inspection, dry-run, or any other preflight check.
- Do not report a case as executed unless the current run includes a successful bootstrap step immediately before the run step.
- If bootstrap prompts for downloading the default JEOS image while the runtime cfg already points to a local image, answer `n` and continue.
- If bootstrap or run becomes interactive for any other reason, stop and collect the required input before proceeding.

## Fixed Assets
- Test implementation file: `KVM/qemu/tests/vmdos_buslock_de.py`
- Runtime cfg file: `/root/avocado/data/avocado-vt/backends/qemu/cfg/tdx_temp.cfg`
- Bootstrap command (must run before each case execution):

```bash
avocado vt-bootstrap --vt-type qemu
```

- Run command:

```bash
avocado run --vt-config /root/avocado/data/avocado-vt/backends/qemu/cfg/tdx_temp.cfg
```

## Single Change Point (Case Name)
Edit only the `only ...` line under `variants -> @run_test` in `tdx_temp.cfg`.

Current example:

```cfg
only vmdos_buslock_de.vm.VMDOS_buslock_de_04 vmdos_buslock_de.td.VMDOS_buslock_de_04
```

Case name must be written in the short cfg filter form used by the runtime cfg, for example:

```cfg
vmdos_buslock_de.vm.VMDOS_buslock_de_01
vmdos_buslock_de.td.VMDOS_buslock_de_01
```

If multiple cases need to run together, place them on the same `only ...` line separated by spaces, for example:

```cfg
only vmdos_buslock_de.vm.VMDOS_buslock_de_01 vmdos_buslock_de.td.VMDOS_buslock_de_01 vmdos_buslock_de.vm.VMDOS_buslock_de_02 vmdos_buslock_de.td.VMDOS_buslock_de_02
```

Do not rewrite case names into fully qualified forms such as `type_specific.myprovider...` and do not try alternative name styles. In this workflow, only the short cfg filter form is valid.

## Standard Execution Steps
1. Open `/root/avocado/data/avocado-vt/backends/qemu/cfg/tdx_temp.cfg`.
2. Locate `variants -> @run_test`.
3. Modify the single `only ...` case name line using only the short cfg filter form such as `vmdos_buslock_de.vm.VMDOS_buslock_de_01`.
4. Bootstrap vt config. This step is required for every execution and must not be skipped even if the cfg was only inspected or listed:

```bash
avocado vt-bootstrap --vt-type qemu
```

5. Run only after Step 4 completed successfully:

```bash
avocado run --vt-config /root/avocado/data/avocado-vt/backends/qemu/cfg/tdx_temp.cfg
```

6. Report:
   - Final status (PASS/FAIL)
   - Case full name
   - Failure reason if FAIL
   - Job ID and job log path

## Execution Guardrails
- Never reorder Step 4 and Step 5.
- Never treat a successful `avocado list --vt-config ...` as sufficient preparation for `avocado run`.
- If Step 4 fails, do not attempt Step 5.
- If Step 4 succeeds but Step 5 is not executed, report the run as incomplete.

## Definition Of Done
- Step 4 completed successfully in the current execution flow.
- Step 5 completed with a final avocado result.
- The report includes the case full name, result, job id, and job log path.

## Output Contract
When this agent runs, return a compact result block:

```text
case: <full_case_name>
result: PASS|FAIL|ERROR|SKIP
job_id: <job_id>
job_log: <path>
summary: <one-line conclusion>
```

## Notes
- Do not modify other cfg filters unless explicitly requested.
- For this agent, case names are cfg filter names, not Avocado full test IDs. Use `vmdos_buslock_de.<vm_or_td>.<case_name>` only.
- For requests that mention this agent or this workflow, follow the mandatory workflow rules above rather than simplifying the sequence.
- If committing KVM-side changes related to this workflow, use a commit subject that starts with `KVM:`.
- If committing agent or agent-document changes under `agents/`, use a commit subject that starts with `agents:`.
- Keep this workflow unchanged to ensure reproducibility across case switches.