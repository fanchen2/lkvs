# PCI/PCIe Register Check Tool

## User-mode fuzzing

This component provides a libFuzzer entrypoint for testing hex string parsing:

```
cd BM/tools/pcie
make fuzz
make fuzz-run
```

- Fuzz target: Hex string and range parsing (sscanf patterns)
- Binary: `fuzz_pcie_check`
- Sanitizers: AddressSanitizer + UBSan
