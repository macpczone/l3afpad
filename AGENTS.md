# L3afpad — Agent Guide

L3afpad is a lightweight GTK3 text editor, a fork of [leafpad](http://tarot.freeshell.org/leafpad/) that uses GTK3 instead of GTK2. It is maintained at https://git.budlabs.xyz/bud/l3afpad-bud.

## Project Overview

| Aspect       | Detail                                    |
|--------------|-------------------------------------------|
| Language     | C                                         |
| Toolkit      | GTK+ 3.x                                  |
| Build system | GNU Autotools (autoconf, automake, intltool) |
| License      | GPL-2.0                                   |
| Version      | 1.0.2                                     |
| Maintainer   | Nube <nubesu@tuta.io> (configure.ac: mikeai@bnm.one) |

L3afpad is designed for **simplicity and minimal weight**: few dependencies, fast startup, essential features only.

## Build System

### Prerequisites

- **Runtime:** GTK+ 3.x libraries, ncurses
- **Build:** automake, intltool, autoconf, libtool

### Build Steps

```bash
./autogen.sh          # Generate configure script (runs autoreconf)
./configure           # Run autoconf configure
make                  # Build
make install-strip    # Install (requires root)
```

### Configure Options

| Flag                        | Default | Description                                      |
|-----------------------------|---------|--------------------------------------------------|
| `--disable-print`           | enabled | Disable printing support                         |
| `--disable-statistics`      | enabled | Disable character/line statistics                |
| `--disable-xinput2`         | enabled | Disable XInput2 (multi-device) support           |
| `--enable-emacs`            | disabled| Enable experimental Emacs key theme              |
| `--enable-search-history`   | disabled| Enable search/replace history                    |
| `--enable-debug`            | disabled| Enable debug output                              |

Note: `--disable-chardetect` is also available (chardetect is enabled by default). These map to preprocessor defines in `config.h`: `ENABLE_PRINT`, `ENABLE_STATISTICS`, `ENABLE_XINPUT2`, `ENABLE_EMACS`, `SEARCH_HISTORY`, `ENABLE_DEBUG`, `ENABLE_CHARDETECT`.

These map to preprocessor defines in `config.h`: `ENABLE_PRINT`, `ENABLE_STATISTICS`, `ENABLE_XINPUT2`, `ENABLE_EMACS`, `SEARCH_HISTORY`.

### Source File

`src/Makefile.am` lists all source files. Adding a new module requires:

1. Creating `foo.h` and `foo.c` in `src/`
2. Adding both to `l3afpad_SOURCES` in `src/Makefile.am`
3. Including `foo.h` in the appropriate header (usually `l3afpad.h` or the module that uses it)
4. Running `autoreconf` / `./autogen.sh` to regenerate build files

## Source Layout

```
l3afpad/
├── configure.ac              # Autoconf configuration
├── autogen.sh                # Bootstrap script (autoreconf wrapper)
├── Makefile.am               # Top-level automake file
├── src/
│   ├── l3afpad.h             # Master header — includes all module headers
│   ├── main.c                # Entry point, config load/save, arg parsing
│   ├── window.h / window.c   # Main window creation (GtkUIManager, GtkTextView)
│   ├── menu.h / menu.c       # Menu bar and toolbar definitions
│   ├── callback.h / callback.c # Signal callbacks for menu items
│   ├── view.h / view.c       # Text view helpers (scroll, cursor)
│   ├── undo.h / undo.c       # Unlimited undo/redo system
│   ├── font.h / font.c       # Font selection and rendering
│   ├── linenum.h / linenum.c # Line number gutter (Cairo drawing)
│   ├── indent.h / indent.c   # Auto-indent, tab width, multi-line indent
│   ├── hlight.h / hlight.c   # Syntax highlighting
│   ├── selector.h / selector.c # Code selector (font chooser dialog)
│   ├── file.h / file.c       # File I/O (open, save, save-as, recent)
│   ├── encoding.h / encoding.c # Character encoding detection/selection
│   ├── search.h / search.c   # Find / Find-Next / Replace dialogs
│   ├── dialog.h / dialog.c   # Generic dialog helpers
│   ├── dnd.h / dnd.c         # Drag-and-drop support
│   ├── utils.h / utils.c     # Utility functions
│   ├── emacs.h / emacs.c     # Emacs key theme (optional)
│   ├── recent.h / recent.c   # Recent files list
│   ├── gtkprint.h / gtkprint.c # Printing (optional)
│   ├── gtksourceiter.h / gtksourceiter.c # GtkTextIter helpers
│   └── autosave.h / autosave.c # Auto-save feature (timer + change count)
├── data/
│   ├── l3afpad.desktop.in    # Desktop entry template
│   ├── l3afpad.png           # Application icon
│   ├── l3afpad.xpm           # XPM icon (fallback)
│   ├── l3afpad_original.svg  # SVG source icon
│   └── icons/                # PNG icons in 16x16 … 256x256 sizes
│   └── icons_another/        # Alternative icon set (16 … 48)
├── po/                       # i18n translations (gettext)
│   ├── LINGUAS               # List of supported locales
│   ├── POTFILES.in           # Source files scanned for translatable strings
│   └── *.po                  # Per-language translation files
└── l3afpad.1                 # Man page
```

## Architecture

### Core Data Model

The application uses a single **`PublicData`** struct (defined in `l3afpad.h`):

```c
typedef struct {
    FileInfo *fi;
    MainWin  *mw;
} PublicData;
```

- **`FileInfo`** — tracks the current file: filename, charset, line ending mode, encoding state.
- **`MainWin`** — holds the top-level GTK widgets: `window`, `menubar` (GtkUIManager), `view` (GtkTextView), `buffer` (GtkTextBuffer).

This struct is a global (`pub`) accessible from all source files.

### Key Design Points

- **Single-buffer, single-window**: No tabs, no multiple documents. Opening a file replaces the current buffer.
- **Unlimited undo/redo**: Implemented in `undo.c` via a custom stack on the GtkTextBuffer.
- **Auto-save**: Two triggers — a 10-second timer (configurable) after last change, or after 150 character changes (configurable). Saves to `~/.cache/l3afpad/buffers/` or alongside the file. Auto-save file is deleted once the file is manually saved.
- **Config persistence**: Settings are written to `~/.config/l3afpad/l3afpadrc` on exit (window size, font, wrap, line numbers, indent, tab width, auto-save settings).
- **Encoding**: Supports OpenI18N-registered codesets with auto-detection for UTF-8.

### GTK UI Structure

- `GtkUIManager` builds the menu from XML in `menu.c`
- `GtkTextView` with a `GtkTextBuffer` is the central editing widget
- Line numbers are drawn in a `GtkEventBox` to the left of the view using Cairo
- Menus use `GtkToggleAction` for options (Word Wrap, Line Numbers, Auto-Indent, Auto-Save, etc.)

## Coding Conventions

- **License header**: Every `.c` and `.h` file starts with the GPL-2.0 header block.
- **Include guard**: `#ifndef _MODULE_H` / `#define _MODULE_H` / `#endif`
- **Master header**: `l3afpad.h` includes `config.h` first (via `HAVE_CONFIG_H`), then GTK/GLib, then all module headers.
- **Naming**: Functions use `module_action()` style (e.g., `file_open_real()`, `autosave_set_timer()`).
- **Memory**: Uses GLib allocation (`g_malloc`, `g_strdup`, `g_free`) consistently.
- **i18n**: All user-facing strings wrapped in `_()` (gettext). New translatable strings must also be added to `po/POTFILES.in` if in a new file.
- **Conditional compilation**: Features guarded by `#if ENABLE_PRINT`, `#if ENABLE_STATISTICS`, etc.

## Key Features & Their Source Files

| Feature                | Source Files                          |
|------------------------|---------------------------------------|
| File open/save/recent  | `file.c`, `recent.c`                  |
| Search & replace       | `search.c`                            |
| Line numbers           | `linenum.c`                           |
| Auto-indent            | `indent.c`                            |
| Syntax highlighting    | `hlight.c`                            |
| Unlimited undo/redo    | `undo.c`                              |
| Encoding selection     | `encoding.c`                          |
| Drag & drop            | `dnd.c`                               |
| Printing               | `gtkprint.c`                          |
| Auto-save              | `autosave.c`                          |
| Emacs key bindings     | `emacs.c`                             |
| Font selection         | `font.c`, `selector.c`                |
| Tab width toggle       | `indent.c` (Ctrl-Tab)                 |

## i18n (Internationalization)

- Translations live in `po/`.
- Supported languages: ar, bg, br, ca, cs, da, de, el, eo, es, et, eu, fi, fr, ga, gl, he, hu, id, it, ja, ko, lt, lv, nl, nn, pl, pt, pt_BR, ru, sk, sl, sr, sv, ta, tr, uk, vi, zh_CN, zh_TW.
- To add a new translation:
  1. Add the locale code to `po/LINGUAS`
  2. Create `po/<locale>.po` from the POT template
  3. Run `make` to generate `po/<locale>.gmo`
- Translatable strings use `_()` macro. New `.c` files must be listed in `po/POTFILES.in`.

## Testing & Running

```bash
# Build
./autogen.sh && ./configure && make

# Run from source tree
src/l3afpad

# Run with specific options
src/l3afpad --help
src/l3afpad --version
src/l3afpad --codeset=UTF-8 --tab-width=4 --jump=42 myfile.txt
```

## Common Tasks

### Adding a new menu item

1. Add the XML entry in `menu.c`'s `ui_info` string.
2. Add a callback function in `callback.c` and connect it in `window.c` or `menu.c`.
3. If it's a toggle action, use `gtk_toggle_action_get_active()` to read state.

### Adding a new configure option

1. Add `AC_ARG_ENABLE(...)` block in `configure.ac`.
2. Add `AC_DEFINE(...)` to set the preprocessor macro.
3. Use `#if ENABLE_FEATURE` in C source.
4. Run `autoreconf` to regenerate `configure`.

### Adding a new source file

1. Create `src/foo.h` and `src/foo.c`.
2. Add both to `l3afpad_SOURCES` in `src/Makefile.am`.
3. Include `foo.h` from `l3afpad.h` or the appropriate module.
4. If it contains translatable strings, add `src/foo.c` to `po/POTFILES.in`.
5. Run `./autogen.sh` to regenerate.

### Modifying the auto-save behavior

- Timer interval: `autosave_set_timer()` / `autosave_get_timer()` (default 10000ms)
- Immediate change threshold: `autosave_set_immediate_changes()` (default 150)
- Same-dir toggle: `autosave_set_same_dir()`
- State toggle: `autosave_set_state()`

All persisted in `~/.config/l3afpad/l3afpadrc`.

## Git Workflow

- The `.gitignore` excludes build artifacts, autom4te cache, generated Makefiles, `.o` files, and `.gmo` files.
- Always commit only source files (`.c`, `.h`, `.am`, `.ac`, `.in`, `.po`, data files) — not generated build output.
- After modifying `configure.ac` or `Makefile.am`, run `./autogen.sh` and commit the regenerated `configure` and `Makefile.in` files as well.

## Known Dependencies

| Dependency   | Used For                        |
|--------------|---------------------------------|
| GTK+ 3.0+    | UI toolkit (GtkTextView, etc.)  |
| GLib         | Memory, strings, file I/O       |
| Pango        | Text rendering, fonts           |
| Cairo        | Line number rendering           |
| ncurses      | Terminal utilities (gedit_utils)|
| gettext      | Internationalization            |
| intltool     | Desktop file translation        |

## Useful References

- [Leafpad (ancestor)](http://tarot.freeshell.org/leafpad/)
- [GTK3 GtkTextView docs](https://docs.gtk.org/gtk3/class.TextView.html)
- [GtkUIManager docs](https://docs.gtk.org/gtk3/class.UIManager.html)
- [Auto-save design](README#auto-save)
