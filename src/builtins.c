#include "shell.h"

int (*built_in_func[]) (char**) = {
    &csh_exit,
    &csh_cd,
    &csh_help
};

//Built in function exit. This function exits the shell
int csh_exit() {

    return 0;
    
}

int csh_cd() {
    
    char *rest_of_line;
    char *curr_dir = strdup(home_dir);

    strtok(line, DELIMS);

    rest_of_line = strtok(NULL, DELIMS);

    if (rest_of_line == NULL) {
        perror("Must have file path after command cd");
        return 1;
    }

    if (!(strcmp(rest_of_line, "..") == 0)) {

        directory_array[directory_count++] = strdup(curr_dir);

        strcat(curr_dir, "/");
        strcat(curr_dir, rest_of_line);
        
        if (chdir(curr_dir) == -1) {
            perror("Error: cannot find directory");
            free(curr_dir);
            return 1;
        } else {
            free(curr_dir);
            return 1;
        }
        
    } else {
        
        if (directory_count > 0) {
            
            if (chdir(directory_array[--directory_count]) == -1) {
                perror("Failed to return to previous directory");
                directory_count++;
                free(curr_dir);
                return 1;
            }

            strcpy(curr_dir, directory_array[directory_count]);
            directory_array[directory_count] = NULL;

        } else {
            perror("Cannot move back directories any further");
            free(curr_dir);
            return 1;
        }

        free(curr_dir);
        return 1;

    }

}

int csh_help() {
    printf("Here are a list of the built in functions you can use:\n");
    for (int i = 0; i < (sizeof(built_in_strs) / sizeof(built_in_strs[0])); i++) {
        printf("  -  \"%s\"\n", built_in_strs[i]);
    }
    return 1;
}

