#define _GNU_SOURCE
#define DELIMS " /\r\a\t\n" 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <limits.h>

//Function declerations
char *csh_read_line(void);
char **csh_parse_line(char*, char*);
void csh_loop();
int csh_execute(char**);
int csh_launch(char**);
int csh_exit();
int csh_cd();

//Global variables
const char *built_in_strs[] = {"exit", "cd"};
char *line;

//Function pointer to built in commands (exit, echo)
int (*built_in_func[]) (char**) = {
    &csh_exit,
    &csh_cd
};

//Built in function exit. This function exits the shell
int csh_exit() {

    return 0;
    
}

int csh_cd() {
     
    char *rest_of_line;
    char cwd[PATH_MAX];
    int bufsize = 64;
    char **tokens = malloc(bufsize * sizeof(char*));    
    char *token;
    int position = 0;
    char new_cwd[PATH_MAX] = "";

    strtok(line, DELIMS);

    rest_of_line = strtok(NULL, DELIMS);

    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        
        int len = strlen(cwd);
        
        if (strcmp(rest_of_line, "..") == 0) {
            
            token = strtok(cwd, "/");

            if (token == NULL) {
                perror("Error cannot find parent directory");
                free(tokens);
                return 1;
            }
            
            while (token != NULL) {
                
                tokens[position] = token;

                if (position >= bufsize) {
                    bufsize *= 2;
                    tokens = realloc(tokens, bufsize * sizeof(char*));

                    if (tokens == NULL) {
                        perror("Error allocating memory");
                        free(tokens);
                        return 1;
                    }
                }

                position++;
                token = strtok(NULL, "/");
            }

            tokens[position - 1] = NULL;
            int i = 0;
            strcat(new_cwd, tokens[i]);
            i++;
            
            while (tokens[i] != NULL) {
                
                strcat(new_cwd, "/");
                strcat(new_cwd, tokens[i]);
                i++;

            }
            
            if (chdir(new_cwd) == -1) {
                perror("Error finding directory");
                free(tokens);
                return 1;
            } else {
                free(tokens);
                return 1;
            }

        } else if ((len + strlen(rest_of_line) + 1) < (PATH_MAX)) {
            
            strcat(cwd, "/");
            strcat(cwd, rest_of_line);
                        
            if (chdir(cwd) == -1) {
                perror("Error finding directory");
                return 1;
            } else {
                return 1;
            }
        } else {
            printf("Error path size is too large");
            return 1;
        }
    } else {
        perror("Error getting directory");
        return 1;
    }

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

    //While loop to read read, parse, and execute input
    do {
       
       printf("> ");

       line = csh_read_line();
       linecopy = malloc(strlen(line) + 1);
       args = csh_parse_line(line, linecopy);
       status = csh_execute(args);
       
       //Freeing memory allocated to args and line to prevent memory leaks before looping again or exiting
       free(line);
       free(args);
       free(linecopy);

    } while (status);
    
}

//Function to read input
char *csh_read_line(void) {
    
    //Declaring variables
    size_t bufsize = 0;
    char *buffer = NULL;    
        
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
   
   
    //Making sure dynamic memory allocation succeeded
    if (linecopy == NULL) {
        perror("Error allocating memory");
        free(linecopy);
        exit(EXIT_FAILURE);
    }
    
    //Deep copying the char pointer line so it can be used later in the echo built in function
    strcpy(linecopy, line);
    
    //dyamically allocating memory to strs to allow for 64 char pointers
    strs = malloc(bufsize * sizeof(char*));
    
    //Making sure dynamic memory allocation succeeded
    if (strs == NULL) {
        perror("Error allocating memory");
        free(strs);
        exit(EXIT_FAILURE);
    }
    
    //Initializing strtok with string linecopy and delimeters then assiging the first token to str
    str = strtok(linecopy, delims);
    
    //This while loop will parse all of line copy and add each token to strs
    while (str != NULL) {
        
        //Adding the first token to strs using position as a guide
        strs[position] = str;
        
        //Checking if the memory for string needs to be expanded by seeing if position is greater than the bufsize allocated
        if (position >= bufsize) {
            bufsize *= 2;

            //Using realloc for new allocation
            strs = realloc(strs, bufsize * sizeof(char*));
            
            //Making sure dynamic memory allocation succeeded
            if (strs == NULL) {
               perror("Error allocating memory");
               free(strs);
               exit(EXIT_FAILURE);
            }
        }
        
        //Null terminating str before adding it to str again
        str = strtok(NULL, delims);
        position++;

    }
    
    //Null ending strs then returning it
    strs[position] = NULL;
    return strs;
}

//Function for executing a program on a new process
int csh_launch(char **args) {
    
    //Declaring variables
    pid_t pid, wpid;
    int status;
    
    //Forking the parent proccess into a child and the parent
    pid = fork();
    
    /*
     * Checking if fork failed then executing new program on child
     * process using execvp(). Also telling the parent process to
     * wait for child process to signal that it is finished.
    */
    if (pid<0) {
        perror("Fork fail");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        int status_code = execvp(args[0], args);
        if (status_code == -1) {
            perror("Process did not execute");
        }
        exit(EXIT_FAILURE);
    } else {
          
        do { 
            wpid = waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }
    return 1;
}

/*
 * Function for deciding whether a command is for a built in
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
    //The command is not built in and now we run the function to launch the new command
    return csh_launch(args);
}

