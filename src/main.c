#include "shell.h"

char **directory_array;
int directory_count = 0;
const char *built_in_strs[NUM_BUILTINS] = {"exit", "cd", "help", "history", ":"};
char *line;
char prev_dir[PATH_MAX];
char home_dir[PATH_MAX];
int curr_hist_index = -1;
char *history[MAX_HISTORY];
int history_count = 0;
char *infile = NULL;
char *outfile = NULL;
struct pipe_command *cmd;
int total_cmds = 0;
int last_status  = 0;

int main(int argc, char **argv) {
    int opt;
    const char *cmd = NULL;

    directory_array = malloc(PATH_MAX * sizeof(char *));
    if (directory_array == NULL) {
        perror("Failed to allocate memory for directory_array");
        exit(EXIT_FAILURE);
    }

    // Parse: -c "command"
    while ((opt = getopt(argc, argv, "c:")) != -1) {
        switch (opt) {
            case 'c':
                cmd = optarg;
                break;
            default:
                fprintf(stderr, "usage: %s [-c command] [script [args...]]\n", argv[0]);
                // Free and exit like a usage error
                for (int i = 0; i < directory_count; i++) free(directory_array[i]);
                free(directory_array);
                return 2;
        }
    }

    int exit_code = 0;

    if (cmd) {
        // One-liner mode
        exit_code = csh_run_command_string(cmd);
    } else if (optind < argc) {
        // Script file mode
        const char *path = argv[optind++];
        FILE *fp = fopen(path, "r");
        if (!fp) {
            fprintf(stderr, "%s: cannot open %s: %s\n", argv[0], path, strerror(errno));
            exit_code = 127;
        } else {
            exit_code = csh_run_script_stream(fp, path);
            fclose(fp);
        }
    } else if (!isatty(STDIN_FILENO)) {
        // Read commands from stdin
        exit_code = csh_run_script_stream(stdin, "<stdin>");
    } else {
        // Interactive TTY mode
        csh_loop();
    }

    // Free memory for directory_array before exiting
    for (int i = 0; i < directory_count; i++) {
        free(directory_array[i]);
    }
    free(directory_array);

    return exit_code;
}


