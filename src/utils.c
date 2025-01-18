#include "shell.h"

void add_to_history(char *cmd) {
    if (history_count < MAX_HISTORY) {
        history[history_count++] = strdup(cmd);
    } else {
        free(history[0]);
        for (int i = 0; i < MAX_HISTORY - 1; i++) {
            history[i] = strdup(history[i + 1]);
        }
        history[MAX_HISTORY - 1] = strdup(cmd);
    }
    curr_hist_index = history_count;
}

void print_history() {
    
   int temp_index = curr_hist_index - 1;

   for (int i = 0; i < temp_index; i++) {
        printf(" - %s\n", history[i]);
   }

}

void free_pipe_commands(int num_commands, struct pipe_command *cmd) {
    if (cmd == NULL) {
        return;
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
