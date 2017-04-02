#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <netinet/in.h>
int fd[2];
    int d;
    int s;
    int n;
    int count;
    char** argvs;
int send_buf( int sockfd, int count, char* buf )
{
	ssize_t n;
	int sent = 0;
	send_int(count, sockfd);

	while( sent < count ) {
		n = write( sockfd, buf + sent, count - sent );
		if (n == 0) {
			return -1;
		}
		sent += n;
	}
	return 1;
}

int send_int( int act, int sockfd )
{
	ssize_t n, sent = 0;

	while( sent < sizeof( int )) {
		n = write( sockfd, &act + sent, sizeof( int ) - sent );
		if (n == 0) {
			return -1;
		}
		sent += n;
	}
	return 1;
}

int blocking_read_int( int sockfd, int* ans )
{
	ssize_t n, recv = 0;
	while( recv < sizeof( int )) {
		n = read( sockfd, ans + recv, sizeof( int ) - recv );
		if (n == 0)
			return -1;
		recv += n;
	}

	return 1;
}

int blocking_read_buf( int sock_id, int i)
{
	int count, recv = 0;
	ssize_t n;
	if (blocking_read_int(sock_id, &count) == -1)
		return -1;
    argvs[i] = (char*)malloc(count*sizeof(char));
	while( recv < count ) {
		n = read( sock_id, argvs[i] + recv, count - recv );
		if (n == 0)
			return -1;
		recv += n;
	}
	return 1;
}

void my_func( char** buf){
    int k = fork();
    if (k < 0) {
        perror(NULL);
        exit(1);
    } else if (k == 0) {
        dup2(fd[1], 1);
        close(fd[0]);
        close(fd[1]);
	close(d);
        close(s);
        execvp(buf[0],buf);
        perror(buf);
        exit(1);
    }
}
int main()
{
    struct sockaddr_in addr;
    struct sockaddr_in client;
    int i;
    int n;
    int ans;
    socklen_t addrlen = sizeof(struct sockaddr_in);
    char* buffer = (char*)malloc(1000*sizeof(char));
    //char** argvs;
    char buf[2];

    pipe(fd);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8081);
    addr.sin_addr.s_addr = INADDR_ANY;
    s = socket(PF_INET,SOCK_STREAM,IPPROTO_IP);

    if(s==-1){
        perror("socket failed\n");
        return 1;
    }
    if(bind(s, &addr, sizeof(addr))==-1){
        perror("bind failed\n");
        return 2;
    }
    if(listen(s,10)==-1){
        perror("listen failed\n");
        return 3;
    }

    d = accept(s, (struct sockaddr*) &client, &addrlen);
    if(d==-1){
        perror("accept failed\n");
    }

    if(blocking_read_int(d,&ans)==1){
        argvs = (char**)malloc((ans+1)*sizeof(char*));
        for(i = 0; i < ans; i++){
             blocking_read_buf(d, i);
        }
        argvs[ans] = NULL;
    }
    puts("Data get\n");

    my_func(argvs);
    close(fd[1]);
    n = read(fd[0], buffer, sizeof(buffer));
    while( n!=0){
        if(send_buf(d,n,buffer)==-1){perror("error sending");}
        //printf("%d \n", n);
        n = read(fd[0], buffer, sizeof(buffer));
        if(n == -1){perror("error reading from pipe");}
    }
    send_int(0,s);
    wait(&n);
    puts("Data send away\n");
    close(d);
    close(s);
    close(fd[0]);
    close(fd[1]);
    printf("\n");
    free(buffer);
    free(argvs);
    return 0;
}
