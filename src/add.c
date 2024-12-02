#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    // Check if the correct number of arguments is provided
    if (argc != 3) {
        printf("Usage: %s <number1> <number2>\n", argv[0]);
        return 1;
    }

    // Convert the arguments to integers
    int num1 = atoi(argv[1]);
    int num2 = atoi(argv[2]);

    // Add the numbers
    int sum = num1 + num2;

    return sum;
}
