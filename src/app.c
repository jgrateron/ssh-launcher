#include "app.h"
#include "i18n.h"
#include "search.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <errno.h>

/* Global pointer for signal handler */
static AppState* g_app_state = NULL;

/* ---- Signal Handling ---- */
static void handle_signal(int sig) {
    (void)sig;
    if (g_app_state)
        g_app_state->running = false;
}

static void cleanup_atexit(void) {
    if (!isendwin())
        endwin();
}

/* ---- Initialization ---- */
AppState* app_init(void) {
    AppState* state = calloc(1, sizeof(AppState));
    if (!state) return NULL;

    /* Resolve paths */
    const char* home = getenv("HOME");
    if (!home) home = ".";

    snprintf(state->config_path, sizeof(state->config_path),
             "%s/.ssh/config", home);
    history_default_path(state->history_path, sizeof(state->history_path));

    /* Parse SSH config */
    state->all_hosts = host_list_parse(state->config_path);
    if (!state->all_hosts) {
        /* Create empty list on failure */
        state->all_hosts = host_list_create(HOST_LIST_INITIAL_CAPACITY);
        if (!state->all_hosts) {
            free(state);
            return NULL;
        }
    }
    state->filtered_hosts = state->all_hosts;

    /* Load history */
    history_load(state->history_path, state->history, &state->history_count);

    /* Initialize UI state */
    state->active_panel = (state->history_count > 0) ? PANEL_RECENTS : PANEL_ALL;
    state->recents_selected = (state->history_count > 0) ? 0 : -1;
    state->all_selected = (state->all_hosts->count > 0) ? 0 : -1;
    state->recents_scroll = 0;
    state->all_scroll = 0;
    state->is_searching = false;
    state->search_buffer[0] = '\0';
    state->search_cursor = 0;
    state->running = true;

    /* Load saved preferences */
    i18n_load();
    state->theme_index = ui_theme_load();

    /* Initialize ncurses (applies default theme 0 internally) */
    if (ui_init(&state->panels) != 0) {
        host_list_free(state->all_hosts);
        free(state);
        return NULL;
    }

    /* Override with saved theme if different from default */
    if (state->theme_index != 0) {
        ui_apply_theme(state->theme_index);
    }

    /* Set up signal handling */
    g_app_state = state;
    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);
    atexit(cleanup_atexit);

    return state;
}

/* ---- Navigation ---- */
static void adjust_scroll(int selected, int* scroll, int visible_rows, int total) {
    if (selected < *scroll) {
        *scroll = selected;
    } else if (selected >= *scroll + visible_rows) {
        *scroll = selected - visible_rows + 1;
    }
    if (*scroll < 0) *scroll = 0;
    int max_scroll = total - visible_rows;
    if (max_scroll < 0) max_scroll = 0;
    if (*scroll > max_scroll) *scroll = max_scroll;
}

static void navigate_up(AppState* state) {
    if (state->active_panel == PANEL_RECENTS) {
        if (state->history_count > 0) {
            state->recents_selected--;
            if (state->recents_selected < 0) state->recents_selected = 0;
            /* Get visible rows for recents panel */
            int h, w;
            getmaxyx(state->panels.recents_panel, h, w);
            (void)w;
            adjust_scroll(state->recents_selected, &state->recents_scroll,
                          h - 2, state->history_count);
        }
    } else {
        int count = state->filtered_hosts ? state->filtered_hosts->count : 0;
        if (count > 0) {
            state->all_selected--;
            if (state->all_selected < 0) state->all_selected = 0;
            int h, w;
            getmaxyx(state->panels.all_panel, h, w);
            (void)w;
            adjust_scroll(state->all_selected, &state->all_scroll, h - 2, count);
        }
    }
}

static void navigate_down(AppState* state) {
    if (state->active_panel == PANEL_RECENTS) {
        if (state->history_count > 0) {
            state->recents_selected++;
            if (state->recents_selected >= state->history_count)
                state->recents_selected = state->history_count - 1;
            int h, w;
            getmaxyx(state->panels.recents_panel, h, w);
            (void)w;
            adjust_scroll(state->recents_selected, &state->recents_scroll,
                          h - 2, state->history_count);
        }
    } else {
        int count = state->filtered_hosts ? state->filtered_hosts->count : 0;
        if (count > 0) {
            state->all_selected++;
            if (state->all_selected >= count)
                state->all_selected = count - 1;
            int h, w;
            getmaxyx(state->panels.all_panel, h, w);
            (void)w;
            adjust_scroll(state->all_selected, &state->all_scroll, h - 2, count);
        }
    }
}

