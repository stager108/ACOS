#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

int mywrite(char* buffer, int count, int fd)
{
    int temp = count;
    int written;
    while (temp >0) {
        written = write(fd, buffer, temp);
        if (written == -1) {
            return -1;
        }
        temp -= written;
        buffer += written;
    }
    return count;
}

#define BUF_SIZE 10
int main(int argc, const char * argv[]) {
	int i = 1;
	int param = O_RDONLY;
        int fd, count;
	char* buffer = (char*)malloc(BUF_SIZE*sizeof(char));
	if (argc>1) {
          if (!strcmp(argv[1], "-u")) {
      	      param |= O_SYNC;
              i = 2;
         }
        }
        if(argc > 1){

	    for (; i<argc; ++i) {

  	        if ((fd = open(argv[i], param)) == -1) {
                    fprintf(stderr, "Error opening file descriptor for file %s.\n", argv[i]);
                    exit(1);
                }
                while( (count = read(fd, buffer, BUF_SIZE)) != 0 ) {
                    if (count == -1) {
                        fprintf(stderr, "Error reading from file %s.\n", argv[i]);
                        exit(1);
                    }
                    if (mywrite(buffer, count, 1/*stdout*/) == -1) {
                        fprintf(stderr, "Error writing to file %s.\n", argv[i]);
                        exit(1);
                    }
                }
                if (close(fd)) {
                    fprintf(stderr, "Error closing file descriptor.\n");
                    exit(1);
                }
	    }
        }
        else{
            while( (count = read(0, buffer, BUF_SIZE)) != 0 ) {
                if (count == -1) {
                    fprintf(stderr, "Error reading from file %s.\n", argv[i]);
                    exit(1);
                } 
                if (mywrite(buffer, count, 1/*stdout*/) == -1) {
                    fprintf(stderr, "Error writing to file %s.\n", argv[i]);
                    exit(1);
                }
            }


        }
	free(buffer);
	return 0;
}

