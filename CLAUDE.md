# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build

```bash
make                  # build: C11, -Wall -Wextra -pedantic, zero warnings
make clean && make    # fresh rebuild
./ssh-launcher        # run (needs ~/.ssh/config and terminal with colors)
```

## Architecture

Six independent C modules in `src/`, each with a `.h` interface and `.c` implementation:

- **`app`** — Application controller. Owns `AppState` struct (the central state object shared by everything), the event loop, keyboard dispatch, and the SSH `fork()`+`execlp()`+`waitpid()` flow. Does NOT own ncurses drawing.
- **`ui`** — All ncurses rendering. Creates 4 independent `newwin` windows (header, recents panel, all panel, status bar). Handles theme definitions and `ui_apply_theme()`/`ui_next_theme()`. Input via `wgetch(stdscr)` with `keypad(stdscr, TRUE)`.
- **`parser`** — Reads `~/.ssh/config`. Skips wildcard hosts (`*`, `?`). Owns the `HostList` dynamic array. Uses `fgets` + `strtok_r`.
- **`history`** — Plain-text persistence at `~/.config/ssh-launcher/history`. Max 5 entries, newest-first, deduplication on save. Uses `mkdir` to ensure the config directory exists.
- **`search`** — Character-skip fuzzy matching (`fuzzy_match`) + `HostList` filtering (`filter_hosts`). Case-insensitive. Empty pattern matches all.

### Memory ownership

| Allocation | Owner | Freed by |
|---|---|---|
| `all_hosts` strings | `app_init` | `app_shutdown` → `host_list_free` |
| `filtered_hosts->names` (shallow) | `update_filter` | same fn on next filter change, or `app_shutdown` |
| History entries | Stack in `AppState` | auto (struct member array) |
| ncurses windows | `ui_init` | `ui_shutdown` → `delwin` × 4 |

`filtered_hosts` borrows string pointers from `all_hosts` (shallow copy). When freeing a filtered list, only the pointer array is freed — never the strings themselves. The string owner is always `all_hosts`.

### Input dispatch

`app_handle_input` has two branches: **search mode** (`is_searching == true`) and **normal mode**. Arrow keys, Enter, and Tab work in both. Printable ASCII only reaches the search buffer in search mode. `Esc` cancels search in search mode, exits the app in normal mode. `F2` cycles themes in both modes.

`wgetch(stdscr)` requires `keypad(stdscr, TRUE)` (set in `ui_init`) so arrow keys and function keys are translated to `KEY_UP`, `KEY_DOWN`, `KEY_F(2)`, etc. Without this, `wgetch` returns raw escape sequences, causing `Esc` to fire on every arrow press.

### Theme system

6 color pairs (`COLOR_DEFAULT` through `COLOR_SEARCH_HIGHLIGHT`) are semantic roles, not literal colors. `ui_apply_theme(index)` calls `init_pair` on all 6 roles with values from a `Theme` struct array. This means themes can be changed at runtime without re-creating ncurses. The selected theme persists to `~/.config/ssh-launcher/theme`.

### SSH connection flow

```
endwin() → fork() → child: execlp("ssh", "ssh", host, NULL) → _exit
                   → parent: waitpid() → refresh()
```

`endwin()` restores terminal to cooked mode so `ssh` can interact with the user directly (password prompts, etc.). `refresh()` returns ncurses to control after SSH exits. Signal handlers ensure `endwin()` is always called even on SIGINT/SIGTERM, plus an `atexit` safety net.

### Window layout

4 independent `newwin` windows (NOT `derwin` — that caused parent-child buffer interference and blank initial frames):

```
Row 0:         header_win    (1 row, full width)
Rows 1..L-4:   recents_panel (left half), all_panel (right half)
Rows L-3..L-1: status_win    (3 rows: border + search + shortcuts)
```

Each `ui_draw` call does `werase` + draw + `wnoutrefresh` on all 4 windows, then a single `doupdate()`.
