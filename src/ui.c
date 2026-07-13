#include "ui.h"
#include "app.h"
#include "i18n.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

/* ---- Theme definitions ---- */
typedef struct {
    short fg_header,    bg_header;     /* title bar */
    short fg_active,    bg_active;     /* active panel: border + title */
    short fg_inactive,  bg_inactive;   /* inactive panel: border + title (subdued) */
    short fg_text,      bg_text;       /* host entries in both panels */
    short fg_select,    bg_select;     /* highlighted host entry */
    short fg_status,    bg_status;     /* status bar */
    const char* name;
} Theme;

static const Theme themes[THEME_COUNT] = {
    /* 0: Default — dark background, cyan header, green active */
    { COLOR_BLACK,  COLOR_CYAN,     /* header: black on cyan */
      COLOR_GREEN,  COLOR_BLACK,    /* active panel: green on black */
      COLOR_BLUE,   COLOR_BLACK,    /* inactive panel: dark blue on black (border visible, subdued) */
      COLOR_WHITE,  COLOR_BLACK,    /* text: white on black */
      COLOR_BLACK,  COLOR_GREEN,    /* selected: black on green */
      COLOR_WHITE,  COLOR_BLUE,     /* status: white on blue */
      "Default" },

    /* 1: Light — light background, dark accents */
    { COLOR_WHITE,  COLOR_BLACK,    /* header: white on black */
      COLOR_BLUE,   COLOR_WHITE,    /* active panel: blue on white */
      COLOR_BLACK,  COLOR_WHITE,    /* inactive panel: black on white (dim) */
      COLOR_BLACK,  COLOR_WHITE,    /* text: black on white */
      COLOR_WHITE,  COLOR_BLUE,     /* selected: white on blue */
      COLOR_BLACK,  COLOR_WHITE,    /* status: black on white */
      "Light" },

    /* 2: Monochrome — black & white only */
    { COLOR_WHITE,  COLOR_BLACK,    /* header: white on black */
      COLOR_BLACK,  COLOR_WHITE,    /* active: reverse video */
      COLOR_WHITE,  COLOR_BLACK,    /* inactive: dim */
      COLOR_WHITE,  COLOR_BLACK,    /* text: white on black */
      COLOR_BLACK,  COLOR_WHITE,    /* selected: reverse video */
      COLOR_WHITE,  COLOR_BLACK,    /* status */
      "Monochrome" },

    /* 3: Nord — cool bluish palette */
    { COLOR_WHITE,  COLOR_BLUE,     /* header */
      COLOR_CYAN,   COLOR_BLACK,    /* active: cyan on black */
      COLOR_BLUE,   COLOR_BLACK,    /* inactive: dark blue on black */
      COLOR_WHITE,  COLOR_BLACK,    /* text */
      COLOR_BLACK,  COLOR_CYAN,     /* selected */
      COLOR_CYAN,   COLOR_BLUE,     /* status */
      "Nord" },

    /* 4: Gruvbox — warm retro tones */
    { COLOR_BLACK,  COLOR_YELLOW,   /* header: black on yellow */
      COLOR_YELLOW, COLOR_BLACK,    /* active: yellow on black */
      COLOR_WHITE,  COLOR_BLACK,    /* inactive: white on black (border visible) */
      COLOR_WHITE,  COLOR_BLACK,    /* text */
      COLOR_BLACK,  COLOR_YELLOW,   /* selected: black on yellow */
      COLOR_YELLOW, COLOR_BLACK,    /* status */
      "Gruvbox" },

    /* 5: Solarized Dark — teal/green solarized palette */
    { COLOR_BLACK,  COLOR_CYAN,     /* header */
      COLOR_GREEN,  COLOR_BLACK,    /* active: green on black */
      COLOR_CYAN,   COLOR_BLACK,    /* inactive: subtle cyan on black */
      COLOR_WHITE,  COLOR_BLACK,    /* text */
      COLOR_BLACK,  COLOR_GREEN,    /* selected */
      COLOR_WHITE,  COLOR_BLUE,     /* status */
      "Solarized" },
};

