#ifndef SSH_LAUNCHER_SEARCH_H
#define SSH_LAUNCHER_SEARCH_H

#include <stdbool.h>
#include "parser.h"

/* Fuzzy match: returns true if all characters of `pattern` appear in `target`
 * in order, but not necessarily consecutively. Case-insensitive.
 * An empty pattern matches everything (returns true). */
bool fuzzy_match(const char* pattern, const char* target);

/* Filter a HostList, producing a new HostList containing only hosts
 * that fuzzy_match against `pattern`. The returned list points to the same
 * string memory (shallow copy -- do NOT free the original strings separately).
 * Returns NULL on allocation failure.
 * Caller owns the returned HostList and must call host_list_free() on it.
 * If pattern is empty or NULL, returns a shallow copy of all hosts. */
HostList* filter_hosts(const HostList* source, const char* pattern);

#endif /* SSH_LAUNCHER_SEARCH_H */
