#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>

int main() {
    char *line = NULL;
    size_t len = 0;
    ssize_t nread;

    printf("Enter a line: ");
    nread = getline(&line, &len, stdin);
    if (nread == -1) {
        perror("getline");
        return 1;
    }

    printf("You entered: %s", line);
    free(line); // Free the allocated memory
    return 0;
}
