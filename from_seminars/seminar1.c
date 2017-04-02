#include "unistd.h" 
int main(){

int k;
k = fork();
if(k < 0){
   perror(" ");
   exit(1);

}
if(k==0){

    printf("s: pid = %d ppid = %d\n", getpid(), getppid());

    kill(getppid(), 9);
}
else{
    printf("s: pid = %d ppid = %d\n", getpid(), getppid());
    sleep(100);
}
return 0;
}//   ps gaxu| grep a.out 

