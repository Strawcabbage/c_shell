#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>


char *csh_read_line(void);
char **csh_parse_line(char*);


int main(int argc, char **argv) {
    
    csh_loop();

}

int csh_loop(void) {
    
    char *line;
    char **args;

    line = csh_read_line();
    args = csh_parse_line(line);
    csh_exec(args);

}

char *csh_read_line(void) {

    size_t bufsize = 0;
    char *buffer = NULL;
    

    if (getline(&buffer, &bufsize, stdin) == -1) {
        if (feof(&buffer)) {
            exit(EXIT_SUCCESS);
        } else {
            perror("Error reading line");
            exit(EXIT_FAILURE);
        }
    }
    
    return buffer;

}

char **csh_parse_line(char *line) {
    
    char *str;
    size_t position = 0;
    size_t bufsize = 64;
    char **strs = malloc(bufsize * sizeof(char*));
    char *delims = " /\r\a\t\n";
    

    if (strs == NULL) {
        perror("Error allocating memory");
        free(strs);
        exit(EXIT_FAILURE);
    }
    
    str = strtok(line, delims);

    while (str != NULL) {
        
        strs[position] = str;

        if (position >= bufsize) {
            bufsize *= 2;
            strs = realloc(strs, bufsize * sizeOf(char*));

            if (strs != NULL) {
               perror("Error allocating memory");
            free(strs);
            exit(EXIT_FAILURE);
            }
        }

        str = strtok(NULL, delims);
        position++;

    }

    return strs;
}

void csh_exec(char **args) {
    
}