static void toggle_panel(AppState* state) {
    if (state->active_panel == PANEL_RECENTS) {
        if (state->filtered_hosts && state->filtered_hosts->count > 0) {
            state->active_panel = PANEL_ALL;
        }
    } else {
        if (state->history_count > 0) {
            state->active_panel = PANEL_RECENTS;
        }
    }
}

/* ---- Search ---- */
static void update_filter(AppState* state) {
    if (state->filtered_hosts != state->all_hosts) {
        host_list_free(state->filtered_hosts);
    }

    if (state->search_buffer[0] == '\0') {
        state->filtered_hosts = state->all_hosts;
    } else {
        state->filtered_hosts = filter_hosts(state->all_hosts, state->search_buffer);
        if (!state->filtered_hosts) {
            /* Allocation failed, fall back to all hosts */
            state->filtered_hosts = state->all_hosts;
        }
    }

    state->all_selected = (state->filtered_hosts && state->filtered_hosts->count > 0) ? 0 : -1;
    state->all_scroll = 0;
}

static void start_search(AppState* state) {
    state->is_searching = true;
    state->search_buffer[0] = '\0';
    state->search_cursor = 0;
    state->all_selected = (state->filtered_hosts && state->filtered_hosts->count > 0) ? 0 : -1;
    state->all_scroll = 0;
    /* Switch focus to all panel */
    state->active_panel = PANEL_ALL;
    curs_set(1);
}

static void cancel_search(AppState* state) {
    state->is_searching = false;
    if (state->filtered_hosts != state->all_hosts) {
        host_list_free(state->filtered_hosts);
    }
    state->filtered_hosts = state->all_hosts;
    state->all_selected = (state->all_hosts && state->all_hosts->count > 0) ? 0 : -1;
    state->all_scroll = 0;
    state->search_buffer[0] = '\0';
    state->search_cursor = 0;
    curs_set(0);
}

static void append_search_char(AppState* state, char c) {
    int len = (int)strlen(state->search_buffer);
    if (len < HOST_NAME_MAX - 1) {
        /* Shift chars right of cursor to make room */
        memmove(&state->search_buffer[state->search_cursor + 1],
                &state->search_buffer[state->search_cursor],
                (size_t)(len - state->search_cursor + 1));
        state->search_buffer[state->search_cursor] = c;
        state->search_cursor++;
        update_filter(state);
    }
}

static void delete_search_char(AppState* state) {
    int len = (int)strlen(state->search_buffer);
    if (len > 0 && state->search_cursor > 0) {
        memmove(&state->search_buffer[state->search_cursor - 1],
                &state->search_buffer[state->search_cursor],
                (size_t)(len - state->search_cursor + 1));
        state->search_cursor--;
        update_filter(state);
    }
}

static void delete_search_char_forward(AppState* state) {
    int len = (int)strlen(state->search_buffer);
    if (state->search_cursor < len) {
        memmove(&state->search_buffer[state->search_cursor],
                &state->search_buffer[state->search_cursor + 1],
                (size_t)(len - state->search_cursor));
        update_filter(state);
    }
}

/* ---- SSH Connection ---- */
static void app_connect(AppState* state, const char* host) {
    if (!host || !state) return;

    /* Save to history */
    history_save(state->history_path, host);
    history_load(state->history_path, state->history, &state->history_count);
    state->recents_selected = (state->history_count > 0) ? 0 : -1;
    state->recents_scroll = 0;

    /* Suspend ncurses */
    ui_suspend();

    printf("Connecting to %s...\n\n", host);
    fflush(stdout);

    pid_t pid = fork();
    if (pid == 0) {
        /* Child: execute ssh */
        execlp("ssh", "ssh", host, (char*)NULL);
        fprintf(stderr, "ssh: command not found or exec failed: %s\n", strerror(errno));
        _exit(EXIT_FAILURE);
    } else if (pid > 0) {
        /* Parent: wait for SSH to finish */
        int status;
        waitpid(pid, &status, 0);
    } else {
        fprintf(stderr, "Failed to fork: %s\n", strerror(errno));
    }

    /* Resume ncurses */
    ui_resume();
}

