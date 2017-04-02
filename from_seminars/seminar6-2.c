#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int fd[2];

void ls() {
    int k = fork();
    if (k < 0) {
        perror(NULL);
        exit(1);
    } else if (k == 0) {
        dup2(fd[1], 1);
        close(fd[0]);
        close(fd[1]);
        execlp("ls", "ls", NULL);
        perror("ls");
        exit(1);
    }
}

void wc() {
    int k = fork();
    if (k < 0) {
        perror(NULL);
        exit(1);
    } else if (k == 0) {
        dup2(fd[0], 0);
        close(fd[0]);
        close(fd[1]);
        execlp("wc", "wc", NULL);
        perror("wc");
        exit(1);
    }
}

int main(int argc, char **argv) {
    int k, n, i;
    pipe(fd);
    ls();
    wc();
    close(fd[0]);
    close(fd[1]);
    wait(&n);
    wait(&n);
}
