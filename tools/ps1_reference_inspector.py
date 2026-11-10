#!/usr/bin/env python3
"""Validate and inventory a PS1 CUE/BIN or ISO reference without modifying it."""
import argparse
import hashlib
import json
import re
from datetime import datetime, timezone
from pathlib import Path

FILE_RE = re.compile(r'^\s*FILE\s+"?(.+?)"?\s+(BINARY|MOTOROLA|WAVE|MP3|AIFF)\s*$', re.I)
TRACK_RE = re.compile(r'^\s*TRACK\s+(\d+)\s+([A-Z0-9/]+)\s*$', re.I)
INDEX_RE = re.compile(r'^\s*INDEX\s+(\d+)\s+(\d{2}:\d{2}:\d{2})\s*$', re.I)

def sha256(path):
    h = hashlib.sha256()
    with path.open("rb") as f:
        for chunk in iter(lambda: f.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()

def parse_cue(path):
    tracks, errors, current_file, current_track = [], [], None, None
    for n, line in enumerate(path.read_text(encoding="utf-8-sig", errors="replace").splitlines(), 1):
        s = line.strip()
        if not s or s.upper().startswith(("REM ", "TITLE ", "PERFORMER ", "CATALOG ", "FLAGS ", "PREGAP ", "POSTGAP ")):
            continue
        m = FILE_RE.match(line)
        if m:
            current_file = m.group(1).strip().strip('"')
            continue
        m = TRACK_RE.match(line)
        if m:
            current_track = {"number": int(m.group(1)), "mode": m.group(2).upper(), "file": current_file, "indexes": {}}
            if not current_file:
                errors.append(f"Line {n}: TRACK appears before FILE.")
            tracks.append(current_track)
            continue
        m = INDEX_RE.match(line)
        if m and current_track is not None:
            current_track["indexes"][m.group(1)] = m.group(2)
    if not tracks:
        errors.append("No TRACK entries found.")
    if not any("01" in t["indexes"] for t in tracks):
        errors.append("No INDEX 01 entry found.")
    return tracks, errors

def inspect(source):
    source = source.expanduser().resolve()
    report = {
        "schema_version": "1.0",
        "created_utc": datetime.now(timezone.utc).isoformat(),
        "source": str(source),
        "source_name": source.name,
        "disc_type": source.suffix.lower().lstrip("."),
        "status": "error",
        "tracks": [], "files": [], "warnings": [], "errors": [],
        "analysis_limits": [
            "Disc metadata cannot reveal the full gameplay loop, controls, level flow, timing, or balance.",
            "Gameplay analysis needs video, screenshots, or observation notes from emulator play.",
            "Use only broad design lessons; create original code, art, characters, music, levels, story, and names."
        ]
    }
    if not source.is_file():
        report["errors"].append(f"Input file not found: {source}")
        return report
    ext = source.suffix.lower()
    if ext == ".cue":
        try:
            tracks, errors = parse_cue(source)
            report["tracks"] = tracks
            report["errors"].extend(errors)
        except OSError as e:
            report["errors"].append(str(e))
        names = list(dict.fromkeys(t["file"] for t in report["tracks"] if t.get("file")))
        candidates = [(source.parent / name).resolve() for name in names]
        for candidate in candidates:
            if not candidate.is_file():
                report["errors"].append(f"CUE references missing file: {candidate.name}")
    elif ext == ".iso":
        candidates = [source]
        report["warnings"].append("ISO supplied directly; no CUE track map is available.")
    elif ext == ".bin":
        candidates = [source]
        report["warnings"].append("BIN supplied without CUE; track boundaries cannot be verified.")
    else:
        report["errors"].append("Use a .cue or .iso file. For BIN, provide its matching CUE.")
        return report
    for p in candidates:
        if p.is_file():
            report["files"].append({"name": p.name, "size_bytes": p.stat().st_size, "sha256": sha256(p)})
    if report["files"]:
        report["status"] = "validated" if not report["errors"] else "partial"
    return report

def main():
    ap = argparse.ArgumentParser(description="Inspect PS1 CUE/BIN or ISO reference metadata.")
    ap.add_argument("input", type=Path)
    ap.add_argument("--output", type=Path, default=Path("reports"))
    args = ap.parse_args()
    report = inspect(args.input)
    args.output.mkdir(parents=True, exist_ok=True)
    stem = args.input.stem
    json_path = args.output / f"{stem}-reference-report.json"
    brief_path = args.output / f"{stem}-ai-analysis-brief.md"
    json_path.write_text(json.dumps(report, indent=2), encoding="utf-8")
    files = "\n".join(f"- {f['name']} — {f['size_bytes']} bytes — SHA-256 {f['sha256']}" for f in report["files"]) or "- None"
    tracks = "\n".join(f"- Track {t['number']}: {t['mode']}, file {t['file']}, indexes {t['indexes']}" for t in report["tracks"]) or "- Not available"
    brief = f"""# AI reference analysis brief

Status: **{report['status']}**
Input: `{report['source_name']}`

## Disc files
{files}

## Tracks
{tracks}

## Errors
{chr(10).join('- ' + e for e in report['errors']) or '- None'}

## Warnings
{chr(10).join('- ' + w for w in report['warnings']) or '- None'}

## Instructions for the AI agent
- Treat this report as disc metadata, not proof of gameplay behavior.
- Request gameplay recordings, screenshots, or observation notes before describing mechanics.
- Label findings as observed, inferred, or unknown.
- Learn high-level genre patterns only. Do not copy source code, characters, art, music, dialogue, maps, logos, or proprietary assets.
- Design a distinct original game and implement it as a separate PSn00bSDK project.
- Produce a PS1 feasibility plan, original asset list, milestones, and emulator acceptance tests.

## Limitations
{chr(10).join('- ' + x for x in report['analysis_limits'])}
"""
    brief_path.write_text(brief, encoding="utf-8")
    print(f"Status: {report['status']}")
    print(f"Report: {json_path}")
    print(f"AI brief: {brief_path}")
    if report["errors"]:
        raise SystemExit(1)

if __name__ == "__main__":
    main()
