#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {

    // Convert the arguments to integers
    int num1 = atoi(argv[1]);

    // Add the numbers
    int sum = num1 * 2;

    // Output the result
    printf("The sum of %d and 2 is %d\n", num1, sum);

    return 0;
}
