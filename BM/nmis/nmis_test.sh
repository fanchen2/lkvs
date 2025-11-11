#!/bin/bash
# SPDX-License-Identifier: GPL-2.0-only
# Copyright (c) 2025 Intel Corporation
# Author:   Farrah Chen <farrah.chen@intel.com>
# @Desc This script verify NMI Source tests

cd "$(dirname "$0")" 2>/dev/null || exit 1
source ../.env


: "${CASE_NAME:=""}"

usage() {
  cat <<__EOF
  usage: ./${0##*/} [-t TESTCASE_ID] [-H]
  -t  TEST CASE ID
  -H  show this
__EOF
}

# Cmdline test: Check FRED and NMIS CPUID, verify if FRED is enabled, check if NMIS error
nmi_enable_test() {
  #CPUID.0x7.1.EAX[17] == 1
  do_cmd "cpuid_check 7 0 1 0 a 17"
  #CPUID.0x7.1.EAX[20] == 1
  do_cmd "cpuid_check 7 0 1 0 a 20"
  do_cmd "grep -q 'fred=on' '/proc/cmdline'"
  do_cmd "dmesg | grep 'Initialize FRED on CPU'"
  should_fail "dmesg | grep 'NMI without source information! Disable source reporting.'"
}

# NMI Source check for IPI by NMI kselftest
nmis_ipi_test() {
    nmi_enable_test
    general_test.sh -t kconfig -k "CONFIG_X86_FRED=y"
    if [ $? -ne 0 ]; then
        die "CONFIG_DEBUG_NMI_SELFTEST is not set in Kconfig, ipi test cannot be executed!"
    fi
    full_dmesg_check "NMI testsuite" "$CONTAIN"
    full_dmesg_check "Good, all   2 testcases passed!" "$CONTAIN"
    dmesg_not_contain "NMI without source information! Disable source reporting."
}

# Check NMI Source for local interrupts triggered by CPU through local vector table (LVT), such as PMI
nmis_pmi_test() {
    nmi_enable_test
    cat /proc/interrupts | grep NMI > /tmp/pmi_before
    perf top &> /dev/null &
    pid=$!
    sleep 5
    kill -9 $pid
    should_fail "dmesg | grep 'NMI without source information! Disable source reporting.'"

    cat /proc/interrupts | grep NMI > /tmp/pmi_after
    local cpu_num=$(cat /tmp/pmi_before | grep -o -E '[0-9]+' | wc -l)
    local i=2
    local end=$((cpu_num+1))

    for i in $(seq 2 $end)
    do
        local before=$(cat /tmp/pmi_before | awk '{print $'$i'}')
        local after=$(cat /tmp/pmi_after | awk '{print $'$i'}')
        if [ $after -le $before ]; then
            die "PMI test failed!"
        fi
    done
}

# NMI Source IPI backtrace test
nmis_ipi_bt_test() {
    nmi_enable_test
    echo 1 > /proc/sysrq-trigger
    full_dmesg_check "Changing Loglevel" "$CONTAIN"
    full_dmesg_check "Loglevel set to 1" "$CONTAIN"
    cat /proc/interrupts | grep NMI > /tmp/nmi_before
    echo l > /proc/sysrq-trigger
    full_dmesg_check "Show backtrace of all active CPUs" "$CONTAIN"
    cat /proc/interrupts | grep NMI > /tmp/nmi_after
    dmesg_not_contain "NMI without source information! Disable source reporting."

    local cpu_num=$(cat /tmp/nmi_before | grep -o -E '[0-9]+' | wc -l)
    local i=2
    local inc=0
    local end=$((cpu_num+1))

    for i in $(seq 2 $end)
    do
        local before=$(cat /tmp/nmi_before | awk '{print $'$i'}')
        local after=$(cat /tmp/nmi_after | awk '{print $'$i'}')
        if [ $after -gt $before ]; then
            inc=$((inc+1))
        fi
    done

    if [ $inc -ne $((cpu_num - 1)) ]; then
        die "CPU backtrace IPI test failed!"
    fi
}

nmis_test() {
  case $TEST_SCENARIO in
    enabling)
      nmi_enable_test
      ;;
    ipi)
      nmis_ipi_test
      ;;
    pmi)
      nmis_pmi_test
      ;;
    ipi_bt)
      nmis_ipi_bt_test
      ;;
    *)
      echo "Invalid option"
      return 1
  esac
  return 0
 }

while getopts :t:H arg; do
  case $arg in
    t)
      TEST_SCENARIO=$OPTARG
      ;;
    H)
      usage && exit 0
      ;;
    \?)
      usage
      die "Invalid Option -$OPTARG"
      ;;
    :)
      usage
      die "Option -$OPTARG requires an argument."
      ;;
  esac
done

nmis_test "$@"
# Call teardown for passing case
exec_teardown
