// 6.c — exit(): terminates the process immediately
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int main() {

    printf("Before exit\n");

    exit(0);

    printf("After exit\n");

    return 0;
}