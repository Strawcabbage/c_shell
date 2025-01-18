// parser.c
#include "shell.h"

//Parsing the read input into a char pointer to an array of Strings
char **csh_parse_line(char *linecopy) {
    
    //Declaring Variables
    char *str;
    size_t position = 0;
    size_t bufsize = 64;
    char **strs;
    char *delims = DELIMS;

    
    //dyamically allocating memory to strs to allow for 64 char pointers
    strs = malloc(bufsize * sizeof(char*));
    
    //Making sure dynamic memory allocation succeeded
    if (strs == NULL) {
        perror("Error allocating memory strs");
        free(strs);
        exit(EXIT_FAILURE);
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
    return strs;
}

struct pipe_command* initialize_commands(char **strs) {
    // Define the number of commands
    int num_commands = 0;
    int index = 0;
    int first_pipe = 0;
    int pipe_index = 0;
    int arg_index = 0;

    int *pipe_indexes = malloc((MAX_PIPES + 1) * sizeof(int));
    if (pipe_indexes == NULL) {
        perror("Failed to allocate memory for pipe_indexes");
        free(pipe_indexes);
    }
    
    // Finding how many times pipes are used
    pipe_indexes[pipe_index++] = -1;
    while (strs[index] != NULL) {
        
        if (strcmp(strs[index], "|") == 0) {

            pipe_indexes[pipe_index++] = index;

            if (first_pipe == 0) {
                num_commands += 2;
                first_pipe++;
            } else {
                num_commands++;
            }   
        }
        index++;
    }
    
    pipe_index = 0;

    // Allocate memory for the array of pipe_command structs
    struct pipe_command *commands = malloc(num_commands * sizeof(struct pipe_command));
    if (commands == NULL) {
        perror("Failed to allocate memory for commands");
        free(commands);
    }
        
    for (int i = 0; i < num_commands; i++) {
        
        arg_index = 0;
        
        commands[i].argv = malloc(MAX_PIPE_ARGUMENTS * sizeof(char *));

        if (commands[i].argv == NULL) {
            perror("Failed to allocate memory for argv");
            for (int j = i - 1; j >= 0; j--) {
                for (int k = 0; commands[j].argv[k] != NULL; k++) {
                    free(commands[j].argv[k]); // Free each string in argv
                }
                free(commands[j].argv); // Free argv array
            }
        }
        

        if (i < num_commands - 1) {

            for (int j = pipe_indexes[pipe_index++] + 1; j < pipe_indexes[pipe_index]; j++) {
                
                commands[i].argv[arg_index] = strdup(strs[j]);

                if (commands[i].argv[arg_index] == NULL) {
                    perror("Failed to allocate memory for string");
                    // Free already allocated strings in argv
                    for (int k = 0; k < arg_index; k++) {
                        free(commands[i].argv[k]);
                    }
                    free(commands[i].argv);
                }

                arg_index++;

            }
        } else {
            
            arg_index = 0;
            
            for (int i = pipe_indexes[pipe_index] + 1; strs[i] != NULL; i++) {

                commands[num_commands - 1].argv[arg_index] = strdup(strs[i]);

                if (commands[num_commands - 1].argv[arg_index] == NULL) {
                    perror("Failed to allocate memory for string");
                    // Free already allocated strings in argv
                    for (int j = 0; j < arg_index; j++) {
                        free(commands[num_commands - 1].argv[j]);
                    }
                    free(commands[num_commands - 1].argv);
                }
                
                arg_index++;

            }

        }
 
    } 

    free(pipe_indexes);

    return commands;
}
