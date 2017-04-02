#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <netinet/in.h>

int main()
{   struct sockaddr_un addr;
    addr.sun_family = AF_LOCAL;
   // addr.sun_port = htons(8080);
    int s = socket(PF_LOCAL,SOCK_STREAM,IPPROTO_IP);
    strcpy(addr.sun_path, "/tmp/a");
    connect(s, &addr, sizeof(addr));

    return 0;
}
