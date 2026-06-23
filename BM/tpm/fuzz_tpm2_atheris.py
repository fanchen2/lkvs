#!/usr/bin/env python3
# SPDX-License-Identifier: (GPL-2.0 OR BSD-3-Clause)

import sys

import atheris

with atheris.instrument_imports():
    import tpm2


def _safe_text(raw: bytes) -> str:
    return raw.decode("utf-8", errors="ignore")[:32]


def TestOneInput(data: bytes) -> None:
    fdp = atheris.FuzzedDataProvider(data)

    alg = int.from_bytes(fdp.ConsumeBytes(2), "big")
    name = _safe_text(fdp.ConsumeBytes(32))
    rc = int.from_bytes(fdp.ConsumeBytes(2), "big")

    try:
        tpm2.get_digest_size(alg)
    except Exception:
        pass

    try:
        tpm2.get_hash_function(alg)
    except Exception:
        pass

    try:
        tpm2.get_algorithm(name)
    except Exception:
        pass

    nonce = fdp.ConsumeBytes(64)
    hmac = fdp.ConsumeBytes(64)
    session_attributes = int.from_bytes(fdp.ConsumeBytes(1), "big")
    auth_cmd = tpm2.AuthCommand(
        session_handle=int.from_bytes(fdp.ConsumeBytes(4), "big"),
        nonce=nonce,
        session_attributes=session_attributes,
        hmac=hmac,
    )
    try:
        bytes(auth_cmd)
        len(auth_cmd)
    except Exception:
        pass

    sensitive = tpm2.SensitiveCreate(
        user_auth=fdp.ConsumeBytes(64),
        data=fdp.ConsumeBytes(64),
    )
    try:
        bytes(sensitive)
        len(sensitive)
    except Exception:
        pass

    public = tpm2.Public(
        object_type=int.from_bytes(fdp.ConsumeBytes(2), "big"),
        name_alg=int.from_bytes(fdp.ConsumeBytes(2), "big"),
        object_attributes=int.from_bytes(fdp.ConsumeBytes(4), "big"),
        auth_policy=fdp.ConsumeBytes(64),
        parameters=fdp.ConsumeBytes(64),
        unique=fdp.ConsumeBytes(64),
    )
    try:
        bytes(public)
        len(public)
    except Exception:
        pass

    err = tpm2.ProtocolError(int.from_bytes(fdp.ConsumeBytes(4), "big"), rc)
    str(err)


def main() -> None:
    atheris.Setup(sys.argv, TestOneInput)
    atheris.Fuzz()


if __name__ == "__main__":
    main()
