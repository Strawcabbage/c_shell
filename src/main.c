#include "shell.h"

char **directory_array;
int directory_count = 0;
const char *built_in_strs[] = {"exit", "cd", "help"};
char *line;
char prev_dir[PATH_MAX];
char home_dir[PATH_MAX];
int curr_hist_index = -1;
char *history[MAX_HISTORY];
int history_count = 0;
char *infile = NULL;
char *outfile = NULL;

int main(int argc, char **argv) {

    directory_array = malloc(PATH_MAX * sizeof(char *));
    if (directory_array == NULL) {
        perror("Failed to allocate memory for directory_array");
        exit(EXIT_FAILURE);
    }

    csh_loop();

    // Free memory for directory_array before exiting
    for (int i = 0; i < directory_count; i++) {
        free(directory_array[i]);
    }
    free(directory_array);

    return 0;
}

