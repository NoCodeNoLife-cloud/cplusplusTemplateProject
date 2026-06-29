#!/usr/bin/env python3
"""
Migrate #include paths for Phase 1 of cppforge project restructuring.

This script updates all #include "..." paths in source files to use the new
<cppforge/...> format after headers were moved from common/src/ and log/src/
to common/include/cppforge/ and log/include/cppforge/glog/.

Usage: python scripts/migrate_includes.py
Run from the project root directory.
"""

import os
import re
import subprocess
from pathlib import Path
from collections import defaultdict

# ─── Configuration ──────────────────────────────────────────────────────────
PROJECT_ROOT = Path(__file__).resolve().parent.parent
os.chdir(PROJECT_ROOT)

# Old include root directories
COMMON_OLD = "common/src"
LOG_OLD = "log/src"

# New include prefix in #include <...> format
COMMON_NEW_PREFIX = "cppforge/"
LOG_NEW_PREFIX = "cppforge/glog/"

# File patterns to scan
SCAN_PATTERNS = [
    "common/src/**/*.cc",
    "common/include/cppforge/**/*.hpp",
    "log/src/**/*.cc",
    "log/include/cppforge/glog/**/*.hpp",
    "client/src/**/*.cc",
    "client/src/**/*.hpp",
    "client/src/**/*.h",
    "server/src/**/*.cc",
    "server/src/**/*.hpp",
    "ci/gtest/src/**/*.cc",
    "ci/benchmark/src/**/*.cc",
]

# Includes that should NOT be modified
EXCLUDE_PATTERNS = [
    # Local config headers (not moved)
    re.compile(r'#include\s+"config/ConfigParam\.h"'),
    re.compile(r'#include\s+"config/ConfigParam\.hpp"'),
    # Server source references (not moved, use relative paths)
    re.compile(r'#include\s+"\.\./\.\./server/src/'),
    # Generated/proto files
    re.compile(r'#include\s+"generated/'),
    # External libraries included with quotes
    re.compile(r'#include\s+"rapidjson/'),
]


def get_moved_headers():
    """Parse git status to get the mapping of old path -> new cppforge path."""
    result = subprocess.run(
        ["git", "status", "--short"],
        capture_output=True, text=True, check=True
    )

    # Mapping from old include path (relative to common/src/ or log/src/)
    #   to new include path (relative to cppforge/)
    # Also track basename -> list of (directory, cppforge_path) for disambiguation
    path_mapping = {}      # full old path -> new cppforge path
    basename_map = defaultdict(list)  # basename -> list of (dirname_old, cppforge_path)
    basename_includes = set()  # Set of basenames that appear in multiple directories

    for line in result.stdout.strip().split('\n'):
        if not line.strip():
            continue
        # Pattern: "R  oldpath -> newpath"
        parts = line.strip().split()
        if len(parts) >= 4 and parts[0] == 'R':
            old_path = parts[1]
            new_path = parts[3]

            # Determine if it's a common or log header
            if old_path.startswith(COMMON_OLD + "/"):
                rel_path = old_path[len(COMMON_OLD) + 1:]  # e.g., "auth/AuthenticationException.hpp"
                cppforge_path = COMMON_NEW_PREFIX + rel_path  # "cppforge/auth/AuthenticationException.hpp"
                path_mapping[rel_path] = cppforge_path
                basename = os.path.basename(rel_path)
                basename_map[basename].append((os.path.dirname(rel_path), cppforge_path))

            elif old_path.startswith(LOG_OLD + "/"):
                rel_path = old_path[len(LOG_OLD) + 1:]  # e.g., "config/GLogConfigurator.hpp"
                cppforge_path = LOG_NEW_PREFIX + rel_path  # "cppforge/glog/config/GLogConfigurator.hpp"
                path_mapping[rel_path] = cppforge_path
                basename = os.path.basename(rel_path)
                basename_map[basename].append((os.path.dirname(rel_path), cppforge_path))

    # Find basenames that appear in multiple directories
    for basename, entries in basename_map.items():
        dirs = set(e[0] for e in entries)
        if len(dirs) > 1:
            basename_includes.add(basename)

    return path_mapping, basename_map, basename_includes


def collect_source_files():
    """Get all source files that need scanning using glob patterns."""
    files = []
    for pattern in SCAN_PATTERNS:
        matches = list(Path().glob(pattern))
        files.extend(matches)
    return sorted(set(files))