/* ---- Input Handling ---- */
static const char* get_selected_host(const AppState* state) {
    if (state->active_panel == PANEL_RECENTS) {
        if (state->recents_selected >= 0 && state->recents_selected < state->history_count) {
            return state->history[state->recents_selected];
        }
    } else {
        if (state->filtered_hosts && state->all_selected >= 0 &&
            state->all_selected < state->filtered_hosts->count) {
            return state->filtered_hosts->names[state->all_selected];
        }
    }
    return NULL;
}

static void handle_input(AppState* state, int ch) {
    if (state->is_searching) {
        switch (ch) {
        case 27:  /* Escape */
            cancel_search(state);
            return;
        case '\n':
        case '\r':
        case KEY_ENTER:
            /* Connect to selected host from search results */
            {
                const char* host = get_selected_host(state);
                if (host) {
                    app_connect(state, host);
                }
            }
            return;
        case KEY_BACKSPACE:
        case 127:
        case '\b':
            delete_search_char(state);
            return;
        case KEY_DC:
            delete_search_char_forward(state);
            return;
        case KEY_LEFT:
            if (state->search_cursor > 0)
                state->search_cursor--;
            return;
        case KEY_RIGHT:
            if (state->search_cursor < (int)strlen(state->search_buffer))
                state->search_cursor++;
            return;
        case KEY_UP:
            navigate_up(state);
            return;
        case KEY_DOWN:
            navigate_down(state);
            return;
        case KEY_RESIZE:
            ui_resize(&state->panels);
            return;
        case '\t':  /* Tab: toggle panel while searching */
            toggle_panel(state);
            return;
        case KEY_F(1):  /* Toggle language */
            i18n_toggle();
            i18n_save();
            return;
        case KEY_F(2):  /* Cycle color theme */
            state->theme_index = ui_next_theme(state->theme_index);
            ui_apply_theme(state->theme_index);
            ui_theme_save(state->theme_index);
            return;
        default:
            if (ch >= 32 && ch <= 126) {
                append_search_char(state, (char)ch);
            }
            return;
        }
    }

    /* Normal mode input handling */
    switch (ch) {
    case 27:  /* Escape */
    case 'q':  /* Salir (estilo htop/less/vim) */
        state->running = false;
        break;
    case KEY_UP:
    case 'k':
        navigate_up(state);
        break;
    case KEY_DOWN:
    case 'j':
        navigate_down(state);
        break;
    case '\t':  /* Tab */
        toggle_panel(state);
        break;
    case '\n':
    case '\r':
    case KEY_ENTER:
        {
            const char* host = get_selected_host(state);
            if (host) {
                app_connect(state, host);
            }
        }
        break;
    case '/':
        start_search(state);
        break;
    case KEY_F(1):  /* Toggle language */
        i18n_toggle();
        i18n_save();
        break;
    case KEY_F(2):  /* Cycle color theme */
        state->theme_index = ui_next_theme(state->theme_index);
        ui_apply_theme(state->theme_index);
        ui_theme_save(state->theme_index);
        break;
    case KEY_RESIZE:
        ui_resize(&state->panels);
        /* Re-clamp scroll offsets after resize */
        {
            int h, w;
            getmaxyx(state->panels.recents_panel, h, w);
            (void)w;
            adjust_scroll(state->recents_selected, &state->recents_scroll,
                          h - 2, state->history_count);
        }
        {
            int h, w;
            getmaxyx(state->panels.all_panel, h, w);
            (void)w;
            int count = state->filtered_hosts ? state->filtered_hosts->count : 0;
            adjust_scroll(state->all_selected, &state->all_scroll, h - 2, count);
        }
        break;
    default:
        break;
    }
}

/* ---- Event Loop ---- */
void app_run(AppState* state) {
    if (!state) return;

    while (state->running) {
        ui_draw(&state->panels, state);
        int ch = ui_get_input();

        if (ch == ERR) {
            if (!state->running) break;
            continue;
        }

        handle_input(state, ch);
    }
}

/* ---- Cleanup ---- */
void app_shutdown(AppState* state) {
    if (!state) return;

    ui_shutdown(&state->panels);

    if (state->filtered_hosts != state->all_hosts) {
        host_list_free(state->filtered_hosts);
    }
    host_list_free(state->all_hosts);

    free(state);
    g_app_state = NULL;
}
