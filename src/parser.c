#include "shell.h"

//Parsing the read input into a char pointer to an array of Strings
char **csh_parse_line(char *line, char *linecopy) {
    
    //Declaring Variables
    char *str;
    size_t position = 0;
    size_t bufsize = 64;
    char **strs;
    char *delims = DELIMS;
    //char *command[10];
    total_cmds = 0;
    //int pipe = 0;

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
    /*
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
    
    printf("%s\n", cmd[0].argv[0]);*/
    /*printf("%s\n", cmd[total_cmds - 2].argv[1]);
    printf("%s\n", cmd[total_cmds - 1].argv[0]);
    printf("%s\n", cmd[total_cmds - 1].argv[1]);*/
    cmd[total_cmds].argv = NULL;

    return strs;
}

