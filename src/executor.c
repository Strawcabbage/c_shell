#include "shell.h"

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
    
    //If the argument entered is empty or failed, then the loop is restarted and input is received again
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
    
    int index = 0;
    int num_commands = 0;
    int first_pipe = 0;

    while (args[index] != NULL) {
        
        if (strcmp(args[index], "|") == 0) {

            if (first_pipe == 0) {
                num_commands += 2;
                first_pipe++;
            } else {
                num_commands++;
            }   
        }
        index++;
    } 

    if (num_commands > 1) {
        return fork_pipes(num_commands, initialize_commands(args));
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

    free_pipe_commands(n, cmd);
    
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