/* ---- Public API ---- */
void ui_apply_theme(int theme_index) {
    int t = theme_index % THEME_COUNT;
    if (t < 0) t = 0;

    init_pair(COLOR_HEADER,        themes[t].fg_header,   themes[t].bg_header);
    init_pair(COLOR_PANEL_ACTIVE,  themes[t].fg_active,   themes[t].bg_active);
    init_pair(COLOR_PANEL_INACTIVE,themes[t].fg_inactive, themes[t].bg_inactive);
    init_pair(COLOR_TEXT,          themes[t].fg_text,     themes[t].bg_text);
    init_pair(COLOR_SELECTED,      themes[t].fg_select,   themes[t].bg_select);
    init_pair(COLOR_STATUS,        themes[t].fg_status,   themes[t].bg_status);
}

int ui_next_theme(int current) {
    return (current + 1) % THEME_COUNT;
}

/* ---- Theme persistence (~/.config/ssh-launcher/theme) ---- */

static void theme_config_path(char* buf, size_t bufsize) {
    const char* home = getenv("HOME");
    if (!home) home = ".";
    snprintf(buf, bufsize, "%s/.config/ssh-launcher/theme", home);
}

int ui_theme_save(int theme_index) {
    char path[512];
    theme_config_path(path, sizeof(path));

    char dir[512];
    snprintf(dir, sizeof(dir), "%s/.config/ssh-launcher",
             getenv("HOME") ? getenv("HOME") : ".");
    mkdir(dir, 0755);
    snprintf(dir, sizeof(dir), "%s/.config",
             getenv("HOME") ? getenv("HOME") : ".");
    mkdir(dir, 0755);

    FILE* f = fopen(path, "w");
    if (!f) return -1;
    fprintf(f, "%d\n", theme_index % THEME_COUNT);
    fclose(f);
    return 0;
}

int ui_theme_load(void) {
    char path[512];
    theme_config_path(path, sizeof(path));

    FILE* f = fopen(path, "r");
    if (!f) return 0;

    int index = 0;
    if (fscanf(f, "%d", &index) != 1) {
        fclose(f);
        return 0;
    }
    fclose(f);

    if (index < 0) index = 0;
    if (index >= THEME_COUNT) index = 0;
    return index;
}

/* ---- Init / shutdown ---- */
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
    keypad(stdscr, TRUE);

    /* Apply default theme (index 0) */
    ui_apply_theme(0);

    /* Layout: header (1) + panels (LINES-4) + status (3) = LINES */
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

void ui_resize(UIPanels* panels) {
    if (!panels) return;

    /* Resize and reposition each window to match new LINES/COLS.
     * Layout: header (1) + panels (LINES-4) + status (3) = LINES */
    wresize(panels->header_win,    1,           COLS);
    mvwin(panels->header_win,      0,           0);

    wresize(panels->recents_panel, LINES - 4,   COLS / 2);
    mvwin(panels->recents_panel,   1,           0);

    wresize(panels->all_panel,     LINES - 4,   COLS - COLS / 2);
    mvwin(panels->all_panel,       1,           COLS / 2);

    wresize(panels->status_win,    3,           COLS);
    mvwin(panels->status_win,      LINES - 3,   0);

    /* Clear everything so the next ui_draw repaints correctly */
    werase(stdscr);
    wnoutrefresh(stdscr);
    doupdate();
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
                           int selected, int scroll_offset) {
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
            wattron(win, COLOR_PAIR(COLOR_TEXT));
        }

        const char* marker = (idx == selected) ? "> " : "  ";
        int max_len = width - 4;
        mvwprintw(win, y, x, "%s%-*.*s", marker, max_len, max_len, entries[idx]);

        wattroff(win, COLOR_PAIR(COLOR_SELECTED));
        wattroff(win, COLOR_PAIR(COLOR_TEXT));
    }
}

/* Draw host entries from a HostList */
static void draw_hostlist(WINDOW* win, const HostList* list,
                          int selected, int scroll_offset) {
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
            wattron(win, COLOR_PAIR(COLOR_TEXT));
        }

        const char* marker = (idx == selected) ? "> " : "  ";
        int max_len = width - 4;
        mvwprintw(win, y, x, "%s%-*.*s", marker, max_len, max_len, list->names[idx]);

        wattroff(win, COLOR_PAIR(COLOR_SELECTED));
        wattroff(win, COLOR_PAIR(COLOR_TEXT));
    }
}

