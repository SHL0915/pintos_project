#include <stdio.h>
#include <string.h>
#include <syscall.h>

int
main (int argc, char **argv)
{   
    
    int argument[4];
    for(int i = 0; i < 4; i++) {
        int now = 0, len = strlen(argv[i + 1]);
        for(int j = 0; j < len; j++) {
            char c = argv[i + 1][j];
            now = now * 10;
            now += (int)(c - '0');
        }
        argument[i] = now;
    }

    printf("%d %d\n", fibonacci(argument[0]), max_of_four_int(argument[0], argument[1], argument[2], argument[3]));    

    return EXIT_SUCCESS;
}
