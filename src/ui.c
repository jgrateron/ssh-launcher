#include "ui.h"
#include "app.h"
#include <stdlib.h>
#include <string.h>

int ui_init(UIPanels* panels) {
    if (!panels) return -1;

    initscr();
    if (!has_colors()) {
        endwin();
        return -1;
    }

    start_color();
    cbreak();
    noecho();
    curs_set(0);
    keypad(stdscr, TRUE);  /* enable function keys on stdscr for input */

    /* Initialize color pairs */
    init_pair(COLOR_DEFAULT,   COLOR_WHITE,  COLOR_BLACK);
    init_pair(COLOR_HEADER,    COLOR_BLACK,  COLOR_CYAN);
    init_pair(COLOR_RECENT,    COLOR_GREEN,  COLOR_BLACK);
    init_pair(COLOR_SELECTED,  COLOR_BLACK,  COLOR_WHITE);
    init_pair(COLOR_STATUS,    COLOR_WHITE,  COLOR_BLUE);
    init_pair(COLOR_SEARCH_HIGHLIGHT, COLOR_YELLOW, COLOR_BLACK);

    /* Create independent windows (non-overlapping screen regions).
     * Layout: header (1) + panels (LINES-4) + status (3) = LINES */
    panels->header_win    = newwin(1,           COLS,            0, 0);
    panels->recents_panel = newwin(LINES - 4,   COLS / 2,        1, 0);
    panels->all_panel     = newwin(LINES - 4,   COLS - COLS / 2, 1, COLS / 2);
    panels->status_win    = newwin(3,           COLS,            LINES - 3, 0);

    if (!panels->header_win || !panels->recents_panel ||
        !panels->all_panel || !panels->status_win) {
        ui_shutdown(panels);
        return -1;
    }

    keypad(panels->header_win, TRUE);
    keypad(panels->recents_panel, TRUE);
    keypad(panels->all_panel, TRUE);
    keypad(panels->status_win, TRUE);

    /* Initial screen clear */
    werase(stdscr);
    wnoutrefresh(stdscr);
    doupdate();

    return 0;
}

void ui_suspend(void) {
    endwin();
}

void ui_resume(void) {
    refresh();
}

void ui_shutdown(UIPanels* panels) {
    if (!panels) return;

    if (panels->header_win)    delwin(panels->header_win);
    if (panels->recents_panel) delwin(panels->recents_panel);
    if (panels->all_panel)     delwin(panels->all_panel);
    if (panels->status_win)    delwin(panels->status_win);

    panels->header_win    = NULL;
    panels->recents_panel = NULL;
    panels->all_panel     = NULL;
    panels->status_win    = NULL;

    endwin();
}

/* Draw a box with title at the top border */
static void draw_titled_box(WINDOW* win, const char* title, int color_pair) {
    int height, width;
    getmaxyx(win, height, width);
    (void)height;

    werase(win);
    wattron(win, COLOR_PAIR(color_pair));
    wborder(win, 0, 0, 0, 0, 0, 0, 0, 0);

    if (title && width > 2) {
        int title_len = (int)strlen(title);
        int x = 1;
        if (title_len > width - 2) title_len = width - 2;
        mvwprintw(win, 0, x, "%.*s", title_len, title);
    }
    wattroff(win, COLOR_PAIR(color_pair));
}

/* Draw host entries from history array */
static void draw_host_list(WINDOW* win, char entries[][HOST_NAME_MAX], int count,
                           int selected, int scroll_offset, int color_pair) {
    int height, width;
    getmaxyx(win, height, width);
    int visible_rows = height - 2;
    if (visible_rows < 0) return;

    for (int i = 0; i < visible_rows && (i + scroll_offset) < count; i++) {
        int idx = i + scroll_offset;
        int y = i + 1;
        int x = 1;

        if (idx == selected) {
            wattron(win, COLOR_PAIR(COLOR_SELECTED));
        } else {
            wattron(win, COLOR_PAIR(color_pair));
        }

        const char* marker = (idx == selected) ? "> " : "  ";
        int max_len = width - 3;
        mvwprintw(win, y, x, "%s%-*.*s", marker, max_len, max_len, entries[idx]);

        wattroff(win, COLOR_PAIR(COLOR_SELECTED));
        wattroff(win, COLOR_PAIR(color_pair));
    }
}

/* Draw host entries from a HostList */
static void draw_hostlist(WINDOW* win, const HostList* list,
                          int selected, int scroll_offset, int color_pair) {
    int height, width;
    getmaxyx(win, height, width);
    int visible_rows = height - 2;
    if (visible_rows < 0 || !list) return;

    for (int i = 0; i < visible_rows && (i + scroll_offset) < list->count; i++) {
        int idx = i + scroll_offset;
        int y = i + 1;
        int x = 1;

        if (idx == selected) {
            wattron(win, COLOR_PAIR(COLOR_SELECTED));
        } else {
            wattron(win, COLOR_PAIR(color_pair));
        }

        const char* marker = (idx == selected) ? "> " : "  ";
        int max_len = width - 3;
        mvwprintw(win, y, x, "%s%-*.*s", marker, max_len, max_len, list->names[idx]);

        wattroff(win, COLOR_PAIR(COLOR_SELECTED));
        wattroff(win, COLOR_PAIR(color_pair));
    }
}

