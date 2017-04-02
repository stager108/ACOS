#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netdb.h>
char* buf;
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

int blocking_read_buf( int sock_id)
{
	int count, recv = 0;
	ssize_t n;
	if (blocking_read_int(sock_id, &count) == -1)
		return -1;
    buf = (char*)malloc(count*sizeof(char));
	while( recv < count ) {
		n = read( sock_id, buf + recv, count - recv );
		if (n == 0)
			return -1;
		recv += n;
	}

	return 1;
}

int my_write(char* buffer, int count, int fd)
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
    return 1;
}

int main(int argc, char* argv[]){
    int d;
    int j;
    int i;
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8081);
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    //strcpy(addr.sin_path, "/tmp/a");
    int s = socket(PF_INET,SOCK_STREAM,IPPROTO_IP);
    if(s==-1){
        perror("socket failed");
    }

    if( connect(s, (struct sockaddr *)&addr, sizeof(addr)) >= 0) {
        printf("Connect with server\n");
        //if(blocking_read_buf(s, buf)==-1) printf("fail\n");
        send_int(argc - 1, s);
        for(i = 1; i < argc; i++){
            if(send_buf(s,strlen(argv[i]),argv[i])==-1){perror("error sending");}
        }
        puts("Data Send\n");
        while(blocking_read_buf(s)!=-1) {
            if(my_write(buf,sizeof(buf), 1) == -1){perror("error writing");}
            free(buf);

        }
    }
    else{  printf(" connection failed \n");}
    close(s);


    return 0;
}
