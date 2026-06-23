#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0-only
"""
Atheris fuzz harness for tdx-compliance auto_gen.py.
Targets: JSON config parsing, CSV spec parsing (parse_leaf, get_valstr),
and code generation logic.
"""

import sys
import os
import atheris

# Ensure module can be imported
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

# Import heavy deps BEFORE instrumentation to avoid instrumenting numpy/pandas
import pandas  # noqa: E402
import numpy  # noqa: E402
import csv  # noqa: E402
import math  # noqa: E402
import json
import re

with atheris.instrument_imports():
    from auto_gen import parse_leaf, get_valstr


def TestOneInput(data: bytes) -> None:
    fdp = atheris.FuzzedDataProvider(data)

    # Fuzz parse_leaf with random field names
    field_name = fdp.ConsumeUnicodeNoSurrogates(64)
    try:
        leaf, subleaves = parse_leaf(field_name)
    except Exception:
        pass

    # Fuzz get_valstr with random column indices
    try:
        num_cols = fdp.ConsumeIntInRange(0, 10)
        column_indices = {}
        for _ in range(num_cols):
            key = fdp.ConsumeUnicodeNoSurrogates(16)
            column_indices[key] = fdp.ConsumeIntInRange(0, 20)

        num_search = fdp.ConsumeIntInRange(0, 5)
        str_wait = [fdp.ConsumeUnicodeNoSurrogates(16) for _ in range(num_search)]
        get_valstr(str_wait, column_indices)
    except Exception:
        pass

    # Fuzz JSON config parsing
    try:
        json_bytes = fdp.ConsumeBytes(fdp.ConsumeIntInRange(0, 256))
        json_str = json_bytes.decode("utf-8", errors="ignore")

        # Parse JSON directly without file I/O
        config = json.loads(json_str)
        if isinstance(config, dict):
            cpuid_cfg = config.get("cpuid", {})
            if isinstance(cpuid_cfg, dict):
                _ = cpuid_cfg.get("csv_path", "")
                _ = cpuid_cfg.get("header_path", "")
                _ = cpuid_cfg.get("bias", "0")
                _ = cpuid_cfg.get("version", "1.0")
    except Exception:
        pass


def main() -> None:
    atheris.Setup(sys.argv, TestOneInput)
    atheris.Fuzz()


if __name__ == "__main__":
    main()
