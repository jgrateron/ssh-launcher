#ifndef SSH_LAUNCHER_APP_H
#define SSH_LAUNCHER_APP_H

#include <stdbool.h>
#include "parser.h"
#include "history.h"
#include "ui.h"

typedef enum {
    PANEL_RECENTS,
    PANEL_ALL
} PanelFocus;

/* Complete application state */
typedef struct AppState {
    UIPanels   panels;
    PanelFocus active_panel;
    bool       running;
    bool       is_searching;

    /* Selection indices */
    int        recents_selected;   /* 0 to history_count-1, or -1 if empty */
    int        all_selected;       /* 0 to filtered_count-1, or -1 if empty */

    /* Scroll offsets for the two list panels */
    int        recents_scroll;
    int        all_scroll;

    /* Search state */
    char       search_buffer[HOST_NAME_MAX];
    int        search_cursor;      /* position within search_buffer */

    /* Data */
    HostList*  all_hosts;          /* all hosts from SSH config */
    HostList*  filtered_hosts;     /* currently displayed hosts (filtered or all) */

    char       history[HISTORY_MAX][HOST_NAME_MAX];
    int        history_count;

    /* File paths */
    char       config_path[HOST_NAME_MAX * 2];
    char       history_path[HOST_NAME_MAX * 2];
} AppState;

/* Allocate and initialize the application state. Parses SSH config, loads history.
 * Returns NULL on fatal error. */
AppState* app_init(void);

/* Main event loop. Blocks until the user exits. */
void app_run(AppState* state);

/* Clean up all resources and free state. */
void app_shutdown(AppState* state);

#endif /* SSH_LAUNCHER_APP_H */