void ui_draw(const UIPanels* panels, const struct AppState* state) {
    if (!panels || !state) return;

    /* Check terminal size */
    if (LINES < 10 || COLS < 40) {
        clear();
        const char* msg = "Terminal too small. Need at least 40x10.";
        mvprintw(LINES / 2, (COLS - (int)strlen(msg)) / 2, "%s", msg);
        mvprintw(LINES / 2 + 1, (COLS - 22) / 2, "Press any key to exit.");
        refresh();
        return;
    }

    /* ---- Header ---- */
    draw_titled_box(panels->header_win, "SSH LAUNCHER v1.0", COLOR_HEADER);
    mvwprintw(panels->header_win, 0, COLS - 4, "[?]");

    /* ---- Recents Panel ---- */
    draw_titled_box(panels->recents_panel, " RECENTES ", COLOR_RECENT);

    if (state->history_count == 0) {
        int h, w;
        getmaxyx(panels->recents_panel, h, w);
        wattron(panels->recents_panel, COLOR_PAIR(COLOR_RECENT));
        mvwprintw(panels->recents_panel, h / 2, (w - 7) / 2, "(vacio)");
        wattroff(panels->recents_panel, COLOR_PAIR(COLOR_RECENT));
    } else {
        draw_host_list(panels->recents_panel, (char(*)[HOST_NAME_MAX])state->history,
                       state->history_count, state->recents_selected,
                       state->recents_scroll, COLOR_RECENT);
    }

    /* ---- All Hosts Panel ---- */
    char all_title[320];
    if (state->is_searching && state->search_buffer[0] != '\0') {
        snprintf(all_title, sizeof(all_title), " BUSQUEDA: %s ",
                 state->search_buffer);
    } else {
        snprintf(all_title, sizeof(all_title), " TODOS LOS SERVIDORES ");
    }
    draw_titled_box(panels->all_panel, all_title, COLOR_SELECTED);

    int host_count = state->filtered_hosts ? state->filtered_hosts->count : 0;
    if (host_count == 0) {
        int h, w;
        getmaxyx(panels->all_panel, h, w);
        wattron(panels->all_panel, COLOR_PAIR(COLOR_DEFAULT));
        const char* empty_msg;
        if (state->all_hosts && state->all_hosts->count == 0) {
            empty_msg = "No hosts in ~/.ssh/config";
        } else if (state->is_searching) {
            empty_msg = "No matches";
        } else {
            empty_msg = "No hosts found";
        }
        mvwprintw(panels->all_panel, h / 2, (w - (int)strlen(empty_msg)) / 2,
                  "%s", empty_msg);
        wattroff(panels->all_panel, COLOR_PAIR(COLOR_DEFAULT));
    } else {
        draw_hostlist(panels->all_panel, state->filtered_hosts,
                      state->all_selected, state->all_scroll, COLOR_DEFAULT);
    }

    /* Highlight active panel border */
    if (state->active_panel == PANEL_RECENTS) {
        wattron(panels->recents_panel, COLOR_PAIR(COLOR_SELECTED));
        wborder(panels->recents_panel, 0, 0, 0, 0, 0, 0, 0, 0);
        wattroff(panels->recents_panel, COLOR_PAIR(COLOR_SELECTED));
    } else {
        wattron(panels->all_panel, COLOR_PAIR(COLOR_SELECTED));
        wborder(panels->all_panel, 0, 0, 0, 0, 0, 0, 0, 0);
        wattroff(panels->all_panel, COLOR_PAIR(COLOR_SELECTED));
    }

    /* ---- Status Bar ---- */
    draw_titled_box(panels->status_win, NULL, COLOR_STATUS);

    /* Row 1 of status: search input */
    wattron(panels->status_win, COLOR_PAIR(COLOR_STATUS));
    mvwprintw(panels->status_win, 1, 2, "Busqueda: %s",
              state->search_buffer);
    if (state->is_searching) {
        wattron(panels->status_win, A_BLINK);
        int cursor_x = 12 + state->search_cursor;
        mvwprintw(panels->status_win, 1, cursor_x, " ");
        wattroff(panels->status_win, A_BLINK);
    }
    wattroff(panels->status_win, COLOR_PAIR(COLOR_STATUS));

    /* Row 2 of status: shortcuts */
    wattron(panels->status_win, COLOR_PAIR(COLOR_STATUS) | A_BOLD);
    mvwprintw(panels->status_win, 2, 2,
              "[Enter] Conectar  [/] Buscar  [Tab] Panel  [Esc] %s",
              state->is_searching ? "Cancelar" : "Salir");
    wattroff(panels->status_win, COLOR_PAIR(COLOR_STATUS) | A_BOLD);

    /* Refresh all windows — independent newwin's, order doesn't matter */
    wnoutrefresh(panels->header_win);
    wnoutrefresh(panels->recents_panel);
    wnoutrefresh(panels->all_panel);
    wnoutrefresh(panels->status_win);
    doupdate();
}

int ui_get_input(void) {
    return wgetch(stdscr);
}
