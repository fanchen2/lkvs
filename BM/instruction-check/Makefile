# SPDX-License-Identifier: GPL-2.0-only

PYTHON ?= python3
ATHERIS_FLAGS ?= -max_total_time=30 -print_final_stats=1

fuzz:
	$(PYTHON) fuzz_instruction_check_atheris.py $(ATHERIS_FLAGS)

fuzz-run: fuzz

clean:
	rm -rf crash-* leak-* oom-* slow-* && find . -type d -name '__pycache__' -exec rm -rf {} + 2>/dev/null || true

.PHONY: fuzz fuzz-run clean
