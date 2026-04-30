#!/bin/bash
set -euo pipefail

file="${1:?Usage: $0 <file.cc>}"

echo "=== clang-tidy ==="
clang-tidy -p build -fix "$file" 2>/dev/null  # suppress system header warning summary (stderr); user code diagnostics (stdout) unaffected

echo "=== cpplint ==="
cpplint "$file"

echo "=== clang-format ==="
clang-format -i "$file"
