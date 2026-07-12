#include "parser.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

HostList* host_list_create(int initial_capacity) {
    if (initial_capacity < 1) initial_capacity = HOST_LIST_INITIAL_CAPACITY;

    HostList* list = malloc(sizeof(HostList));
    if (!list) return NULL;

    list->names = malloc((size_t)initial_capacity * sizeof(char*));
    if (!list->names) {
        free(list);
        return NULL;
    }

    list->count = 0;
    list->capacity = initial_capacity;
    return list;
}

static bool host_name_contains_wildcard(const char* name) {
    return strchr(name, '*') != NULL || strchr(name, '?') != NULL;
}

int host_list_append(HostList* list, const char* name) {
    if (!list || !name) return -1;

    /* Resize if needed */
    if (list->count >= list->capacity) {
        int new_capacity = list->capacity * 2;
        char** new_names = realloc(list->names, (size_t)new_capacity * sizeof(char*));
        if (!new_names) return -1;
        list->names = new_names;
        list->capacity = new_capacity;
    }

    list->names[list->count] = strdup(name);
    if (!list->names[list->count]) return -1;

    list->count++;
    return 0;
}

HostList* host_list_parse(const char* path) {
    if (!path) return NULL;

    FILE* file = fopen(path, "r");
    if (!file) {
        /* File doesn't exist -- return empty list, not an error */
        return host_list_create(HOST_LIST_INITIAL_CAPACITY);
    }

    HostList* list = host_list_create(HOST_LIST_INITIAL_CAPACITY);
    if (!list) {
        fclose(file);
        return NULL;
    }

    char line[4096];
    while (fgets(line, sizeof(line), file)) {
        /* Trim trailing newline */
        size_t len = strlen(line);
        while (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r')) {
            line[--len] = '\0';
        }

        /* Skip empty lines and comments */
        if (len == 0 || line[0] == '#') continue;

        /* Check if this is a Host line (case-sensitive as per SSH config spec) */
        if (strncmp(line, "Host ", 5) != 0) {
            /* Also check for "Host\t" (tab-separated) */
            if (strncmp(line, "Host", 4) != 0 || (line[4] != ' ' && line[4] != '\t'))
                continue;
        }

        /* Tokenize the rest of the line after "Host" */
        char* rest = line + 4;  /* skip "Host" */
        char* saveptr;
        char* token = strtok_r(rest, " \t", &saveptr);

        while (token) {
            if (!host_name_contains_wildcard(token) && strlen(token) > 0) {
                if (host_list_append(list, token) != 0) {
                    /* Allocation failure -- return what we have so far */
                    fclose(file);
                    return list;
                }
            }
            token = strtok_r(NULL, " \t", &saveptr);
        }
    }

    fclose(file);
    return list;
}

void host_list_free(HostList* list) {
    if (!list) return;
    for (int i = 0; i < list->count; i++) {
        free(list->names[i]);
    }
    free(list->names);
    free(list);
}
