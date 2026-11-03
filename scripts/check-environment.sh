#!/usr/bin/env bash
set -euo pipefail
printf 'Codespace host: '; uname -srm
printf 'CMake: '; cmake --version | head -1
printf 'Git: '; git --version
printf '\nRun scripts/bootstrap-mxe.sh once to build the Windows Qt 6 toolchain.\n'
