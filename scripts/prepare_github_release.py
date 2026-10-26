#!/usr/bin/env python3
"""Prepare a clean GitHub source tree and release assets using an explicit allowlist."""

import argparse
import hashlib
import json
import re
import shutil
import tarfile
import zipfile
from pathlib import Path


VERSION = "01.04"
UPSTREAM = "01955d7c1a4ecdff4245aa6e3fc08fd64625e4a9"


def sha256(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def copy_tree(source, destination):
    destination.mkdir(parents=True, exist_ok=True)
    for path in sorted(source.rglob("*")):
        if path.is_symlink():
            raise ValueError(f"Unexpected symlink in export: {path}")
        if path.is_file():
            target = destination / path.relative_to(source)
            target.parent.mkdir(parents=True, exist_ok=True)
            shutil.copy2(path, target)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output", type=Path, default=Path("publish"))
    args = parser.parse_args()
    root = Path(__file__).resolve().parents[1]
    output = args.output.resolve()
    if output != root / "publish" or output.exists():
        raise SystemExit("Use a new project-local publish/ directory; existing exports are not overwritten.")

    vpk = root / "build" / f"TsukihimeVita-{VERSION}.vpk"
    if not vpk.is_file():
        raise SystemExit(f"Build the release first: {vpk.name}")
    with zipfile.ZipFile(vpk) as archive:
        if archive.testzip():
            raise SystemExit("VPK integrity check failed")
        allowed = {
            "eboot.bin", "sce_sys/param.sfo", "sce_sys/icon0.png",
            "sce_sys/livearea/contents/bg.png",
            "sce_sys/livearea/contents/startup.png",
            "sce_sys/livearea/contents/template.xml",
        }
        if set(archive.namelist()) != allowed:
            raise SystemExit("Unexpected files in the release VPK")

    repository = output / "tsukihime-vita"
    release = output / "release-assets"
    repository.mkdir(parents=True)
    release.mkdir()
    for name in (".gitignore", "CMakeLists.txt", "README.md", "LICENSE", "THIRD_PARTY.md", "CHANGELOG.md"):
        shutil.copy2(root / name, repository / name)
    for name in ("assets/livearea", "diagnostics", "docs", "releases", "scripts", "engine/license", "engine/src/onsjh", "engine/src/common"):
        copy_tree(root / name, repository / name)
    shutil.copy2(root / "engine/UPSTREAM.md", repository / "engine/UPSTREAM.md")

    # Reject accidental private paths, game data, SDK files, and nested Git data.
    for path in repository.rglob("*"):
        if not path.is_file():
            continue
        relative = path.relative_to(repository)
        if any(part in {".git", ".aws", ".codex", ".agents", ".vitasdk", "Tsukihime", "build", "tools"} for part in relative.parts):
            raise SystemExit(f"Forbidden path in public source export: {relative}")
        if path.suffix.lower() in {".nsa", ".sar", ".ogg", ".ttf", ".exe", ".dll", ".vpk"} or path.name == "nscript.dat":
            raise SystemExit(f"Unexpected game or binary file in source export: {relative}")

    # Authored public documents use plain text and no emoji characters. Upstream
    # notices are retained verbatim and are not rewritten by this check.
    emoji = re.compile("[\U0001F000-\U0001FAFF\u2600-\u27BF]")
    for path in [repository / "README.md", repository / "THIRD_PARTY.md", repository / "CHANGELOG.md", *list((repository / "docs").glob("*.md")), *list((repository / "releases").glob("*.md"))]:
        text = path.read_text(encoding="utf-8")
        if emoji.search(text) or "/home/bashar/" in text or "/media/bashar/" in text:
            raise SystemExit(f"Private path or emoji in public documentation: {path.name}")

    shutil.copy2(vpk, release / vpk.name)
    source = release / f"TsukihimeVita-{VERSION}-source.tar.gz"
    with tarfile.open(source, "w:gz") as archive:
        archive.add(repository, arcname=f"TsukihimeVita-{VERSION}-source")
    checksum_files = (release / vpk.name, source)
    (release / "SHA256SUMS").write_text(
        "".join(f"{sha256(path)}  {path.name}\n" for path in checksum_files), encoding="utf-8"
    )
    manifest = {
        "repository_name": "tsukihime-vita", "release_tag": "v1.04", "app_version": VERSION,
        "title_id": "TSUKI0001", "upstream_commit": UPSTREAM,
        "source_files": sum(path.is_file() for path in repository.rglob("*")),
        "release_assets": [{"name": path.name, "bytes": path.stat().st_size, "sha256": sha256(path)} for path in checksum_files],
    }
    (output / "release-manifest.json").write_text(json.dumps(manifest, indent=2) + "\n", encoding="utf-8")
    print(json.dumps(manifest, indent=2))
    print(f"Repository source: {repository}")
    print(f"Release files: {release}")


if __name__ == "__main__":
    main()
