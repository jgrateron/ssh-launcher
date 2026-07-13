#include "i18n.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

/* Current language */
static Language current_language = LANG_ES;

/* ---- String tables ---- */
static const char* strings[LANG_COUNT][STR_COUNT] = {
    /* LANG_ES (Spanish) */
    [LANG_ES] = {
        [STR_RECIENTES]       = " RECIENTES ",
        [STR_ALL_SERVERS]     = " TODOS LOS SERVIDORES ",
        [STR_BUSQUEDA_FMT]    = " BUSQUEDA: %s ",
        [STR_EMPTY]           = "(vacio)",
        [STR_NO_HOSTS_CONFIG] = "No hay hosts en ~/.ssh/config",
        [STR_NO_MATCHES]      = "Sin coincidencias",
        [STR_NO_HOSTS_FOUND]  = "No se encontraron hosts",
        [STR_SEARCH_LABEL]    = "Busqueda: %s",
        [STR_SHORTCUTS]       = "[Enter] Conectar  [/] Buscar  [Tab] Panel  [F1] Idioma  [F2] Temas  [Esc/q] %s",
        [STR_CANCEL]          = "Cancelar",
        [STR_EXIT]            = "Salir",
        [STR_CONNECTING]      = "Conectando a %s...",
        [STR_TERMINAL_SMALL]  = "Terminal muy pequeno. Minimo 40x10.",
        [STR_PRESS_ANY_KEY]   = "Presiona cualquier tecla para salir.",
    },
    /* LANG_EN (English) */
    [LANG_EN] = {
        [STR_RECIENTES]       = " RECENT ",
        [STR_ALL_SERVERS]     = " ALL SERVERS ",
        [STR_BUSQUEDA_FMT]    = " SEARCH: %s ",
        [STR_EMPTY]           = "(empty)",
        [STR_NO_HOSTS_CONFIG] = "No hosts in ~/.ssh/config",
        [STR_NO_MATCHES]      = "No matches",
        [STR_NO_HOSTS_FOUND]  = "No hosts found",
        [STR_SEARCH_LABEL]    = "Search: %s",
        [STR_SHORTCUTS]       = "[Enter] Connect  [/] Search  [Tab] Panel  [F1] Lang  [F2] Themes  [Esc/q] %s",
        [STR_CANCEL]          = "Cancel",
        [STR_EXIT]            = "Exit",
        [STR_CONNECTING]      = "Connecting to %s...",
        [STR_TERMINAL_SMALL]  = "Terminal too small. Need at least 40x10.",
        [STR_PRESS_ANY_KEY]   = "Press any key to exit.",
    },
};

/* ---- Public API ---- */
const char* t(StringKey key) {
    if (key >= STR_COUNT) return "???";
    return strings[current_language][key];
}

void i18n_set_language(Language lang) {
    if (lang < LANG_COUNT) current_language = lang;
}

Language i18n_get_language(void) {
    return current_language;
}

Language i18n_toggle(void) {
    current_language = (current_language + 1) % LANG_COUNT;
    return current_language;
}

/* ---- Persistence ---- */
static void lang_config_path(char* buf, size_t bufsize) {
    const char* home = getenv("HOME");
    if (!home) home = ".";
    snprintf(buf, bufsize, "%s/.config/ssh-launcher/language", home);
}

int i18n_save(void) {
    char path[512];
    lang_config_path(path, sizeof(path));

    /* Ensure parent directories exist */
    char dir[512];
    snprintf(dir, sizeof(dir), "%s/.config/ssh-launcher",
             getenv("HOME") ? getenv("HOME") : ".");
    mkdir(dir, 0755);
    snprintf(dir, sizeof(dir), "%s/.config",
             getenv("HOME") ? getenv("HOME") : ".");
    mkdir(dir, 0755);

    FILE* f = fopen(path, "w");
    if (!f) return -1;
    fprintf(f, "%d\n", (int)current_language);
    fclose(f);
    return 0;
}

void i18n_load(void) {
    char path[512];
    lang_config_path(path, sizeof(path));

    FILE* f = fopen(path, "r");
    if (!f) return;  /* No saved language, keep default (ES) */

    int val = 0;
    if (fscanf(f, "%d", &val) == 1) {
        if (val >= 0 && val < LANG_COUNT)
            current_language = (Language)val;
    }
    fclose(f);
}