void ui_draw(const UIPanels* panels, const struct AppState* state) {
    if (!panels || !state) return;

    /* Check terminal size */
    if (LINES < 10 || COLS < 40) {
        clear();
        const char* msg = t(STR_TERMINAL_SMALL);
        mvprintw(LINES / 2, (COLS - (int)strlen(msg)) / 2, "%s", msg);
        const char* key_msg = t(STR_PRESS_ANY_KEY);
        mvprintw(LINES / 2 + 1, (COLS - (int)strlen(key_msg)) / 2, "%s", key_msg);
        refresh();
        return;
    }

    bool recents_active = (state->active_panel == PANEL_RECENTS);

    /* ---- Header ---- */
    draw_titled_box(panels->header_win, "SSH LAUNCHER v1.0", COLOR_HEADER);
    mvwprintw(panels->header_win, 0, COLS - 4, "[?]");

    /* ---- Recents Panel ---- */
    int recents_color = recents_active ? COLOR_PANEL_ACTIVE : COLOR_PANEL_INACTIVE;
    draw_titled_box(panels->recents_panel, t(STR_RECIENTES), recents_color);

    if (state->history_count == 0) {
        int h, w;
        getmaxyx(panels->recents_panel, h, w);
        wattron(panels->recents_panel, COLOR_PAIR(recents_color));
        const char* empty = t(STR_EMPTY);
        mvwprintw(panels->recents_panel, h / 2, (w - (int)strlen(empty)) / 2, "%s", empty);
        wattroff(panels->recents_panel, COLOR_PAIR(recents_color));
    } else {
        draw_host_list(panels->recents_panel, (char(*)[HOST_NAME_MAX])state->history,
                       state->history_count, state->recents_selected,
                       state->recents_scroll);
    }

    /* ---- All Hosts Panel ---- */
    int all_color = recents_active ? COLOR_PANEL_INACTIVE : COLOR_PANEL_ACTIVE;
    char all_title[320];
    if (state->is_searching && state->search_buffer[0] != '\0') {
        snprintf(all_title, sizeof(all_title), t(STR_BUSQUEDA_FMT),
                 state->search_buffer);
    } else {
        snprintf(all_title, sizeof(all_title), "%s", t(STR_ALL_SERVERS));
    }
    draw_titled_box(panels->all_panel, all_title, all_color);

    int host_count = state->filtered_hosts ? state->filtered_hosts->count : 0;
    if (host_count == 0) {
        int h, w;
        getmaxyx(panels->all_panel, h, w);
        wattron(panels->all_panel, COLOR_PAIR(COLOR_TEXT));
        const char* empty_msg;
        if (state->all_hosts && state->all_hosts->count == 0) {
            empty_msg = t(STR_NO_HOSTS_CONFIG);
        } else if (state->is_searching) {
            empty_msg = t(STR_NO_MATCHES);
        } else {
            empty_msg = t(STR_NO_HOSTS_FOUND);
        }
        mvwprintw(panels->all_panel, h / 2, (w - (int)strlen(empty_msg)) / 2,
                  "%s", empty_msg);
        wattroff(panels->all_panel, COLOR_PAIR(COLOR_TEXT));
    } else {
        draw_hostlist(panels->all_panel, state->filtered_hosts,
                      state->all_selected, state->all_scroll);
    }

    /* ---- Status Bar ---- */
    draw_titled_box(panels->status_win, NULL, COLOR_STATUS);

    /* Row 1: search input */
    wattron(panels->status_win, COLOR_PAIR(COLOR_STATUS));
    mvwprintw(panels->status_win, 1, 2, t(STR_SEARCH_LABEL), state->search_buffer);
    if (state->is_searching) {
        wattron(panels->status_win, A_BLINK);
        int cursor_x = 12 + state->search_cursor;
        mvwprintw(panels->status_win, 1, cursor_x, " ");
        wattroff(panels->status_win, A_BLINK);
    }
    wattroff(panels->status_win, COLOR_PAIR(COLOR_STATUS));

    /* Row 2: shortcuts */
    wattron(panels->status_win, COLOR_PAIR(COLOR_STATUS) | A_BOLD);
    mvwprintw(panels->status_win, 2, 2,
              t(STR_SHORTCUTS),
              state->is_searching ? t(STR_CANCEL) : t(STR_EXIT));
    wattroff(panels->status_win, COLOR_PAIR(COLOR_STATUS) | A_BOLD);

    /* Refresh all windows */
    wnoutrefresh(panels->header_win);
    wnoutrefresh(panels->recents_panel);
    wnoutrefresh(panels->all_panel);
    wnoutrefresh(panels->status_win);
    doupdate();
}

int ui_get_input(void) {
    return wgetch(stdscr);
}
