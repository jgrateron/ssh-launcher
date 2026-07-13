#include "i18n.h"
#include <stdlib.h>
#include <string.h>

/* Current language */
static Language current_language = LANG_ES;

/* ---- String tables ---- */
static const char* strings[LANG_COUNT][STR_COUNT] = {
    /* LANG_ES (Spanish) */
    [LANG_ES] = {
        [STR_RECIENTES]       = " RECIENTES ",
        [STR_ALL_SERVERS]     = " TODOS LOS SERVIDORES ",
        [STR_BUSQUEDA_FMT]    = " BÚSQUEDA: %s ",
        [STR_EMPTY]           = "(vacio)",
        [STR_NO_HOSTS_CONFIG] = "No hay hosts en ~/.ssh/config",
        [STR_NO_MATCHES]      = "Sin coincidencias",
        [STR_NO_HOSTS_FOUND]  = "No se encontraron hosts",
        [STR_SEARCH_LABEL]    = "Búsqueda: %s",
        [STR_SHORTCUTS]       = "[Enter] Conectar  [/] Buscar  [Tab] Panel  [F2] Temas  [Esc/q] %s",
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
        [STR_SHORTCUTS]       = "[Enter] Connect  [/] Search  [Tab] Panel  [F2] Themes  [Esc/q] %s",
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

void i18n_load(void) {
    /* Auto-detect language from $LANG environment variable.
     * Spanish if LANG starts with "es", English for everything else. */
    const char* lang = getenv("LANG");
    if (!lang) lang = getenv("LC_ALL");
    if (!lang) lang = getenv("LC_MESSAGES");

    if (lang && (strncmp(lang, "es", 2) == 0)) {
        current_language = LANG_ES;
    } else {
        current_language = LANG_EN;
    }
}
