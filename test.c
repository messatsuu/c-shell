#include <stdio.h>
#include <string.h>

int main() {
    char input[] = "this is a test";
    int input_size = strlen(input);

    int i = 0;
    int number_of_spaces = 0;
    for (int i = 0; input[i]; i++) number_of_spaces += (input[i] == ' ');

    printf("%d\n", number_of_spaces);
}
