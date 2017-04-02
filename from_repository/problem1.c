
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int cmp(const void* a, const void* b){//char**
    return strcmp(*(char **)a,*(char**)b);
}

char* resize_buffer(char* buffer, int size){
    char* new_buffer;
    new_buffer = realloc(buffer, sizeof(char) * 2 * size);
  //  free(buffer);
    return new_buffer;
}

char** resize_buffer_1(char** buffer, int size){
    char** new_buffer;
    new_buffer = realloc(buffer, sizeof(char*) * 2 * size);
  //  free(buffer);
    return new_buffer;
}

int main(){
    char** lexicon = (char**)malloc(10*sizeof(char*));
    char* text = (char*)malloc(10*sizeof(char));
    char* reserved = (char*)malloc(2*sizeof(char));
    int newsymbol, next; 
    size_t size = 0;
    size_t words = 0;
    int sizeoflexicon = 10;
    int sizeoftext = 10;
    int i = 0;
    int flag = 0;
    int isEnd = 0, Need = 0, Res = 0;
    newsymbol = getchar();

    while(newsymbol != EOF){
        switch(newsymbol){
        case '&': case'|': 
            isEnd = 1;
            Res = 1;
            reserved[0] = newsymbol;
            next = getchar();
            if(next == newsymbol){
                reserved[1] = next;
                Need = 1;
            }
            else{
                newsymbol = next;
                Need = 0;            
            }
            break;
        case '\'':case'"':
            next = newsymbol;
            newsymbol = getchar();
            while((next != newsymbol)&&(newsymbol != EOF)){

                text[size] = newsymbol; 
                size++;
                if (size == sizeoftext){
                    text = resize_buffer(text, sizeoftext);
                    sizeoftext *= 2;
                }
                newsymbol = getchar();
            }
            if(newsymbol == EOF) {flag = 1;}
            Need = 1;
            break;
        case ';': isEnd = 1; Need = 1; Res = 1; reserved[0] = ';'; break;
        default:
            if(isspace(newsymbol)){ isEnd = 1; Need = 1; Res = 0; break;}
            else{
                text[size] = newsymbol; 
                size++;
                if (size == sizeoftext){
                    text = resize_buffer(text, sizeoftext);
                    sizeoftext *= 2;
                }
            Need = 1;
            break;
           }
        }

        if((isEnd == 1)&&(size >0)){
            lexicon[words] = text;
            words++;
            if (words == sizeoflexicon){
                lexicon = resize_buffer_1(lexicon, sizeoflexicon);
                sizeoflexicon *= 2;
            }
            text = (char*)malloc(100*sizeof(char));
            size = 0;
            sizeoftext = 10;
            isEnd = 0;
        }
        if(Res == 1){
            lexicon[words] = reserved;
            words++;
            if (words == sizeoflexicon){
                lexicon = resize_buffer_1(lexicon, sizeoflexicon);
                sizeoflexicon *= 2;
            }
            reserved = (char*)malloc(2*sizeof(char));
            isEnd = 0;
            Res = 0;
         }
        if(Need == 1){newsymbol = getchar(); Need = 0; }
    }
    if(size > 0){lexicon[words] = text; words++; text = NULL;}
    qsort(lexicon, words, (sizeof(char**)), cmp);
    printf("\n");
    if(flag ==0){
        for(i = 0; i < words; i++){
            printf("\"%s\"\n", lexicon[i]);
            free(lexicon[i]);
        }
    }
    else{  printf("%s\n", "error");}
    free(lexicon);
    return 0;
}
