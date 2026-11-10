#!/usr/bin/env python3
"""
i18n-coverage.py — Report per-locale translation coverage against en.json.

Counts keys that are absent from a locale file and keys whose value is an empty
string (I18n::Get() treats both as untranslated and falls back to English).

Usage:
    python tools/i18n-coverage.py                  # Table for every locale
    python tools/i18n-coverage.py --locale zh_CN   # Detail for one locale
    python tools/i18n-coverage.py --locale zh_CN --report   # Also list keys identical to English
    python tools/i18n-coverage.py --locale zh_CN --check    # CI mode: exit 1 if not 100% covered
"""

import argparse
import json
import sys
from pathlib import Path


def find_translations_dir() -> Path:
    d = Path(__file__).resolve().parent.parent
    if (d / "CMakeLists.txt").exists():
        return d / "package" / "SKSE" / "Plugins" / "CommunityShaders" / "Translations"
    d = Path.cwd()
    while d != d.parent:
        if (d / "CMakeLists.txt").exists():
            return d / "package" / "SKSE" / "Plugins" / "CommunityShaders" / "Translations"
        d = d.parent
    print("Error: Could not find project root (CMakeLists.txt)", file=sys.stderr)
    sys.exit(1)


def load(path: Path) -> dict:
    with open(path, "r", encoding="utf-8") as f:
        data = json.load(f)
    if not isinstance(data, dict):
        raise ValueError(f"{path.name}: root must be a JSON object")
    return {k: v for k, v in data.items() if k != "_meta"}


def measure(en: dict, locale: dict) -> tuple[int, int, int]:
    missing = sum(1 for k in en if k not in locale)
    empty = sum(1 for k in en if k in locale and not str(locale[k]).strip())
    identical = sum(1 for k in en if k in locale and locale[k] == en[k])
    return missing, empty, identical


def main():
    if sys.stdout.encoding != "utf-8":
        sys.stdout.reconfigure(encoding="utf-8")

    parser = argparse.ArgumentParser(description="Report translation coverage against en.json.")
    parser.add_argument("--locale", help="Locale code to report in detail (e.g. zh_CN)")
    parser.add_argument("--report", action="store_true",
                        help="With --locale, also list keys whose value equals the English text")
    parser.add_argument("--check", action="store_true",
                        help="With --locale, exit 1 when any key is missing or empty")
    args = parser.parse_args()

    translations = find_translations_dir()
    en_path = translations / "en.json"
    if not en_path.exists():
        print("Error: en.json not found", file=sys.stderr)
        sys.exit(1)

    en = load(en_path)

    if not args.locale:
        print(f"Reference: en.json ({len(en)} keys)\n")
        print(f"{'locale':<10} {'keys':>6} {'missing':>8} {'empty':>7} {'coverage':>9}")
        print("-" * 44)
        for path in sorted(translations.glob("*.json")):
            if path.name == "en.json":
                continue
            try:
                locale = load(path)
            except (json.JSONDecodeError, ValueError) as e:
                print(f"{path.stem:<10} SKIP ({e})", file=sys.stderr)
                continue
            missing, empty, _identical = measure(en, locale)
            covered = len(en) - missing - empty
            print(f"{path.stem:<10} {len(locale):>6} {missing:>8} {empty:>7} {covered / len(en):>8.1%}")
        return

    locale_path = translations / f"{args.locale}.json"
    if not locale_path.exists():
        print(f"Error: {locale_path.name} not found", file=sys.stderr)
        sys.exit(1)

    locale = load(locale_path)
    missing, empty, identical = measure(en, locale)
    covered = len(en) - missing - empty
    print(f"Locale: {args.locale}")
    print(f"  en.json keys: {len(en)}")
    print(f"  missing:      {missing}")
    print(f"  empty value:  {empty}")
    print(f"  coverage:     {covered / len(en):.1%}")

    if args.report:
        same = [k for k in en if k in locale and locale[k] == en[k]]
        print(f"\n{len(same)} key(s) identical to English (expected for brand and acronym values):")
        for k in same:
            print(f"  {k} = {en[k]}")
        if identical and not same:
            pass

    if args.check and (missing or empty):
        print("\nFAIL: every key needs a non-empty translation for this locale.", file=sys.stderr)
        sys.exit(1)


if __name__ == "__main__":
    main()