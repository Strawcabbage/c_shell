#define _GNU_SOURCE
#define DELIMS " /\r\a\t\n" 
#define MAX_HISTORY 100
#define MAX_PIPES 10
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <limits.h>
#include <termios.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

//Function declerations
char *csh_read_line(void);
char **csh_parse_line(char*, char*);
void csh_loop();
int csh_execute(char**);
int csh_launch(char**);
void enableRawMode();
void disableRawMode();
void addToHistory(const char *);
int csh_exit();
int csh_cd();
int csh_help();


struct pipe_command
{
    char **argv;
};

int spawn_proc(int, int, struct pipe_command *);
int fork_pipes(int, struct pipe_command *);

//Global variables
const char *built_in_strs[] = {"exit", "cd", "help"};
char *line;
char prev_dir[PATH_MAX];
char curr_dir[PATH_MAX];
int curr_hist_index = -1;
char *history[MAX_HISTORY];
int history_count = 0;
char *infile = NULL;
char *outfile = NULL;
struct pipe_command *cmd;
int total_cmds = 0;

//Function pointer to built in commands (exit, echo)
int (*built_in_func[]) (char**) = {
    &csh_exit,
    &csh_cd,
    &csh_help
};

//Built in function exit. This function exits the shell
int csh_exit() {

    return 0;
    
}

int csh_cd() {
    
    char *rest_of_line;
    
    strtok(line, DELIMS);

    rest_of_line = strtok(NULL, DELIMS);

    if (rest_of_line == NULL) {
        perror("Must have file path after command cd");
        return 1;
    }

    if (!(strcmp(rest_of_line, "..") == 0)) {

        strcpy(prev_dir, curr_dir);

        strcat(curr_dir, "/");
        strcat(curr_dir, rest_of_line);

        if (chdir(curr_dir) == -1) {
            perror("Error: cannot find directory");
            return 1;
        } else {
            return 1;
        }

    } else {
        
        if (prev_dir != NULL) {
            
            if (chdir(prev_dir) == -1) {
                perror("Failed to return to previous directory");
                return 1;
            } else {
                strcpy(curr_dir, prev_dir);
                return 1;
            }

        } else {
            perror("Cannot move back directories any further");
            return 1;
        }

    }

}

int csh_help() {
    printf("Here are a list of the built in functions you can use:\n");
    for (int i = 0; i < (sizeof(built_in_strs) / sizeof(built_in_strs[0])); i++) {
        printf("  -  \"%s\"\n", built_in_strs[i]);
    }
    return 1;
}



//Main function which starts the shell input loop function
int main(int argc, char **argv) {
    
    csh_loop();
    return 0;

}

//The shell loop function which loops through recieving and reading input until there is a request to exit
void csh_loop(void) {
    
    char **args;
    int status;
    char *linecopy;
    
    if (getcwd(curr_dir, sizeof(curr_dir)) == NULL) {
        perror("Unable to locate filepath to current directory");
    }

    //While loop to read read, parse, and execute input
    do {
       
        printf("%s> ", curr_dir);

        fflush(stdout);
        infile = NULL;
        outfile = NULL;

        line = csh_read_line();
        linecopy = malloc(strlen(line) + 1);
        args = csh_parse_line(line, linecopy);
        status = csh_execute(args);
       
        //Freeing memory allocated to args and lines to prevent memory leaks before looping again or exiting
        free(line);
        free(args);
        free(linecopy);

        for (int i = 0; cmd[i].argv != NULL; i++) {
            free(cmd[i].argv);
        }
        free(cmd);
        
    } while (status);
    
}

//Function to read input
char *csh_read_line(void) {
    
    //Declaring variables
    size_t bufsize = 0;
    char *buffer = NULL;    


    //enableRawMode();

    /*
     * Using getline() to dynamically allocate memory
     * to the char pointer, buffer, then checking if
     * the function was successful by equating it to -1
     * (the output if EOF is reached or if there's an error)
    */
    if (getline(&buffer, &bufsize, stdin) == -1) {
        //If EOF is reached the the function was a success, else it failed
        if (feof(stdin)) {
            exit(EXIT_SUCCESS);
        } else {
            perror("Error reading line");
            exit(EXIT_FAILURE);
        }
    }

    buffer[strcspn(buffer, "\n")] = 0;
    
    //disableRawMode();

    //Return buffer after getline() was used
    return buffer;

}

