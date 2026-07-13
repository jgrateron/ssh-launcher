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

### Keyboard

| Key | Action |
|---|---|
| `↑` `↓` / `j` `k` | Navigate list |
| `Tab` | Switch panel (Recents ↔ All) |
| `Enter` | Connect via SSH to selected host |
| `/` | Search (incremental fuzzy filter) |
| `F2` | Cycle color theme |
| `Esc` / `q` | Cancel search / Exit |

### Mouse

| Action | Behavior |
|---|---|
| Click | Activate panel + select host at clicked row |
| Double-click | Select host + connect via SSH immediately |
| Scroll wheel | Navigate up/down in active panel |

## Language

Language is auto-detected from the `$LANG` environment variable. Spanish (`es_*`) and English (everything else) are supported. No manual toggle needed.

## Themes

6 built-in themes, cycle with `F2`. Both panels share the same background; the active panel's title stands out, the inactive panel is subdued but still visible.

0. **Default** — cyan header, green active on black
1. **Light** — dark text on white, blue accents
2. **Monochrome** — black and white only
3. **Nord** — cool bluish palette
4. **Gruvbox** — warm retro tones (yellow/green)
5. **Solarized** — teal/green solarized dark palette

The selected theme persists across sessions in `~/.config/ssh-launcher/theme`.

## Data Files

| File | Location |
|---|---|
| SSH config | `~/.ssh/config` |
| History (last 10) | `~/.config/ssh-launcher/history` |
| Saved theme | `~/.config/ssh-launcher/theme` |

## Project Structure

```
src/
  main.c          # Entry point
  app.h / app.c   # State, event loop, SSH launch, mouse/input handling
  parser.h / .c   # SSH config parser (~/.ssh/config)
  history.h / .c  # History persistence (last 10 connections)
  search.h / .c   # Fuzzy search (character-skip matching)
  ui.h / ui.c     # ncurses UI (windows, themes, drawing, resize)
  i18n.h / i18n.c # Language auto-detection ($LANG, ES/EN)
```

## Build

```bash
make clean && make    # C11, -Wall -Wextra -pedantic, zero warnings
```

## Fuzzy Search

Uses **character-skip matching** (fzf-style): all characters in the pattern must appear in order within the host name, but not necessarily consecutively.

Example: `prd` matches `prod-web-01` (p, r, d appear in order).

## Acknowledgments

This project was developed with assistance from [Claude Code](https://claude.com/claude-code), Anthropic's AI-powered software development tool.

🤖 Generated with [Claude Code](https://claude.com/claude-code)
