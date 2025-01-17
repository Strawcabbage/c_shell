// shell.h
#ifndef SHELL_H
#define SHELL_H

#define DELIMS " /\r\a\t\n"
#define MAX_HISTORY 100
#define MAX_PIPES 10
#define MAX_PIPE_ARGUMENTS 10

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
extern int (*built_in_func[])(char **);
extern const char *built_in_strs[3];
extern char *line;
extern char prev_dir[PATH_MAX];
extern char home_dir[PATH_MAX];
extern int curr_hist_index;
extern char *history[MAX_HISTORY];
extern int history_count;
extern char *infile;
extern char *outfile;

struct pipe_command* initialize_commands(char **strs);
int csh_execute(char **);
int csh_launch(char **);
int csh_exit();
int csh_cd();
int csh_help();
char *csh_read_line(void);
char **csh_parse_line(char *, char *);
void csh_loop(void);
int fork_pipes(int, struct pipe_command *);
int spawn_proc(int, int, struct pipe_command *);
void addToHistory(const char *);
void enableRawMode(void);
void disableRawMode(void);

#endif

