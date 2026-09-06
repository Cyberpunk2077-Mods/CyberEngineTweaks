#!/usr/bin/env python3
"""Verify CET locale parity, placeholders, and source localization keys."""
from __future__ import annotations

import json
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
LOCALES = ROOT / "locales"
EXPECTED_CODES = {
    "ar-ar", "cz-cz", "de-de", "en-us", "es-es", "es-mx", "fr-fr",
    "hu-hu", "it-it", "jp-jp", "kr-kr", "pl-pl", "pt-br", "ru-ru",
    "th-th", "tr-tr", "ua-ua", "zh-cn", "zh-tw",
}
PLACEHOLDER = re.compile(r"%[a-zA-Z]|\{\}")
KEY_USE = re.compile(r'loc\.Get\("([^"]+)"\)|GetLocalization\(\)\.Get\("([^"]+)"\)')
DEFAULT_KEY = re.compile(r'\{"([^"]+)",\s*"')


def main() -> None:
    paths = sorted(LOCALES.glob("*.json"))
    codes = {p.stem for p in paths}
    if codes != EXPECTED_CODES:
        raise SystemExit(f"locale set mismatch: missing={EXPECTED_CODES-codes}, extra={codes-EXPECTED_CODES}")

    values = {p.stem: json.loads(p.read_text(encoding="utf-8")) for p in paths}
    english = values["en-us"]
    errors: list[str] = []
    for code, data in values.items():
        missing = english.keys() - data.keys()
        extra = data.keys() - english.keys()
        if missing or extra:
            errors.append(f"{code}: missing={sorted(missing)}, extra={sorted(extra)}")
        for key in english.keys() & data.keys():
            if not isinstance(data[key], str) or not data[key].strip():
                errors.append(f"{code}: empty/non-string value for {key}")
            if PLACEHOLDER.findall(english[key]) != PLACEHOLDER.findall(data[key]):
                errors.append(f"{code}: placeholder mismatch for {key}")

    source = "\n".join(p.read_text(encoding="utf-8") for p in (ROOT / "src").rglob("*.cpp"))
    used = {a or b for a, b in KEY_USE.findall(source)}
    missing_used = used - english.keys()
    if missing_used:
        errors.append(f"source keys absent from en-us: {sorted(missing_used)}")

    defaults = (ROOT / "src/overlay/Localization.cpp").read_text(encoding="utf-8")
    default_keys = set(DEFAULT_KEY.findall(defaults[defaults.index("kEnglishDefaults"):defaults.index("} // namespace")]))
    if default_keys != set(english):
        errors.append(f"built-in fallback mismatch: missing={sorted(set(english)-default_keys)}, extra={sorted(default_keys-set(english))}")

    if errors:
        raise SystemExit("\n".join(errors))
    print(f"Verified {len(paths)} locales, {len(english)} keys, placeholders, source uses, and built-in fallbacks.")


if __name__ == "__main__":
    main()
