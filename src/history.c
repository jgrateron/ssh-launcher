#include "history.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

const char* history_default_path(char* buf, size_t bufsize) {
    const char* home = getenv("HOME");
    if (!home) home = ".";
    snprintf(buf, bufsize, "%s/.ssh_launcher_history", home);
    return buf;
}

int history_load(const char* path, char buffer[HISTORY_MAX][HOST_NAME_MAX], int* count) {
    if (!path || !buffer || !count) return -1;

    *count = 0;

    FILE* file = fopen(path, "r");
    if (!file) return 0;  /* No history yet, not an error */

    char line[HOST_NAME_MAX];
    while (*count < HISTORY_MAX && fgets(line, sizeof(line), file)) {
        /* Strip trailing newline */
        size_t len = strlen(line);
        while (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r')) {
            line[--len] = '\0';
        }

        if (len > 0) {
            strncpy(buffer[*count], line, HOST_NAME_MAX - 1);
            buffer[*count][HOST_NAME_MAX - 1] = '\0';
            (*count)++;
        }
    }

    fclose(file);
    return 0;
}

int history_save(const char* path, const char* host) {
    if (!path || !host) return -1;

    /* Load existing entries */
    char existing[HISTORY_MAX][HOST_NAME_MAX];
    int existing_count = 0;
    history_load(path, existing, &existing_count);

    /* Build new list: new host first, then existing (minus duplicate) */
    char new_entries[HISTORY_MAX][HOST_NAME_MAX];
    int new_count = 0;

    /* Add the new host at position 0 */
    strncpy(new_entries[new_count], host, HOST_NAME_MAX - 1);
    new_entries[new_count][HOST_NAME_MAX - 1] = '\0';
    new_count++;

    /* Add existing entries, skipping duplicates of the new host */
    for (int i = 0; i < existing_count && new_count < HISTORY_MAX; i++) {
        if (strcmp(existing[i], host) != 0) {
            strncpy(new_entries[new_count], existing[i], HOST_NAME_MAX - 1);
            new_entries[new_count][HOST_NAME_MAX - 1] = '\0';
            new_count++;
        }
    }

    /* Write to file */
    FILE* file = fopen(path, "w");
    if (!file) return -1;

    for (int i = 0; i < new_count; i++) {
        fprintf(file, "%s\n", new_entries[i]);
    }

    fclose(file);
    return 0;
}
