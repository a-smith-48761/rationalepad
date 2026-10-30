# Run from MSYS2 UCRT64 terminal: powershell.exe -ExecutionPolicy Bypass -File build-windows.ps1
# Or run build-windows.sh directly in the MSYS2 UCRT64 terminal.
$ErrorActionPreference = 'Stop'
Write-Host 'Prefer build-windows.sh in MSYS2 UCRT64; this PowerShell wrapper requires cmake, ninja and Qt6 on PATH.'
cmake -S $PSScriptRoot -B "$PSScriptRoot/build" -G Ninja -DCMAKE_BUILD_TYPE=Release
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
cmake --build "$PSScriptRoot/build" --parallel
exit $LASTEXITCODE
