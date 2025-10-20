// shell.h
#ifndef SHELL_H
#define SHELL_H

#define DELIMS " \r\a\t\n"
#define MAX_HISTORY 10
#define MAX_PIPES 10
#define MAX_PIPE_ARGUMENTS 10
#define NUM_BUILTINS 5

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <limits.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

struct pipe_command {
    char **argv;
};

extern char **directory_array;
extern int directory_count;
extern int (*built_in_func[NUM_BUILTINS])(char **);
extern const char *built_in_strs[NUM_BUILTINS];
extern char *line;
extern char prev_dir[PATH_MAX];
extern char home_dir[PATH_MAX];
extern int curr_hist_index;
extern char *history[MAX_HISTORY];
extern int history_count;
extern char *infile;
extern char *outfile;
extern int last_status;

/*
 *  |function declerations|
 */

// builtins.c
int csh_exit(char **);
int csh_cd(char **);
int csh_help(char **);
int csh_history(char **);


// loop.c
char *csh_read_line(void);
void csh_loop(void);

// parser.c
char **csh_parse_line(char *);
struct pipe_command* initialize_commands(char **strs);

// executor.c
int csh_execute(char **);
int csh_launch(char **);
int fork_pipes(int, struct pipe_command *);
int spawn_proc(int, int, struct pipe_command *);

// utils.c
void add_to_history(char *);
void print_history(void);
void free_pipe_commands(int, struct pipe_command *);
void print_commands(struct pipe_command *, int);

int csh_run_command_string(const char *cmd);
int csh_run_script_stream(FILE *fp, const char *name);

#endif

