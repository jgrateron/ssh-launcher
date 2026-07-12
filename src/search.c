#include "search.h"
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

bool fuzzy_match(const char* pattern, const char* target) {
    if (!pattern || !*pattern) return true;
    if (!target) return false;

    while (*pattern) {
        char pc = (char)tolower((unsigned char)*pattern);

        /* Advance target until we find matching char or hit end */
        while (*target && tolower((unsigned char)*target) != pc)
            target++;

        if (!*target) return false;  /* pattern char not found */
        target++;
        pattern++;
    }

    return true;
}

HostList* filter_hosts(const HostList* source, const char* pattern) {
    if (!source) return NULL;

    /* Empty pattern or NULL -> return shallow copy of all hosts */
    if (!pattern || !*pattern) {
        HostList* result = host_list_create(source->count > 0 ? source->count : 1);
        if (!result) return NULL;
        for (int i = 0; i < source->count; i++) {
            if (host_list_append(result, source->names[i]) != 0) {
                host_list_free(result);
                return NULL;
            }
        }
        return result;
    }

    HostList* result = host_list_create(HOST_LIST_INITIAL_CAPACITY);
    if (!result) return NULL;

    for (int i = 0; i < source->count; i++) {
        if (fuzzy_match(pattern, source->names[i])) {
            if (host_list_append(result, source->names[i]) != 0) {
                host_list_free(result);
                return NULL;
            }
        }
    }

    return result;
}