//Parsing the read input into a char pointer to an array of Strings
char **csh_parse_line(char *line, char *linecopy) {
    
    //Declaring Variables
    char *str;
    size_t position = 0;
    size_t bufsize = 64;
    char **strs;
    char *delims = DELIMS;
    char *command[10];
    total_cmds = 0;
    int pipe = 0;

    //Making sure dynamic memory allocation succeeded
    if (linecopy == NULL) {
        perror("Error allocating memory to linecopy");
        free(linecopy);
        exit(EXIT_FAILURE);
    }
    
    //Deep copying the char pointer line so it can be used later in the echo built in function
    strcpy(linecopy, line);
    
    //dyamically allocating memory to strs to allow for 64 char pointers
    strs = malloc(bufsize * sizeof(char*));
    
    //Making sure dynamic memory allocation succeeded
    if (strs == NULL) {
        perror("Error allocating memory strs");
        free(strs);
        exit(EXIT_FAILURE);
    }
    
    cmd = malloc(10 * sizeof(struct pipe_command));
    
    if (cmd == NULL) {
        perror("Error allocating memory to cmd");
        free(cmd);
    }

    for (int i = 0; i < 10; ++i) {
        cmd[i].argv = NULL;
    }
    
    //Initializing strtok with string linecopy and delimeters then assiging the first token to str
    str = strtok(linecopy, delims);
    
    //This while loop will parse all of line copy and add each token to strs
    while (str != NULL) {
        
        if (strcmp(str, "<") == 0) {
            
            infile = strtok(NULL, " ");

        } else if (strcmp(str, ">") == 0) {
            
            outfile = strtok(NULL, " ");

        } else {

            //Adding the first token to strs using position as a guide
            strs[position] = str;
        
            //Checking if the memory for string needs to be expanded by seeing if position is greater than the bufsize allocated
            if (position >= bufsize) {
                bufsize *= 2;

                //Using realloc for new allocation
                strs = realloc(strs, bufsize * sizeof(char*));
            
                //Making sure dynamic memory allocation succeeded
                if (strs == NULL) {
                   perror("Error reallocating memory to strs");
                   free(strs);
                   exit(EXIT_FAILURE);
                }
            }
            
        }

        //Null terminating str before adding it to str again
        str = strtok(NULL, delims);
        position++; 
        
    }

        
    //Null ending strs then returning it
    strs[position] = NULL;

    for (int i = 0; strs[i] != NULL; i++) {
        if (strcmp(strs[i], "|") == 0) {

            int last_pipe = -1;
            int command_index = 0;
            for (int j = i - 1; j > -1; j--) {
                if (strcmp(strs[j], "|") == 0) {
                    last_pipe = j;
                    break;
                }
            }
            for (int j = last_pipe + 1; j < i; j++) {
                command[command_index++] = strs[j];
            }
            
             
            command[command_index] = NULL;
            
            
            cmd[total_cmds].argv = malloc(5 * sizeof(char *));
            if (cmd[total_cmds].argv == NULL) {
                perror("Error allocationg memory to argv");
            }
            
            for (int j = 0; command[j] != NULL; j++) {
                cmd[total_cmds].argv[j] = command[j];
            }
            
            printf("%s\n", cmd[0].argv[0]);

            for (int j = 0; command[j] != NULL; j++) { 
                command[j] = NULL;
            }
            
            total_cmds++;
            pipe = 1;
        }
    }

    if (pipe) {
        int last_pipe = 0;
        int command_index = 0;
        
        for (int i = position - 1; i > -1; i--) {
            if (strcmp(strs[i], "|") == 0) {
                last_pipe = i;
                break;
            }
        }
        
        for (int i = last_pipe + 1; strs[i] != NULL; i++) {
            command[command_index++] = strs[i];
            command[command_index] = NULL;
        }
        
        cmd[total_cmds].argv = malloc(5 * sizeof(char *));
        if (cmd[total_cmds].argv == NULL) {
            perror("Error allocating memory to argv for last pipe");   
        }

        for (int i = 0; command[i] != NULL; i++) {
            cmd[total_cmds].argv[i] = command[i];
        }
        
        for (int i = 0; command[i] != NULL; i++) {
            command[i] = NULL;
        }
        
    }
    
    printf("%s\n", cmd[0].argv[0]);
    /*printf("%s\n", cmd[total_cmds - 2].argv[1]);
    printf("%s\n", cmd[total_cmds - 1].argv[0]);
    printf("%s\n", cmd[total_cmds - 1].argv[1]);*/
    cmd[total_cmds].argv = NULL;

    return strs;
}

