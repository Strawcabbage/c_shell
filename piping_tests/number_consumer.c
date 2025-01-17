#include <stdio.h>
#include <stdlib.h>

int main() {
    char buffer[256];
    int sum = 0;
    int num;

    // Read each line from stdin
    while (fgets(buffer, sizeof(buffer), stdin) != NULL) {
        // Convert the string to an integer and add to sum
        num = atoi(buffer);
        sum += num;
    }

    printf("Sum: %d\n", sum); // Output the sum
    return 0;
}

