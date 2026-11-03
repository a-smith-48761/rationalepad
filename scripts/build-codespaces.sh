#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
MXE_ROOT="${MXE_ROOT:-/workspaces/mxe-toolchain}"
TARGET=x86_64-w64-mingw32.static
WRAPPER="$MXE_ROOT/usr/bin/$TARGET-cmake"
EXEFILE=RationalePad

if [[ ! -x "$WRAPPER" ]]; then
  echo 'MXE CMake wrapper missing. Run scripts/bootstrap-mxe.sh first.' >&2
  exit 1
fi

export PATH="$MXE_ROOT/usr/bin:$PATH"
"$WRAPPER" -S . -B build-codespaces -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build-codespaces --parallel "${JOBS:-2}"
file build-codespaces/${EXEFILE}.exe
mkdir -p dist
cp build-codespaces/${EXEFILE}.exe dist/
(cd dist && zip -q -9 ../${EXEFILE}-win64.zip ${EXEFILE}.exe)
printf "\nExecutable: %s/dist/${EXEFILE}.exe\nZIP: %s/${EXEFILE}-win64.zip\n" "$PWD" "$PWD"
