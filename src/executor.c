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
        // Piped commands
        if (fork_pipes(num_commands, initialize_commands(args)) == -1) {
            perror("Failed to execute piped commands");
            return 1; // Continue the shell loop on failure
        }
        return 1; // Continue the shell loop on success
    }

    // Non-piped command
    if (csh_launch(args) == -1) {
        perror("Failed to launch command");
    }

    return 1;
}

int fork_pipes(int n, struct pipe_command *cmd) {
    int i;
    int in = 0;          // Input file descriptor (initially stdin)
    int fd[2];           // Pipe file descriptors
    pid_t pid;

    for (i = 0; i < n - 1; i++) {
        // Create a pipe
        if (pipe(fd) == -1) {
            perror("pipe failed");
            free_pipe_commands(n, cmd);
            return -1;
        }

        // Fork a child process
        if ((pid = fork()) == -1) {
            perror("fork failed");
            free_pipe_commands(n, cmd);
            return -1;
        }

        if (pid == 0) {
            // Child process
            if (in != 0) {
                dup2(in, STDIN_FILENO); // Set stdin to the input of the previous pipe
                close(in);
            }
            dup2(fd[1], STDOUT_FILENO); // Set stdout to the write end of the current pipe
            close(fd[0]);               // Close unused read end of the pipe
            close(fd[1]);               // Close write end after dup2

            execvp(cmd[i].argv[0], cmd[i].argv); // Execute the command
            perror("execvp failed");            // If execvp fails
            exit(EXIT_FAILURE);
        } else {
            // Parent process
            close(fd[1]); // Close write end of the current pipe
            if (in != 0) close(in); // Close previous input
            in = fd[0]; // Save read end of the current pipe for the next iteration
        }
    }

    // Handle the last command in the pipeline
    if ((pid = fork()) == -1) {
        perror("fork failed");
        free_pipe_commands(n, cmd);
        return -1;
    }

    if (pid == 0) {
        if (in != 0) {
            dup2(in, STDIN_FILENO); // Set stdin to the input of the last pipe
            close(in);
        }

        execvp(cmd[i].argv[0], cmd[i].argv); // Execute the last command
        perror("execvp failed");            // If execvp fails
        exit(EXIT_FAILURE);
    }

    close(in); // Close the final read end in the parent process

    // Wait for all child processes to complete
    for (i = 0; i < n; i++) {
        wait(NULL);
    }

    free_pipe_commands(n, cmd);
    return 0;
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

