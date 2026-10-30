#!/usr/bin/env bash
set -euo pipefail
if [[ "${MSYSTEM:-}" != 'UCRT64' ]]; then
  echo 'Run in MSYS2 UCRT64.' >&2; exit 1
fi
cd "$(dirname "$0")"
./build-windows.sh
mkdir -p dist/sqldrivers
cp build/QtSqliteWindows.exe dist/
# Copy Qt6/MinGW runtime DLLs and platform plugins, following runtime dependencies.
windeployqt6 --release --compiler-runtime --dir dist dist/QtSqliteWindows.exe
# The SQLite SQL driver is loaded dynamically, not always detected by deployment tools.
cp /ucrt64/share/qt6/plugins/sqldrivers/qsqlite.dll dist/sqldrivers/ 2>/dev/null || \
cp /ucrt64/plugins/sqldrivers/qsqlite.dll dist/sqldrivers/
echo 'Deployment directory: dist/'
