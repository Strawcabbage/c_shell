#include "shell.h"

void addToHistory(const char *cmd) {
    if (history_count < MAX_HISTORY) {
        history[history_count] = strdup(cmd);
        history_count++;
    } else {
        perror("Max history count exceeded, please restart shell");
    }
    curr_hist_index = history_count;
}

void free_pipe_commands(int num_commands, struct pipe_command *cmd) {
    if (cmd == NULL) {
        return; // Nothing to free
    }

    // Loop through each command
    for (int i = 0; i < num_commands; i++) {
        if (cmd[i].argv != NULL) {
            // Free each string in argv
            for (int j = 0; cmd[i].argv[j] != NULL; j++) {
                free(cmd[i].argv[j]);
            }
            // Free the argv array itself
            free(cmd[i].argv);
        }
    }

    // Free the array of pipe_command structs
    free(cmd);
}

/*
 *  |Debugging Functions|
 */

void print_commands(struct pipe_command *cmd, int num_commands) {
    printf("Printing parsed commands:\n");
    for (int i = 0; i < num_commands; i++) {
        printf("Command %d:\n", i + 1);
        for (int j = 0; cmd[i].argv[j] != NULL; j++) {
            printf("  Arg %d: %s\n", j + 1, cmd[i].argv[j]);
        }
        printf("\n");
    }
}
