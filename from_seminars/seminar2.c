#include "unistd.h" 
//запускать кучу сыновей из argv!!!!!  
void son(){
    execlp("ls","ls",NULL);

    perror("ls");
    exit(1);

}

int main(){

int k;
k = fork();

if(k==0){
    son();
    wait();
}
else{
    int  k1 = fork();
    if(k1==0){
       son();
    }else{
   wait();
   printf("happy_end\n");
}

}
return 0;
}//   ps gaxu| grep a.out 
