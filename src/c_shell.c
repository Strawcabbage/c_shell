#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>


char *csh_read_line(void);
char **csh_parse_line(char*);
void csh_loop();
int csh_exec(char**);
int csh_launch(char**);

int main(int argc, char **argv) {
    
    csh_loop();
    return 0;
}

void csh_loop(void) {
    
    char *line;
    char **args;
    int status;

    do {
    
       printf("> ");

       line = csh_read_line();
       args = csh_parse_line(line);
       status = csh_exec(args);
        
       free(line);
       free(args);

    } while (status);
    
}

char *csh_read_line(void) {

    size_t bufsize = 0;
    char *buffer = NULL;

    if (getline(&buffer, &bufsize, stdin) == -1) {
        if (feof(stdin)) {
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
            strs = realloc(strs, bufsize * sizeof(char*));

            if (strs == NULL) {
               perror("Error allocating memory");
               free(strs);
               exit(EXIT_FAILURE);
            }
        }
        
        str = strtok(NULL, delims);
        position++;

    }
    
    strs[position] = NULL;
    return strs;
}

int csh_exec(char **args) {
    
    const char *built_in[] = {"cd", "echo"};
    int built_in_exe = 0;

    for (int i = 0; i < (sizeof(built_in) / sizeof(built_in[0])); i++) {
        if (strcmp(args[0], built_in[i]) == 0) {
            built_in_exe = 1;
        }
    }

    if (built_in_exe) {
        return csh_launch(args);
        
    } else {
        pid_t pid = fork();
        
        if (pid<0) {
            perror("Fork fail");
            exit(EXIT_FAILURE);
            return 0;
        }
        if (!pid) {
            int status_code = execvp(args[0], args);
            if (status_code == -1) {
                perror("Process did not execute");
                exit(EXIT_FAILURE);
                return 0;
            }
            return 1;
        } else {
            exit(EXIT_SUCCESS);
            return 1;
        }
    }

}

int csh_launch(char **args) {
    printf("You're not supposed to be here");
    return 0;
}
