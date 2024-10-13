#define _GNU_SOURCE
#define DELIMS " /\r\a\t\n" 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>


char *csh_read_line(void);
char **csh_parse_line(char*);
void csh_loop();
int csh_execute(char**);
int csh_launch(char**);
int csh_exit();
int csh_echo();

const char *built_in_strs[] = {"exit", "echo"};
char *line;

int (*built_in_func[]) (char**) = {
    &csh_exit,
    &csh_echo
};

int csh_exit() {

    return 0;
    
}

int csh_echo() {
    
    char *first_token;
    char *rest_of_line;
    char *delim = DELIMS;

    first_token = strtok(line, delim);

    rest_of_line = strtok(NULL, "");

    if (rest_of_line != NULL) {
        printf("%s", rest_of_line);
    }

    return 1;

}

int main(int argc, char **argv) {
    
    csh_loop();
    return 0;
}

void csh_loop(void) {
    
    char **args;
    int status;

    do {
    
       printf("> ");

       line = csh_read_line();
       args = csh_parse_line(line);
       status = csh_execute(args);
        
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
    char **strs;
    char *delims = DELIMS;

    char *linecopy = malloc(strlen(line) + 1);

    if (linecopy == NULL) {
        perror("Error allocating memory");
        free(linecopy);
        exit(EXIT_FAILURE);
    }

    strcpy(linecopy, line);

    strs = malloc(bufsize * sizeof(char*));
 
    if (strs == NULL) {
        perror("Error allocating memory");
        free(strs);
        exit(EXIT_FAILURE);
    }
    
    str = strtok(linecopy, delims);

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

int csh_launch(char **args) {
    
    pid_t pid, wpid;
    int status;

    pid = fork();
        
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

int csh_execute(char **args) {
    
    if (args[0] == NULL) {
        return 0;
    }

    for (int i = 0; i < (sizeof(built_in_strs) / sizeof(built_in_strs[0])); i++) {
        if (strcmp(args[0], built_in_strs[i]) == 0) {

            return (*built_in_func[i])(args);
        }
    }
    return csh_launch(args);
}



