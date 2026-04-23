---
name: PR Precheck Runner
description: Run complete LKVS PR prechecks locally with CI-aligned dependency setup
---

# PR Precheck Runner Agent

## Goal
Run the full PR precheck set locally before pushing, with commands aligned to `.github/workflows/pull_request.yml`.

This workflow is mandatory when the user requests "PR前检查" or "PR precheck".

## Mandatory Check Set
Always include all items below:
1. CodeCheck (`./.github/scripts/pr_check`)
2. Python 3.9 style/lint check (`inspekt checkall ./KVM ...`)
3. Cfg lint check (`./.github/scripts/cfg-lint-check.py` on changed cfg files)
4. Cartesian syntax check (parse changed cfg files with avocado-vt `cartesian_config.py`)

Do not omit `cfg-lint-check` or `cartesian-syntax-check`.

## CI-Aligned Dependency Setup

### A. CodeCheck dependencies
Install tools required by `CodeCheck` in CI:

```bash
sudo apt install -y python3 python3-pip python3-git python3-ply git shellcheck perl codespell
sudo mkdir -p /usr/share/codespell
sudo wget https://raw.githubusercontent.com/codespell-project/codespell/master/codespell_lib/data/dictionary.txt -O /usr/share/codespell/dictionary.txt
sudo wget https://raw.githubusercontent.com/torvalds/linux/master/scripts/spelling.txt -O /usr/bin/spelling.txt
sudo wget https://raw.githubusercontent.com/torvalds/linux/master/scripts/const_structs.checkpatch -O /usr/bin/const_structs.checkpatch
sudo wget https://raw.githubusercontent.com/torvalds/linux/master/scripts/checkpatch.pl -O /usr/bin/checkpatch.pl
sudo chmod +x /usr/bin/checkpatch.pl
```

If `checkpatch.pl` is missing, this setup step is required before running `pr_check`.

### B. Python 3.9 check dependencies
Use Python 3.9 and install CI dependencies:

```bash
pip install sphinx
pip install -r ./.github/scripts/requirements-ci.txt
```

The executable name is `inspekt` (not `inspektor`).

## Standard Execution Steps
1. Ensure repo is up-to-date and compare base is available:

```bash
git fetch origin --prune
```

2. Run CodeCheck:

```bash
./.github/scripts/pr_check origin/main HEAD
```

3. Run Python 3.9 style/lint check:

```bash
inspekt checkall ./KVM --disable-style E501,E265,W601,E402,E722,E741 --disable-lint=W,R,C,E0601,E1002,E1101,E1103,E1120,F0401,I0011,I1101 --enable-lint W0611,W1201 --no-license-check
```

4. Find changed cfg files and run cfg lint:

```bash
changed_cfg=$(git diff --name-only origin/main...HEAD -- ./KVM/qemu/*.cfg)
if [ -n "$changed_cfg" ]; then
  ./.github/scripts/cfg-lint-check.py $changed_cfg
fi
```

5. Run Cartesian syntax check for each changed cfg:

```bash
if [ -n "$changed_cfg" ]; then
  for cfg in $changed_cfg; do
    cp "$cfg" /tmp/$(basename "$cfg")
    sed -i '1s/^/variants:\n/' /tmp/$(basename "$cfg")
    curl -fsSL https://raw.githubusercontent.com/avocado-framework/avocado-vt/master/virttest/cartesian_config.py \
      | python3 - -f /tmp/$(basename "$cfg")
  done
fi
```

## Dependency Troubleshooting
- `bash: checkpatch.pl: command not found`
  - Run the CodeCheck dependency setup block above.
- `bash: inspekt: command not found`
  - Install Python dependencies from `./.github/scripts/requirements-ci.txt`.
  - Verify with `command -v inspekt`.
- `bash: inspektor: command not found`
  - Expected. Use `inspekt`, not `inspektor`.

## Definition Of Done
- `pr_check` exits with code 0.
- `inspekt checkall` exits with code 0.
- If cfg files changed: `cfg-lint-check.py` exits with code 0.
- If cfg files changed: Cartesian parser check exits with code 0 for each changed cfg.

## Output Contract
Return a compact report:

```text
code_check: PASS|FAIL
python39_check: PASS|FAIL
cfg_lint: PASS|FAIL|SKIP(no cfg changed)
cartesian_syntax: PASS|FAIL|SKIP(no cfg changed)
failed_step: <name or none>
summary: <one-line conclusion>
```
