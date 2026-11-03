#!/usr/bin/env bash
set -euo pipefail
# Keep the toolchain on the Codespaces workspace volume, outside the repo.
# Codespace deletion removes it; devcontainer rebuild ordinarily preserves it.
MXE_ROOT="${MXE_ROOT:-/workspaces/mxe-toolchain}"
MXE_TARGET="x86_64-w64-mingw32.static"
JOBS="${JOBS:-2}"
if [[ ! -f "$MXE_ROOT/Makefile" ]]; then
  if [[ -e "$MXE_ROOT" ]]; then
    echo "Path exists but is not MXE: $MXE_ROOT" >&2
    exit 1
  fi
  git clone --depth 1 https://github.com/mxe/mxe.git "$MXE_ROOT"
fi
printf 'MXE: %s\nTarget: %s\nBuild jobs: %s\n' "$MXE_ROOT" "$MXE_TARGET" "$JOBS"
# Qtbase also builds the appropriate host-side Qt tools and SQLite driver.
make -C "$MXE_ROOT" -j"$JOBS" MXE_TARGETS="$MXE_TARGET" qt6-qtbase
printf '\nToolchain ready. Run scripts/build-codespaces.sh\n'
