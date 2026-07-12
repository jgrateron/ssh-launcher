#ifndef SSH_LAUNCHER_PARSER_H
#define SSH_LAUNCHER_PARSER_H

#define HOST_LIST_INITIAL_CAPACITY 16

typedef struct {
    char** names;      /* dynamically allocated array of host name strings */
    int    count;      /* number of entries currently stored */
    int    capacity;   /* allocated size of the names array */
} HostList;

/* Create a new empty HostList with the given initial capacity.
 * Returns NULL on allocation failure. */
HostList* host_list_create(int initial_capacity);

/* Parse ~/.ssh/config at `path`. Returns NULL on file open failure.
 * Returns a HostList even if zero hosts found (empty list).
 * Skips wildcard hosts (containing * or ?).
 * Caller owns the returned HostList and must call host_list_free(). */
HostList* host_list_parse(const char* path);

/* Append a host name to the list (copies the string internally).
 * Returns 0 on success, -1 on allocation failure. */
int host_list_append(HostList* list, const char* name);

/* Free all memory owned by the list. Safe to call with NULL. */
void host_list_free(HostList* list);

#endif /* SSH_LAUNCHER_PARSER_H */
