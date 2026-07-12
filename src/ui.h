#ifndef SSH_LAUNCHER_UI_H
#define SSH_LAUNCHER_UI_H

#include <ncurses.h>

/* Color pair IDs — semantic roles, colors set by active theme.
 * Both panels share the same background (COLOR_TEXT).
 * The active panel stands out via its title/border (COLOR_PANEL_ACTIVE),
 * while the inactive panel is subdued (COLOR_PANEL_INACTIVE). */
enum {
    COLOR_HEADER        = 1,  /* title bar at the top */
    COLOR_PANEL_ACTIVE  = 2,  /* active panel: border + title */
    COLOR_PANEL_INACTIVE= 3,  /* inactive panel: border + title (subdued) */
    COLOR_TEXT          = 4,  /* host entries in both panels */
    COLOR_SELECTED      = 5,  /* highlighted host entry */
    COLOR_STATUS        = 6,  /* status bar at the bottom */
};

/* Number of available themes */
#define THEME_COUNT 6

/* Panel window handles */
typedef struct {
    WINDOW* header_win;       /* top row: title bar */
    WINDOW* recents_panel;    /* left panel: recent hosts */
    WINDOW* all_panel;        /* right panel: all hosts */
    WINDOW* status_win;       /* bottom 3 rows: search + shortcuts */
} UIPanels;

/* Forward declaration */
struct AppState;

/* Initialize ncurses: initscr, cbreak, noecho, keypad, colors, create windows.
 * Returns 0 on success, -1 on failure (no color support). */
int ui_init(UIPanels* panels);

/* Suspend ncurses before launching SSH. Calls endwin(). */
void ui_suspend(void);

/* Resume ncurses after SSH exits. Calls refresh(). */
void ui_resume(void);

/* Shutdown: delete all windows, call endwin(). */
void ui_shutdown(UIPanels* panels);

/* Full redraw of all panels based on current app state. */
void ui_draw(const UIPanels* panels, const struct AppState* state);

/* Get keyboard input. Returns the key code (or character). */
int ui_get_input(void);

/* Apply a theme by re-initializing all color pairs. */
void ui_apply_theme(int theme_index);

/* Return the next theme index (wraps around). */
int ui_next_theme(int current);

/* Save theme index to persistent config file (~/.config/ssh-launcher/theme).
 * Returns 0 on success, -1 on failure. */
int ui_theme_save(int theme_index);

/* Load theme index from persistent config file.
 * Returns the saved index, or 0 (default) if file doesn't exist. */
int ui_theme_load(void);

#endif /* SSH_LAUNCHER_UI_H */
