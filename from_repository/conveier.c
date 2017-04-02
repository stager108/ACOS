#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int fd[2][2];

void get_first(char* name, int ind) {
    int k = fork();
    if (k < 0) {
        perror(NULL);
        exit(1);
    } else if (k == 0) {
        dup2(fd[ind][1], 1);
        close(fd[0][0]);
        close(fd[0][1]);
        execlp(name, name, NULL);
        perror("getfirst");
        exit(1);
    }
}

void get_function(char* name, int ind1, int ind2) {
    int k = fork();
    if (k < 0) {
        perror(NULL);
        exit(1);
    } else if (k == 0) {
        dup2(fd[ind1][0], 0);
        dup2(fd[ind2][1], 1);
        if(fd[ind1][0] != 0) close(fd[ind1][0]);
        if(fd[ind1][1] != 1)  close(fd[ind1][1]);
        if(fd[ind2][0] != 0)  close(fd[ind2][0]);
        if(fd[ind2][1] != 1) close(fd[ind2][1]);
        execlp(name, name, NULL);
        perror("fail fail fail");
        exit(1);
    }
}
void get_last(char* name, int ind) {
    int k = fork();
    if (k < 0) {
        perror(NULL);
        exit(1);
    } else if (k == 0) {
        dup2(fd[ind][0], 0);
        close(fd[0][0]);
        close(fd[0][1]);
        close(fd[1][0]);
        close(fd[1][1]);
        execlp(name, name, NULL);
        perror("getlast");
        exit(1);
    }
}

int main(int argc, char **argv) {
    int k, n, i;

    pipe(fd[0]);
    get_first(argv[1],0);
    int ind1 = 0;
    int ind2 = 1;
   for(i = 2; i< argc-1; i++){
        pipe(fd[ind2]);
        get_function(argv[i], ind1, ind2);
        close(fd[ind1][0]);
        close(fd[ind1][1]);
        ind1 = (ind1 + 1)%2;
        ind2 = (ind2 + 1)%2;
   }
    get_last(argv[argc - 1],ind1);
    close(fd[0][0]);
    close(fd[0][1]);
    close(fd[1][0]);
    close(fd[1][1]);
    for(i=1; i<argc; i++){
        wait(&n);
    }

}