//Function for executing a program on a new process
int csh_launch(char **args) {
    
    //Declaring variables
    pid_t cpid, wpid;
    int status;
    int status_code;
    int saved_stdin = dup(STDIN_FILENO);
    int saved_stdout = dup(STDOUT_FILENO);

    
    // Forking the parent proccess into a child and the parent
    cpid = fork();
    
    /*
     * Checking if fork failed then executing new program on child
     * process using execvp(). Also telling the parent process to
     * wait for child process to signal that it is finished.
    */
    if (cpid<0) {
        perror("Fork fail");
        exit(EXIT_FAILURE);       
        return 1;
    } else if (cpid == 0) {

        if (infile) {
            int fd_in = open(infile, O_RDONLY);
            if (fd_in < 0) {
            
                perror("failed opening file to read");
                return 1;

            }
            if (dup2(fd_in, STDIN_FILENO) < 0) {
                perror("redirecting input failed");
                close(fd_in);
                return 1;
            }
            close(fd_in);
        }

        if (outfile) {
            int fd_out = open(outfile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd_out < 0) {

                perror("Failed opening file to write");
                return 1;

            }
            if (dup2(fd_out, STDOUT_FILENO) < 0) {
                perror("redirecting output failed");
                close(fd_out);
                return 1;
            }
            close(fd_out);
        }

        status_code = execvp(args[0], args);
        if (status_code == -1) {
            perror("Process did not execute");
        }
        exit(EXIT_FAILURE);

    } else {  
        do { 
            wpid = waitpid(cpid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
        
        fflush(stdout);
        dup2(saved_stdin, STDIN_FILENO);
        dup2(saved_stdout, STDOUT_FILENO);
        close(saved_stdin);
        close(saved_stdout);

    }
    return 1;
}

/* 
 * The function for deciding whether a command is for a built in
 * function or a system command by using built_in_strs
 * then returning a status after checking
*/
int csh_execute(char **args) {
    
    //If the argument entered is empty or failed, then the loop is restarted and input is recieved again
    if (args[0] == NULL || *args[0] == '\n') {
        return 1;
    }
 
    //Checkin whether the command is built in by looping through built_in_strs
    for (int i = 0; i < (sizeof(built_in_strs) / sizeof(built_in_strs[0])); i++) {
        if (strcmp(args[0], built_in_strs[i]) == 0) {
            //The command has evaluated to be a built in command
            return (*built_in_func[i])(args);
        }
    }

    if (cmd[0].argv != NULL) {
        return fork_pipes(total_cmds, cmd);
    }

    //The command is not built in and now we run the function to launch the new command
    return csh_launch(args);
}

int fork_pipes(int n, struct pipe_command *cmd) {
    int i;
    pid_t pid;
    int in, fd [2];
    
    // The first process should get its input from the original file descriptor 0.
    in = 0;

    // Note the loop bound, we spawn here all, but the last stage of the pipeline.
    for (i = 0; i < n - 1; i++) {
        
        for (int j = 0; cmd[i].argv[j] != NULL; j++) {
            printf("%s\n", cmd[i].argv[j]);
        }
        printf("\n");
        
        if (pipe(fd) == -1) {
            perror("pipe");
        }
        // f [1] is the write end of the pipe, we carry `in` from the prev iteration.
        spawn_proc(in, fd [1], cmd + i);

        // No need for the write end of the pipe, the child will write here.
        close(fd [1]);

        // Keep the read end of the pipe, the next child will read from there.
        in = fd[0];
        }

    /* Last stage of the pipeline - set stdin be the read end of the previous pipe
     and output to the original file descriptor 1. */
    if (in != 0)
    dup2(in, 0);

    // execute the last stage with the current process.
    if (execvp(cmd[i].argv [0], (char *const *)cmd[i].argv) == -1) {
        perror("failed to execute command");
    }
    
    fflush(stdout);

    return 1;
}

int spawn_proc(int in, int out, struct pipe_command *cmd) {
    pid_t pid;

    if ((pid = fork()) == 0) {
        if (in != 0) {
            dup2(in, 0);
            close(in);
        }

        if (out != 1) {
            dup2(out, 1);
            close(out);
        }

        return execvp (cmd->argv[0], (char *const *)cmd->argv);
    }

    return pid;
}

void addToHistory(const char *cmd) {
    if (history_count < MAX_HISTORY) {
        history[history_count] = strdup(cmd);
        history_count++;
    } else {
        perror("Max history count exceeded, please restart shell");
    }
    curr_hist_index = history_count;
}

void enableRawMode() {
    struct termios raw;
    tcgetattr(STDIN_FILENO, &raw);
    raw.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}


void disableRawMode() {
    struct termios raw;
    tcgetattr(STDIN_FILENO, &raw);
    raw.c_lflag |= ICANON | ECHO;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}
