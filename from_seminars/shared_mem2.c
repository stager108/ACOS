#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>
#include <memory.h>
#include <stdlib.h>
#include <malloc.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>

int main(int argc, char ** argv){
	int fd = shm_open("mem", O_RDWR, 0600);
	if (fd == -1){
		perror("shm_open");
		exit(EXIT_FAILURE);
	} 
	char* mem = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);	
	if (mem == MAP_FAILED) {
		perror("mmap");
		close(fd);
		exit(EXIT_FAILURE);
	} 
	puts(mem);
	if (munmap(mem, 4096) == -1) {
		perror("unmmap");
		close(fd);
		exit(EXIT_FAILURE);
	} 
	shm_unlink("mem");
}
