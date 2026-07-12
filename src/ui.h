#ifndef SSH_LAUNCHER_UI_H
#define SSH_LAUNCHER_UI_H

#include <ncurses.h>

/* Color pair IDs */
enum {
    COLOR_DEFAULT        = 1,  /* white on black */
    COLOR_HEADER         = 2,  /* black on cyan (reversed) */
    COLOR_RECENT         = 3,  /* green on black */
    COLOR_SELECTED       = 4,  /* black on white (standout) */
    COLOR_STATUS         = 5,  /* white on blue */
    COLOR_SEARCH_HIGHLIGHT = 6,  /* yellow on black */
};

/* Panel window handles */
typedef struct {
    WINDOW* header_win;       /* top row: title bar */
    WINDOW* recents_panel;    /* left panel: recent hosts */
    WINDOW* all_panel;        /* right panel: all hosts */
    WINDOW* status_win;       /* bottom 2 rows: search + shortcuts */
    WINDOW* root_win;         /* full-screen root window */
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

#endif /* SSH_LAUNCHER_UI_H */
