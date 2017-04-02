#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <netinet/in.h>

int main()
{   struct sockaddr_un addr;
    addr.sun_family = AF_LOCAL;
  /**/ // addr.sun_port = 8080;
    int s = socket(PF_LOCAL,SOCK_STREAM,IPPROTO_IP);
    strcpy(addr.sun_path, "/tmp/a");
    bind(s, &addr, sizeof(addr));

 //  /**/ if( connect(s, (struct sockaddr *)&addr, sizeof(addr)) < 0)
  //  {
  //     printf("\n Error : Connect Failed \n");
  //     return 1;
  //  }
/**/
    listen(s,10);
    int d = accept(s, NULL, NULL);

    return 0;
}
