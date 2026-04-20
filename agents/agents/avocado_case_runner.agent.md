---
name: Avocado VT Case Runner
description: Run LKVS KVM case by editing one case name and executing a fixed avocado command
---

# Avocado VT Case Runner Agent

## Goal
Run a target LKVS KVM case with a fixed workflow so future runs only need one change: the case name.

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
only vmdos_buslock_de.td.VMDOS_buslock_de_04
```

Replace only the case name string when switching to a new case.

## Standard Execution Steps
1. Open `/root/avocado/data/avocado-vt/backends/qemu/cfg/tdx_temp.cfg`.
2. Locate `variants -> @run_test`.
3. Modify the single `only ...` case name line.
4. Bootstrap vt config:

```bash
avocado vt-bootstrap --vt-type qemu
```

5. Run:

```bash
avocado run --vt-config /root/avocado/data/avocado-vt/backends/qemu/cfg/tdx_temp.cfg
```

6. Report:
   - Final status (PASS/FAIL)
   - Case full name
   - Failure reason if FAIL
   - Job ID and job log path

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
- Keep this workflow unchanged to ensure reproducibility across case switches.