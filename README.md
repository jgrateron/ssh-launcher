# SSH Launcher

A C11 + ncurses TUI for managing SSH connections. Reads `~/.ssh/config`, lets you navigate, filter, and connect to servers from a dual-panel terminal interface.

## Requirements

- GCC (C11) or compatible compiler
- ncurses development library
- Linux / Unix

## Installation

### 1. Install dependencies

**Debian / Ubuntu:**
```bash
sudo apt install build-essential libncurses-dev
```

**Fedora / RHEL:**
```bash
sudo dnf install gcc ncurses-devel
```

**Arch Linux:**
```bash
sudo pacman -S gcc ncurses
```

**macOS (Homebrew):**
```bash
brew install ncurses
```

### 2. Build and install

```bash
make
sudo make install   # installs to /usr/local/bin/
```

## Usage

```bash
./ssh-launcher
```

Make sure you have a `~/.ssh/config` with hosts configured. You can use `sample_ssh_config` as a starting point:

```bash
cp sample_ssh_config ~/.ssh/config
```

## Controls

| Key | Action |
|---|---|
| `↑` `↓` / `j` `k` | Navigate list |
| `Tab` | Switch panel (Recent ↔ All) |
| `Enter` | Connect via SSH to selected host |
| `/` | Search (incremental fuzzy filter) |
| `F2` | Cycle color theme |
| `Esc` | Cancel search / Exit |

## Themes

6 built-in themes, cycle with `F2`:

0. **Default** — cyan, green, and blue on black
1. **Light** — dark text on light backgrounds
2. **Monochrome** — black and white only
3. **Ocean** — deep blues and cyans
4. **Retro** — amber and green, vintage terminal style
5. **Solarized** — solarized dark palette

The selected theme persists across sessions in `~/.config/ssh-launcher/theme`.

## Data Files

| File | Location |
|---|---|
| SSH config | `~/.ssh/config` |
| History (last 5) | `~/.config/ssh-launcher/history` |
| Saved theme | `~/.config/ssh-launcher/theme` |

## Project Structure

```
src/
  main.c          # Entry point
  app.h / app.c   # State, event loop, orchestration
  parser.h / .c   # SSH config parser (~/.ssh/config)
  history.h / .c  # History persistence (last 5 connections)
  search.h / .c   # Fuzzy search (character-skip matching)
  ui.h / ui.c     # ncurses UI (windows, themes, input)
```

## Build

```bash
make clean && make    # C11, -Wall -Wextra -pedantic, zero warnings
```

## Fuzzy Search

Uses **character-skip matching** (fzf-style): all characters in the pattern must appear in order within the host name, but not necessarily consecutively.

Example: `prd` matches `prod-web-01` (p, r, d appear in order).
