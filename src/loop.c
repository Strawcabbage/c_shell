#include "shell.h"

static char *trim(char *s);

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

        
        add_to_history(line);


        linecopy = strdup(line);

        args = csh_parse_line(linecopy);
        status = csh_execute(args);
       
        //Freeing memory allocated to args and lines to prevent memory leaks before looping again or exiting
        free(line);
        free(args);
        free(linecopy);
 
        
    } while (status);

    for (int i = 0; i < MAX_HISTORY; i++) {
        free(history[i]);
    }
    
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

    buffer[strcspn(buffer, "\n")] = 0;

    //Return buffer after getline() was used
    return buffer;

}

static char *trim(char *s) {
    while (*s==' ' || *s=='\t') s++;
    char *e = s + strlen(s);
    while (e>s && (e[-1]==' ' || e[-1]=='\t')) --e;
    *e = '\0';
    return s;
}

int csh_run_command_string(const char *cmd) {
    infile = NULL;
    outfile = NULL;

    char *work = strdup(cmd);
    if (!work) { perror("strdup"); return 1; }

    char *save = NULL;
    char *seg  = strtok_r(work, ";", &save);
    while (seg) {
        char *linecopy = strdup(trim(seg));
        if (!linecopy) { perror("strdup"); free(work); return 1; }
        if (*linecopy) {
            add_to_history(linecopy);
            char **args = csh_parse_line(linecopy);
            (void)csh_execute(args);     // sets last_status
            free(args);
        }
        free(linecopy);
        seg = strtok_r(NULL, ";", &save);
        // reset redirections between commands
        infile = NULL; outfile = NULL;
    }

    free(work);
    return last_status;
} 

int csh_run_script_stream(FILE *fp, const char *name) {
    (void)name; // unused for now; could be used for $0 in the future
    char *linebuf = NULL;
    size_t cap = 0;
    int final_status = 0;

    while (getline(&linebuf, &cap, fp) != -1) {
        // Strip trailing newline (your csh_read_line does this)
        linebuf[strcspn(linebuf, "\n")] = 0;
        if (*linebuf == '\0') continue;
        (void)csh_run_command_string(linebuf);
        
        final_status = last_status;
    }
    free(linebuf);
    if (ferror(fp)) {
        perror("read script");
        return 2;
    }
    return final_status;
}

