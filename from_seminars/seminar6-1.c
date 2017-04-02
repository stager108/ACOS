#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void son(char *p) {
    int k = fork();
    if (k < 0) {
        perror(NULL);
        exit(1);
    } else if (k == 0) {
        execlp(p, p, NULL);
        perror(p);
        exit(1);
    }
}

int main(int argc, char **argv) {
    int k, n, i;
    for (i = 1; i < argc; ++i) {
        son(argv[i]);
    }
    for (i = 1; i < argc; ++i) {
        wait(&n);
    }
}
