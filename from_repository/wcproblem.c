#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#define BUF_SIZE 1

void Conclusion(int fd, int* strings, int* words, int* symbols ){
    char previous = EOF;
    int newsymbol;
    int count;
    int* buffer = (int*)malloc(BUF_SIZE*sizeof(int));
    while( (count = read(fd, buffer, BUF_SIZE)) != 0 ) {
        if (count == -1) {
            fprintf(stderr, "Error reading from file \n");
            exit(1);
        }
        else{
            newsymbol = buffer[0];
            if(isspace(newsymbol)){
                if(isgraph(previous)){
                    (*words)++;
                }
                if(newsymbol == '\n'){
                    (*strings)++;
                }
            }
        }
        previous = newsymbol;
        (*symbols)++;
    }
    if(isgraph(previous)){
        (*words)++;
    }
	free(buffer);
}

int main(int argc, const char * argv[]) {
	int i = 1;
	int w = 0;
	int st = 0;
	int sym = 0;
    int words = 0;
	int strings = 0;
	int symbols = 0;
	int *ss = &strings;
    int *ww = &words;
    int *sb = &symbols;
	int param = O_RDONLY;
	int fd;

	if(argc > 1){
        for (i = 1; i < argc; ++i) {
            if ((fd = open(argv[i], param)) == -1) {
                fprintf(stderr, "Error opening file descriptor for file %s.\n", argv[i]);
                exit(1);
            }
            words = 0;
	        strings = 0;
	        symbols = 0;
            Conclusion(fd,ss,ww,sb);
            w +=words;
            st+=strings;
            sym+=symbols;
            if (close(fd)) {
                fprintf(stderr, "Error closing file descriptor.\n");
                exit(1);
            }
            else{
                fprintf(stdout, "%d %d %d %s\n",strings,words,symbols, argv[i]);
            }
        }
        if(argc>2){fprintf(stdout, "total: %d %d %d\n",st,w,sym);}

    }
    else{
        Conclusion(0,ss,ww,sb);
        fprintf(stdout, "\n %d %d %d \n",strings,words,symbols);
    }


	return 0;
}
