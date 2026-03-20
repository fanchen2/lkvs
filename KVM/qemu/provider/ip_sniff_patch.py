# SPDX-License-Identifier: GPL-2.0-only
# Copyright (c) 2026 Intel Corporation
"""
Monkey-patch avocado-vt IP sniffing to support the ``ip_sniff_iface``
Cartesian parameter, without requiring changes to avocado-vt itself.

Usage — add to any test script that needs it:

    from provider import ip_sniff_patch  # pylint: disable=unused-import

Then set the interface in the .cfg file:

    ip_sniff_iface = virbr0

If ``ip_sniff_iface`` is not set, the default "any" is used (original behaviour).

Implementation note
-------------------
avocado-vt sniffer classes (TcpdumpSniffer, TShark*) hard-code the network
interface as ``-i any`` inside the class-level ``options`` string.
``Env.start_ip_sniffing`` constructs sniffers from those class options without
any interface override mechanism.

This module intercepts ``Env.start_ip_sniffing`` and, before delegating to the
original method, rewrites the class-level ``options`` of every registered
sniffer class so that ``-i any`` is replaced with the configured interface.
The original options strings are snapshotted at import time and are always used
as the base, so the patch is idempotent regardless of how many times
``start_ip_sniffing`` is called.
"""

from virttest import ip_sniffing, utils_env

_ORIG_START_IP_SNIFFING = utils_env.Env.start_ip_sniffing
# Snapshot original class-level options at import time so each call to
# start_ip_sniffing always starts from the unmodified template.
_ORIG_OPTIONS = {s_cls: s_cls.options for s_cls in ip_sniffing.Sniffers}


def _patched_start_ip_sniffing(self, params):
    iface = params.get("ip_sniff_iface", "any")
    for s_cls in ip_sniffing.Sniffers:
        # Replace ` any ` with ` {iface} ` to handle both merged and separate option forms:
        # - Merged: -tnpvvvi any 'port...' or -npi any -T fields...
        # - Separate: -i any 'port...' or -i any -T fields...
        s_cls.options = _ORIG_OPTIONS[s_cls].replace(" any ", " %s " % iface)
    _ORIG_START_IP_SNIFFING(self, params)


if utils_env.Env.start_ip_sniffing is not _patched_start_ip_sniffing:
    utils_env.Env.start_ip_sniffing = _patched_start_ip_sniffing
