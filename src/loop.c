#include "shell.h"

void csh_loop(void) {
    
    char **args;
    int status;
    char *linecopy;

    //While loop to read read, parse, and execute input
    do {

        if (getcwd(home_dir, sizeof(home_dir)) == NULL) {
            perror("Unable to locate filepath to current directory");
        }
       
        printf("%s> ", home_dir);

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

