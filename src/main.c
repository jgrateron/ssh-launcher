#include "app.h"
#include <stdlib.h>
#include <stdio.h>

int main(void) {
    AppState* state = app_init();
    if (!state) {
        fprintf(stderr, "Failed to initialize SSH Launcher.\n");
        return EXIT_FAILURE;
    }
    app_run(state);
    app_shutdown(state);
    return EXIT_SUCCESS;
}