def should_exclude(line):
    """Check if an include line should be excluded from modification."""
    for pattern in EXCLUDE_PATTERNS:
        if pattern.search(line):
            return True
    return False


def find_cppforge_path(quoted_path, source_file_rel, path_mapping, basename_map, basename_includes):
    """
    Given a quoted include path, find the corresponding cppforge path.

    Returns the cppforge path string or None if no match found.
    """
    # First try exact match
    if quoted_path in path_mapping:
        return path_mapping[quoted_path]

    # Try matching as a path relative to common/src/ or log/src/
    common_full = os.path.join(COMMON_OLD, quoted_path)
    log_full = os.path.join(LOG_OLD, quoted_path)

    # Normalize to forward slashes
    common_full = common_full.replace('\\', '/')
    log_full = log_full.replace('\\', '/')

    # Check if this was a common or log header
    # (alternative lookup using the old full path)
    # This isn't perfect, let's try directory-based resolution

    # Try basename resolution
    basename = os.path.basename(quoted_path)
    if basename in basename_map:
        entries = basename_map[basename]
        if len(entries) == 1:
            # Only one match, unambiguous
            return entries[0][1]
        else:
            # Multiple matches - disambiguate by source file's location
            sf = source_file_rel.replace('\\', '/')

            # Determine which old src root the source file is in
            source_in_common = sf.startswith("common/")
            source_in_log = sf.startswith("log/")

            # Check if the source file's directory matches one of the entries
            source_dir = os.path.dirname(sf).replace('\\', '/')

            for dirname, cppforge_path in entries:
                # If source is in the same directory as the old header
                if dirname:
                    # For common headers: common/src/<dirname>
                    old_header_dir = f"common/src/{dirname}"
                    new_include_dir = f"common/include/{cppforge_path}"
                else:
                    old_header_dir = "common/src"
                    new_include_dir = f"common/include/{cppforge_path}"

                # Check if source file is in the same directory as the old header
                # This handles .cc files in common/src/ that include by basename
                if source_dir == old_header_dir.replace('\\', '/'):
                    return cppforge_path

                # For .hpp files, they're now in common/include/cppforge/...
                # So same-directory includes still resolved to the same hpp
                if cppforge_path.startswith(COMMON_NEW_PREFIX):
                    hpp_dir = f"common/include/{cppforge_path}"
                    if os.path.dirname(hpp_dir).replace('\\', '/') == source_dir:
                        return cppforge_path

                if cppforge_path.startswith(LOG_NEW_PREFIX):
                    hpp_dir = f"log/include/{cppforge_path}"
                    if os.path.dirname(hpp_dir).replace('\\', '/') == source_dir:
                        return cppforge_path

            # Still ambiguous - check if source references include path prefix
            if basename in basename_includes:
                # Could try matching by checking if source file's directory
                # contains the include path's directory as a segment
                for dirname, cppforge_path in entries:
                    if dirname:
                        # Try: source file path contains dirname
                        if f"/{dirname}/" in sf or sf.startswith(f"{dirname}/"):
                            return cppforge_path

            # Last resort: if source is in log/ and the entry is a log header
            if source_in_log:
                for dirname, cppforge_path in entries:
                    if cppforge_path.startswith(LOG_NEW_PREFIX):
                        return cppforge_path

            if source_in_common and not source_in_log:
                for dirname, cppforge_path in entries:
                    if cppforge_path.startswith(COMMON_NEW_PREFIX):
                        # Pick the one that matches the source directory pattern
                        # This heuristic works for most cases
                        if dirname and dirname in sf:
                            return cppforge_path

            # If still ambiguous and there's a log match, log headers are less common
            # Default: return the first matching common header, or the first entry
            for dirname, cppforge_path in entries:
                if cppforge_path.startswith(COMMON_NEW_PREFIX):
                    return cppforge_path
            return entries[0][1]

    return None


