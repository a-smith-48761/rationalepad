#!/usr/bin/env bash
set -euo pipefail
if [[ "${MSYSTEM:-}" != 'UCRT64' ]]; then
  echo 'Launch the MSYS2 UCRT64 shell (not MSYS, MINGW64, or CLANG64).' >&2
  exit 1
fi
cd "$(dirname "$0")"
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
printf '\nExecutable: %s/build/QtSqliteWindows.exe\n' "$PWD"
