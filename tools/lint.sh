#!/bin/bash
set -euo pipefail

file="${1:?Usage: $0 <file.cc>}"

echo "=== clang-tidy ==="
clang-tidy -p build -fix "$file"

echo "=== cpplint ==="
cpplint "$file"

echo "=== clang-format ==="
clang-format -i "$file"
