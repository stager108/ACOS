#include <pthread.h>
#include <stdio.h>
#include <memory.h>
#include <stdlib.h>
int a = 0;
int threadsNum;
pthread_mutex_t mut;
int* massiv;
int mas_size;
int limit;
int next;

void initializing(){
//make some marks by hands
    massiv[0] = 1; //1
    massiv[3] = 1; //4
    massiv[5] = 1; //6
    massiv[7] = 1; //8
    massiv[8] = 1; //9
    massiv[9] = 1; //10
}
void get_next(){
    while((next < limit)&&( massiv[next] == 1))
        next += 1;
    if(massiv[next] == 1){
        next = -1;
    }
    else{
        printf("%d ", next+1);
    }
}


void * f( int my_c) {
    int t = 0;
    int i = my_c*2 + 1;
   // int k = *(int *)p;
    int temp = 0;
    while( i < mas_size) {
        massiv[i] = 1;
        i+= my_c + 1;

    }
        pthread_mutex_lock(&mut);
        a+=t;
        pthread_mutex_unlock(&mut);
    return NULL;
}

int main(int argc, const char* argv[]) {
    int i;
    int count;
    pthread_mutex_init(&mut, NULL);
    threadsNum = atoi(argv[1]);
    mas_size = atoi(argv[2]) + 10;
    massiv = (int*)calloc(sizeof(int), mas_size);

    initializing();

    int *k = (int*)malloc(threadsNum* sizeof(int));

    for(i = 0; i < threadsNum; ++i ) {
        k[i] = i;
    }
    limit = 9;
    next = 0;
    while (next !=-1){
        count = 0;
        pthread_t *threads = (pthread_t *) malloc(sizeof(pthread_t) * threadsNum);
        for( i = 0; i < threadsNum; ++i ) {
            get_next();
            if( next != -1){
                pthread_create(threads + i, NULL, f(next), k + i);
                count++;
            }
            else{

                limit = limit*limit;
                if(limit > mas_size - 10){
                    limit = mas_size - 10;
                }
    }
                break;
            }
        }
        void *p;
        for( i = 0; i < count; ++i ) {
            pthread_join(threads[i], &p);
        }
    }
    printf("%d\n", a);
    return 0;
}

