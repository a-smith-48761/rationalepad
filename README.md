# RationalePad — first Qt Widgets UI skeleton

Windows x64 C++17 / Qt 6 example for the existing MXE GitHub Codespaces toolchain.

## Build

Replace your repository's `src/main.cpp`, `CMakeLists.txt` and `scripts/build-codespaces.sh` with the versions in this archive, or copy the contents into a new repository with your existing `.devcontainer` and bootstrap scripts.

```bash
bash scripts/build-codespaces.sh
```

Output: `RationalePad-win64.zip` containing `RationalePad.exe`.

If you still see the MXE `MySQL::MySQL` import issue, retain the Qt6QMYSQLDriverPluginConfig.cmake workaround applied earlier.

## Features

- Countdown with four persisted most-recently-used duration buttons, custom hours/minutes/seconds dialog, pause/resume/reset and beep at zero.
- Main plain-text editor with optional lightweight Markdown source highlighting (`View` menu).
- Bottom plain-text editor expands as text wraps and grows, up to 40% of available editor area, then scrolls.
- Both editors use Qt built-in multi-level undo/redo.

## Deliberately deferred

- Saving/loading editor contents (SQLite persistence comes next).
- Timer state persistence across app restarts.
- Full CommonMark highlighting, themes, keyboard shortcuts, packaging installer.

Qt SQL and the static SQLite driver are configured for later implementation; this prototype doesn't open a database yet.
