#!/usr/bin/env python3
"""Apply CI-provided LoRaWAN identity values to se-identity.h before build.

The production bundle manifest is generated from the same TELEMETRY_LORAWAN_*
environment variables. This script makes the compiled firmware use those
values too, closing the gap between manifest evidence and device behavior.
"""

from __future__ import annotations

import argparse
import os
from pathlib import Path
import re
import sys


PROJECT_DIR = Path(__file__).resolve().parents[1]
DEFAULT_IDENTITY_FILE = PROJECT_DIR / "LoRaWAN" / "App" / "se-identity.h"


def clean_hex(value: str, expected_len: int, name: str) -> str:
    cleaned = re.sub(r"[^0-9A-Fa-f]", "", value).upper()
    if len(cleaned) != expected_len:
        raise ValueError(f"{name} must have {expected_len} hex characters, got {len(cleaned)}")
    return cleaned


def mask(value: str, head: int = 4, tail: int = 4) -> str:
    if len(value) <= head + tail:
        return "*" * len(value)
    return f"{value[:head]}{'*' * (len(value) - head - tail)}{value[-tail:]}"


def eui_define(hex_value: str) -> str:
    return "{ " + ", ".join(f"0x{hex_value[i:i + 2]}" for i in range(0, 16, 2)) + " }"


def key_define(hex_value: str) -> str:
    return ",".join(hex_value[i:i + 2] for i in range(0, 32, 2))


def replace_define(text: str, name: str, value: str) -> str:
    pattern = re.compile(rf"^(#define\s+{re.escape(name)}\s+).*$", re.MULTILINE)
    text, count = pattern.subn(rf"\g<1>{value}", text, count=1)
    if count != 1:
        raise ValueError(f"Could not find unique #define {name}")
    return text


def load_identity_from_env() -> tuple[str, str, str, str] | None:
    raw_device_eui = os.environ.get("TELEMETRY_LORAWAN_DEVICE_EUI", "")
    raw_join_eui = os.environ.get("TELEMETRY_LORAWAN_JOIN_EUI", "")
    raw_app_key = os.environ.get("TELEMETRY_LORAWAN_APP_KEY", "")
    raw_nwk_key = os.environ.get("TELEMETRY_LORAWAN_NWK_KEY", "")

    if not any([raw_device_eui, raw_join_eui, raw_app_key, raw_nwk_key]):
        return None

    missing = [
        name
        for name, value in [
            ("TELEMETRY_LORAWAN_DEVICE_EUI", raw_device_eui),
            ("TELEMETRY_LORAWAN_JOIN_EUI", raw_join_eui),
            ("TELEMETRY_LORAWAN_APP_KEY", raw_app_key),
        ]
        if not value
    ]
    if missing:
        raise ValueError("Missing required LoRaWAN identity variables: " + ", ".join(missing))

    device_eui = clean_hex(raw_device_eui, 16, "TELEMETRY_LORAWAN_DEVICE_EUI")
    join_eui = clean_hex(raw_join_eui, 16, "TELEMETRY_LORAWAN_JOIN_EUI")
    app_key = clean_hex(raw_app_key, 32, "TELEMETRY_LORAWAN_APP_KEY")
    nwk_key = clean_hex(raw_nwk_key, 32, "TELEMETRY_LORAWAN_NWK_KEY") if raw_nwk_key else app_key

    if device_eui == join_eui:
        raise ValueError("TELEMETRY_LORAWAN_DEVICE_EUI and TELEMETRY_LORAWAN_JOIN_EUI must differ")

    return device_eui, join_eui, app_key, nwk_key


def apply_identity(identity_file: Path, identity: tuple[str, str, str, str]) -> None:
    device_eui, join_eui, app_key, nwk_key = identity
    text = identity_file.read_text(encoding="utf-8")

    text = replace_define(text, "STATIC_DEVICE_EUI", "1")
    text = replace_define(text, "LORAWAN_DEVICE_EUI", eui_define(device_eui))
    text = replace_define(text, "LORAWAN_JOIN_EUI", eui_define(join_eui))
    text = replace_define(text, "LORAWAN_APP_KEY", key_define(app_key))
    text = replace_define(text, "LORAWAN_NWK_KEY", key_define(nwk_key))

    identity_file.write_text(text, encoding="utf-8", newline="\n")


def main() -> int:
    parser = argparse.ArgumentParser(description="Apply TELEMETRY_LORAWAN_* values to se-identity.h.")
    parser.add_argument("--identity-file", type=Path, default=DEFAULT_IDENTITY_FILE)
    parser.add_argument("--require-custom-keys", action="store_true")
    args = parser.parse_args()

    try:
        identity = load_identity_from_env()
        if identity is None:
            if args.require_custom_keys:
                raise ValueError("TELEMETRY_LORAWAN_* variables are required for this build")
            print("[identity] No TELEMETRY_LORAWAN_* overrides provided; leaving se-identity.h unchanged.")
            return 0

        apply_identity(args.identity_file, identity)
        device_eui, join_eui, app_key, nwk_key = identity
        same_keys = "yes" if app_key == nwk_key else "no"
        print("[identity] Applied LoRaWAN identity to se-identity.h")
        print(f"[identity] DeviceEUI={mask(device_eui)} JoinEUI={mask(join_eui)}")
        print(f"[identity] AppKey={mask(app_key)} NwkKey={mask(nwk_key)} SameKeys={same_keys}")
        return 0
    except Exception as exc:
        print(f"[identity] ERROR: {exc}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
