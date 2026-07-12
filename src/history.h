#ifndef SSH_LAUNCHER_HISTORY_H
#define SSH_LAUNCHER_HISTORY_H

#include <stddef.h>

#define HISTORY_MAX  10
#define HOST_NAME_MAX 256

/* Load history from the given file path into the pre-allocated buffer.
 * `buffer` must be `char[HISTORY_MAX][HOST_NAME_MAX]`.
 * `count` receives the number of entries loaded (0 to HISTORY_MAX).
 * Returns 0 on success, -1 if file cannot be opened (no history yet is not an error,
 * count will be set to 0). */
int history_load(const char* path, char buffer[HISTORY_MAX][HOST_NAME_MAX], int* count);

/* Save a host to the history file. The given host is promoted to position 0
 * (most recent). If the host already exists, its old entry is removed.
 * The result is truncated to HISTORY_MAX entries.
 * Creates the file if it does not exist.
 * Returns 0 on success, -1 on write failure. */
int history_save(const char* path, const char* host);

/* Resolve the default history path: $HOME/.ssh_launcher_history.
 * Writes result into `buf` of size `bufsize`. Returns buf. */
const char* history_default_path(char* buf, size_t bufsize);

#endif /* SSH_LAUNCHER_HISTORY_H */