def process_file(filepath, path_mapping, basename_map, basename_includes):
    """Process a single source file, updating #include paths."""
    # Convert to absolute path and compute relative path
    filepath = filepath.absolute() if not filepath.is_absolute() else filepath
    rel_path = os.path.relpath(filepath, PROJECT_ROOT).replace("\\", "/")
    with open(filepath, 'r', encoding='utf-8', errors='surrogateescape') as f:
        content = f.read()

    lines = content.split('\n')
    modified_lines = []
    changes = []

    for lineno, line in enumerate(lines, 1):
        # Match #include "..."
        match = re.match(r'^(.*#include\s+)"([^"]+)"(.*)$', line)
        if match:
            prefix = match.group(1)  # e.g., "#include "
            quoted_path = match.group(2)  # e.g., "buffer/CharBuffer.hpp"
            suffix = match.group(3)  # trailing content

            # Check exclusion patterns
            if should_exclude(line):
                modified_lines.append(line)
                continue

            # Find the cppforge replacement
            cppforge_path = find_cppforge_path(
                quoted_path, rel_path,
                path_mapping, basename_map, basename_includes
            )

            if cppforge_path:
                new_line = f'{prefix}<{cppforge_path}>{suffix}'
                modified_lines.append(new_line)
                changes.append((lineno, quoted_path, f'<{cppforge_path}>'))
                continue

        modified_lines.append(line)

    if changes:
        new_content = '\n'.join(modified_lines)
        with open(filepath, 'w', encoding='utf-8', errors='surrogateescape') as f:
            f.write(new_content)

    return changes


def validate_no_remaining_old_includes(path_mapping, basename_map_ref):
    """Check that no files still reference old include paths."""
    all_source_files = []
    for pattern in SCAN_PATTERNS:
        all_source_files.extend(Path().glob(pattern))

    remaining = []
    for filepath in sorted(set(all_source_files)):
        filepath = filepath.absolute() if not filepath.is_absolute() else filepath
        with open(filepath, 'r', encoding='utf-8', errors='surrogateescape') as f:
            content = f.read()

        for line in content.split('\n'):
            match = re.search(r'#include\s+"([^"]+\.hpp)"', line)
            if match:
                quoted = match.group(1)
                basename = os.path.basename(quoted)
                # Check if this matches any moved header
                if quoted in path_mapping or basename in basename_map_ref:
                    if not should_exclude(line):
                        remaining.append((filepath, line.strip()))

    return remaining


# ─── Main ────────────────────────────────────────────────────────────────────
if __name__ == '__main__':
    print("=" * 60)
    print("Include Path Migration Script")
    print("=" * 60)

    # Step 1: Get moved headers mapping
    print("\n[1/4] Parsing moved headers from git status...")
    path_mapping, basename_map, basename_includes = get_moved_headers()
    print(f"  Found {len(path_mapping)} moved headers")

    # Check for basename collisions
    collisions = {b: e for b, e in basename_map.items() if len(set(v[0] for v in e)) > 1}
    if collisions:
        print(f"  Warning: {len(collisions)} basename(s) appear in multiple directories:")
        for basename, entries in sorted(collisions.items()):
            dirs = set(e[0] for e in entries)
            print(f"    '{basename}' -> {', '.join(sorted(dirs))}")
    else:
        print("  No basename collisions found.")

    # Step 2: Collect source files
    print("\n[2/4] Collecting source files to scan...")
    source_files = collect_source_files()
    print(f"  Found {len(source_files)} source files")

    # Step 3: Process files
    print("\n[3/4] Processing files...")
    total_changes = 0
    changed_files = 0
    skipped_files = 0

    for filepath in source_files:
        changes = process_file(filepath, path_mapping, basename_map, basename_includes)
        if changes:
            changed_files += 1
            total_changes += len(changes)
            rel_path = os.path.relpath(filepath, PROJECT_ROOT).replace("\\", "/")
            print(f"  {rel_path}: {len(changes)} change(s)")
            for lineno, old, new in changes:
                print(f"    L{lineno}: \"{old}\" -> {new}")
        else:
            skipped_files += 1

    print(f"\n  Summary: {changed_files} files modified, {total_changes} replacements, {skipped_files} files unchanged")

    # Step 4: Validate
    print("\n[4/4] Validating...")
    remaining = validate_no_remaining_old_includes(path_mapping, basename_map)
    if remaining:
        print(f"  WARNING: {len(remaining)} potential old-style includes remain:")
        for filepath, line in remaining[:20]:
            rel = os.path.relpath(filepath, PROJECT_ROOT).replace('\\', '/')
            print(f"    {rel}: {line}")
        if len(remaining) > 20:
            print(f"    ... and {len(remaining) - 20} more")
    else:
        print("  No remaining old-style includes found.")

    print("\n" + "=" * 60)
    print(f"Migration complete: {total_changes} replacements in {changed_files} files")
    print("=" * 60)
