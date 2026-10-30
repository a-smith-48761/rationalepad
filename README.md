# Windows Qt 6 + SQLite cross-compilation in GitHub Codespaces

This example produces a **Windows x86-64 executable** from a **Linux GitHub Codespace** using MXE, Qt 6 Widgets/Sql, MinGW-w64 and CMake. **No MSYS2 installation on your Windows computer is needed.**

## Create the Codespace

1. Upload these files to the root of a GitHub repository, including the hidden `.devcontainer` directory.
2. In GitHub select **Code → Codespaces → Create codespace on main**.
3. Once the devcontainer is ready, open a Codespaces terminal.

## First-time toolchain installation

```bash
bash scripts/bootstrap-mxe.sh
```

This clones [MXE](https://mxe.cc/) into `/workspaces/mxe-toolchain` (outside the repository), compiles the cross compiler and Qt 6 dependencies, and installs them there. This is **a large, potentially multi-hour build** that needs ample disk and available Codespaces compute quota. Use a larger Codespace/disk allocation if you encounter storage or memory errors. The default is `JOBS=2` to reduce memory pressure; change it e.g. `JOBS=4 bash scripts/bootstrap-mxe.sh` if appropriate.

The toolchain files normally survive a Codespace container rebuild because they are placed under `/workspaces`, but deleting the Codespace removes them. No dependencies are built during the devcontainer image build itself.

## Cross-compile the app

```bash
bash scripts/build-codespaces.sh
```

Outputs:

- `build-codespaces/QtSqliteWindows.exe` — Windows executable
- `dist/QtSqliteWindows.exe` — copy for distribution
- `QtSqliteWindows-win64.zip` — download this ZIP from Codespaces using the file browser

The example creates a per-user SQLite database and displays the launch count. For the static target, Qt/SQLite are intended to be linked into the executable rather than deployed as Qt DLLs. Actual portability and plugin inclusion **must be tested on Windows** before distribution. Some runtime DLLs may still be required depending on the MXE/toolchain configuration.

## Technical notes

- `CMakeLists.txt` uses `find_package(Qt6 COMPONENTS Widgets Sql)`.
- MXE's `qt6-qtbase` includes SQLite as a dependency and builds its SQL driver; its own recipe enables `FEATURE_sqlite` support through the SQLite dependency.
- For static Qt, CMake normally imports required platform and SQL plugins automatically through Qt's plugin targets. If `QSQLITE` is missing at runtime, verify the Qt SQLite plugin is linked/imported and add an explicit `Q_IMPORT_PLUGIN(QSQLiteDriverPlugin)` if necessary.
- Compilation and runtime have **not been verified in an actual Codespace** from this authoring environment. MXE upstream versions can change; if bootstrap fails, check MXE's current package requirements.
- Existing `build-windows.sh`, `build-windows.ps1` and `deploy-windows.sh` belong to the original native MSYS2 workflow; for Codespaces use the `scripts/*-codespaces.sh` scripts instead.

## Why MXE?

Qt 6 needs both *Linux host tools* (such as `moc`) and *Windows target libraries*. MXE handles both parts, plus the cross compiler and dependencies. Compiling Qt from scratch is resource-intensive, but leaves a reusable Windows toolchain in the Codespace.
