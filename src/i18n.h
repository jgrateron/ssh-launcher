#ifndef SSH_LAUNCHER_I18N_H
#define SSH_LAUNCHER_I18N_H

typedef enum {
    LANG_ES,
    LANG_EN,
    LANG_COUNT
} Language;

/* All translatable string keys */
typedef enum {
    STR_RECIENTES,
    STR_ALL_SERVERS,
    STR_BUSQUEDA_FMT,      /* "BUSQUEDA: %s " / "SEARCH: %s " */
    STR_EMPTY,             /* "(vacio)" / "(empty)" */
    STR_NO_HOSTS_CONFIG,   /* "No hosts in ~/.ssh/config" */
    STR_NO_MATCHES,        /* "No matches" */
    STR_NO_HOSTS_FOUND,    /* "No hosts found" */
    STR_SEARCH_LABEL,      /* "Busqueda: %s" / "Search: %s" */
    STR_SHORTCUTS,         /* "[Enter] Conectar ..." */
    STR_CANCEL,            /* "Cancelar" / "Cancel" */
    STR_EXIT,              /* "Salir" / "Exit" */
    STR_CONNECTING,        /* "Connecting to %s..." */
    STR_TERMINAL_SMALL,    /* "Terminal too small..." */
    STR_PRESS_ANY_KEY,     /* "Press any key to exit." */
    STR_COUNT
} StringKey;

/* Translate a key to the current language */
const char* t(StringKey key);

/* Set/get/toggle language */
void        i18n_set_language(Language lang);
Language    i18n_get_language(void);
Language    i18n_toggle(void);

/* Persist language to ~/.config/ssh-launcher/language */
int         i18n_save(void);
void        i18n_load(void);

#endif /* SSH_LAUNCHER_I18N_H */
