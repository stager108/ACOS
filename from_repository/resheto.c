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
pthread_mutex_t mut;
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
    while((next < limit)&&( massiv[next] == 1)){
        next += 1;
    }
    if(next == mas_size){
        next = -2;
    }
    else{
    	if(massiv[next] == 1){
            next = -1;
    	}
        else{
            //printf("next get_next value is %d \n", next+1);
            massiv[next] = 1;
        }
    }
}


void * f( int my_c) {
    int t = 0;
    pthread_mutex_lock(&mut);

    int i = my_c*2 + 1;
    pthread_mutex_unlock(&mut);
  //  printf("hello thread \n");
   // int k = *(int *)p;

    while( i < mas_size) {
       // printf("next unsimple %d\n", i+1);
        pthread_mutex_lock(&mut);
        massiv[i] = 1;
        i+= my_c + 1;
        pthread_mutex_unlock(&mut);

    }
   // printf("next simple ready\n");
   // for(t = 0; t < mas_size; t++){
     // printf("%d a %d b",t, massiv[t]);
    //}
    return NULL;
}

int main(int argc, const char* argv[]) {
    int i;

    int count;
    pthread_mutex_init(&mut, NULL);
    threadsNum = atoi(argv[1]);
    mas_size = atoi(argv[2]);
    massiv = (int*)calloc(sizeof(int), mas_size+10);
    initializing();
    int *k = (int*)malloc(threadsNum * sizeof(int));
    int *newsimple = (int*)malloc(threadsNum * sizeof(int));
    pthread_t *threads;
    limit = 9;
    next = 0;
    while (next!= -2){
        count = 0;
       // printf("hello while pthread\n");
        threads = (pthread_t *) malloc(sizeof(pthread_t) * threadsNum);
        //printf("hello 12345\n");
        for( i = 0; i < threadsNum; ++i ) {
            get_next();
            if(( next != -1)&&(next != -2)){
                pthread_mutex_lock(&mut);
                newsimple[count] = next;
                count++;
                pthread_mutex_unlock(&mut);
            }
            else{
                if(next == -1){
                   // printf("limit %d\n", limit);
                    pthread_mutex_lock(&mut);
                    next = limit - 1;
               	    limit = limit*limit;
               	    if(limit > mas_size){
                	    limit = mas_size;
                    }
                    pthread_mutex_unlock(&mut);
                    break;
                   // printf("limit %d\n", limit);
                }
                else break;
            }
        }
        for(i = 0; i < threadsNum; ++i ) {
            k[i] = i;
        }
        for( i = 0; i < count; ++i ) {
           printf("%d ", newsimple[i]+1);
           pthread_create(threads + i, NULL, f,newsimple[i]);
        }
        void *p;
        i = 0;
        while(i < count) {
            pthread_join(threads[i], &p);
            i++;
        }
        free(threads);
    }
    free(massiv);
    free(k);
    free(newsimple);
    printf("There is all!\n");
    return 0;
}

