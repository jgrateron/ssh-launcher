#ifndef SSH_LAUNCHER_UI_H
#define SSH_LAUNCHER_UI_H

#include <ncurses.h>

/* Color pair IDs — semantic roles, colors set by active theme */
enum {
    COLOR_DEFAULT        = 1,  /* body text */
    COLOR_HEADER         = 2,  /* title bar */
    COLOR_RECENT         = 3,  /* recent hosts entries */
    COLOR_SELECTED       = 4,  /* highlighted selection */
    COLOR_STATUS         = 5,  /* status bar */
    COLOR_SEARCH_HIGHLIGHT = 6,  /* search matches */
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

#endif /* SSH_LAUNCHER_UI_H */
