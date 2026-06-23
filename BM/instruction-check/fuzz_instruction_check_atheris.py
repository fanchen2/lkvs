#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0-only

import sys
import atheris

with atheris.instrument_imports():
    import feature_list


def _safe_text(raw: bytes) -> str:
    return raw.decode("utf-8", errors="ignore")[:32]


def TestOneInput(data: bytes) -> None:
    fdp = atheris.FuzzedDataProvider(data)

    # Test feature_list data structure access patterns
    feature_names = list(feature_list.feature_list.keys())
    if not feature_names:
        return

    # Pick a random feature name from the list
    idx = fdp.ConsumeIntInRange(0, len(feature_names) - 1)
    feature_name = feature_names[idx]

    feature_data = feature_list.feature_list.get(feature_name)
    if not feature_data:
        return

    # Fuzz access to cpuid and platforms
    try:
        cpuid_args = feature_data.get("cpuid", [])
        if cpuid_args:
            # Simulate cpuid_check command-line argument parsing
            arg_idx = fdp.ConsumeIntInRange(0, len(cpuid_args) - 1)
            arg = cpuid_args[arg_idx]
            int(arg)
    except Exception:
        pass

    # Fuzz platform filtering
    try:
        platforms = feature_data.get("platforms", set())
        test_platform = _safe_text(fdp.ConsumeBytes(16))
        in_platform = test_platform in platforms
    except Exception:
        pass

    # Test cpu_family_mapping access
    try:
        family_names = list(feature_list.cpu_family_mapping.keys())
        if family_names:
            fam_idx = fdp.ConsumeIntInRange(0, len(family_names) - 1)
            fam_name = family_names[fam_idx]
            family_tuple = feature_list.cpu_family_mapping.get(fam_name)
            if family_tuple:
                _ = family_tuple[0], family_tuple[1]
    except Exception:
        pass


def main() -> None:
    atheris.Setup(sys.argv, TestOneInput)
    atheris.Fuzz()


if __name__ == "__main__":
    main()
