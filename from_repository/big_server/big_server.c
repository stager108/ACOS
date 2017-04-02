#include <time.h>
#include <limits.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <assert.h>
#include <sys/wait.h>
#include <stdio.h>
#include <netdb.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <err.h>

const int false = 0;
const int true = 1;
const char HEADER[] = "HTTP/1.0 %s\r\n" "Server: AUP-ws\r\n""Content-Length: %ld\r\n";
const char FILE_TEXT[] = "Content-Type: text/html\r\n\r\n";
const char FILE_JPEG[] = "Content-Type: image/jpeg\r\n\r\n";
const char DEFAULT_DOC[] = "index.html";
//const char WEB_ROOT[] = "Home/YakushevaSof_yaFedorovna/big_server/static";
//const char WEB_ROOT[] = "static";
char* WEB_ROOT;
const char *HTML_NOTFOUND =
        "<!DOCTYPE html PUBLIC \"-//IETF//DTD HTML 2.0//EN\">\n"
        "<html><head><title>Error 404</title>\n"
        "</head><body>\n"
        "<h2>Our wonderful server can't find document</h2>"
        "</body></html>\r\n";
/*[send_header]*/
#define MY_S_NAME_SIZE 200
typedef struct {
    int is_server; /* server? (vs. client) */
    int mydomain; /* AF_INET or AF_UNIX */
    int fd; /* socket fd */
    char server_name[MY_S_NAME_SIZE]; /* server name */
    fd_set fdset;  /* set for server's select */
    int num_of_fd;/*descriptor*/
} Server;






ssize_t myBigWrite(int fd, const void *buf, size_t nbyte) {/*write the buffer to the descriptor fd*/
    ssize_t nwritten = 0, n;

    do {
        if ((n = write(fd, buf + nwritten, nbyte - nwritten)) == -1) {
            if (errno != EINTR)
                return -1;
        }
        nwritten += n;
    } while (nwritten < nbyte);
    return nwritten;
}
void write_files(int fd,const char* namedir){

    int n;
    int i = 0;
    int j = 0;
    int jj = 0;
    struct dirent *namelist = NULL;
    struct dirent current;
    struct stat buf;
    char* string;
    char* filename = (char*)malloc(1000);
    char* filename1 = (char*)malloc(1000);
    strcat(filename1,        "<!DOCTYPE html PUBLIC \"-//IETF//DTD HTML 2.0//EN\">\n"
        "<html><head><title>Error 404</title>\n"
        "</head><body>\n");
    uid_t id;
    DIR* dir = NULL;
    dir = opendir(namedir);

    if (dir == NULL){perror("opendir");exit;}
    else{
        printf("%s \n", namedir);
        while(1)
        {
            namelist = NULL;
            n = readdir_r(dir, &current, &namelist);
            if(namelist == NULL){break;}
            else{
                if((strcmp(current.d_name,".")!=0)&&(strcmp(current.d_name,"..")!=0)){
                strcpy(filename, namedir);
                strcat(filename, "/");
                strcat(filename, current.d_name);
                  // if (stat(current.d_name, &buf) == -1) {
                if (stat(filename, &buf) == -1) {
                    //  printf("%s \n", current.d_name);
                      perror("stat");
                      exit;
                   }
                   else
                   {    strcat(filename1, "<h2> a href=");
                        strcat(filename1, current.d_name);
                        strcat(filename1, "/<h2>");
                    }
                }

             }
        }
        strcat(filename1,  "</body></html>\r\n");
        int e = myBigWrite(fd, filename1, strlen(filename1));
    n = closedir(dir);
    }
    free(filename);
}



ssize_t myBigRead(int fd, void *buf, size_t nbyte) {/*read the buffer from the descriptor fd*/
    ssize_t nread = 0, n;

    do {
        if ((n = read(fd, &((char *) buf)[nread], nbyte - nread)) == -1) {
            if (errno != EINTR)
                return -1;
        }
        if (n == 0)
            return nread;
        nread += n;
    } while (nread < nbyte);
    return nread;
}

void set_fd_hwm(Server *server, int fd) {
    if (fd > server->num_of_fd)
        server->num_of_fd = fd;
}

void reset_fd_hwm(Server *server, int fd) {
    if (fd == server->num_of_fd)
        server->num_of_fd--;
}

int make_sockaddr(struct sockaddr *sa, socklen_t *len, const char *name, int domain, int will_bind) {
    struct addrinfo *infop = NULL;
    if (domain == AF_UNIX) {
        struct sockaddr_un *sunp = (struct sockaddr_un *) sa;

        if (strlen(name) >= sizeof(sunp->sun_path)) {
            errno = ENAMETOOLONG;
        }
        strcpy(sunp->sun_path, name);
        sunp->sun_family = AF_UNIX;
        *len = sizeof(*sunp);
    } else {
        struct addrinfo hint;
        char nodename[MY_S_NAME_SIZE], *servicename;

        memset(&hint, 0, sizeof(hint));
        hint.ai_family = domain;
        hint.ai_socktype = SOCK_STREAM;
        if (will_bind)
            hint.ai_flags = AI_PASSIVE;
        strcpy(nodename, name);
        servicename = strchr(nodename, ':');
        if (servicename == NULL) {
            errno = EINVAL;
        }
        *servicename = '\0';
        servicename++;
        if (getaddrinfo(nodename, servicename, &hint, &infop) == 0) { exit; };
        memcpy(sa, infop->ai_addr, infop->ai_addrlen);
        *len = infop->ai_addrlen;
        freeaddrinfo(infop);
    }
    return true;

    if (infop != NULL)
        freeaddrinfo(infop);
    return false;

}

Server *mySerOpen(const char *server_name, int server)//open a server by name
{
    Server *myserstr = NULL;
    struct sockaddr_storage sa;
    socklen_t sa_len;

    if (!((myserstr = (Server *) calloc(1, sizeof(Server)))) == 0) {
        exit;
    }
    myserstr->is_server = server;
    if (strncmp(server_name, "//", 2) == 0) {
        myserstr->mydomain = AF_INET;
        server_name += 2;
    }
    else {
        myserstr->mydomain = AF_UNIX;
        if (myserstr->is_server)
            (void) unlink(server_name);
    }
    if (strlen(server_name) >= sizeof(myserstr->server_name)) {
        errno = ENAMETOOLONG;
    }
    strcpy(myserstr->server_name, server_name);
    if ((make_sockaddr((struct sockaddr *) &sa, &sa_len, myserstr->server_name, myserstr->mydomain,
                       myserstr->is_server)) == 0) {
        err(1, "Sockaddr failed");
    }
    if ((myserstr->fd = socket(myserstr->mydomain, SOCK_STREAM, 0)) == -1) {
        err(2, "Socket failed");
    }
    set_fd_hwm(myserstr, myserstr->fd);
    if (myserstr->mydomain == AF_INET) {

        int sockoptval = 1;
        if ((setsockopt(myserstr->fd, SOL_SOCKET, SO_REUSEADDR, &sockoptval, sizeof(sockoptval))) == -1) {
            err(4, "Sockopt failed");
        }
    }
    if (myserstr->is_server) {
        FD_ZERO(&myserstr->fdset);
        if ((bind(myserstr->fd, (struct sockaddr *) &sa, sa_len)) == -1) {
            err(5, "Bind failed");
        }
        if ((listen(myserstr->fd, SOMAXCONN)) == -1) {
            err(6, "Listen failed");
        }
        FD_SET(myserstr->fd, &myserstr->fdset);
    }
    else if ((connect(myserstr->fd, (struct sockaddr *) &sa, sa_len)) == -1) {
        err(7, "Connect failed");
    }
    return myserstr;
}

int mySerClose(Server *server) {
    if (server->is_server) {
        int fd;

        for (fd = 0; fd <= server->num_of_fd; fd++)
            if (FD_ISSET(fd, &server->fdset))
                close(fd);
        if (server->mydomain == AF_UNIX)
            unlink(server->server_name);
    }
    else
        close(server->fd);
    free(server);
    return true;
}

int mySerClose_fd(Server *myserstr, int fd) {
    if ((close(fd)) == -1) {
        err(8, "Close failed");
    };
    FD_CLR(fd, &myserstr->fdset);
    reset_fd_hwm(myserstr, fd);
    return true;
}

int mySerWaitSer(Server *server) {
    if (server->is_server) {
        fd_set fd_set_read;
        int fd, clientfd;
        struct sockaddr_un from;
        socklen_t from_len = sizeof(from);
        while (true) {
            fd_set_read = server->fdset;
            if ((select(server->num_of_fd + 1, &fd_set_read, NULL, NULL, NULL)) == -1) {
                err(9, "Select failed");
            }
            for (fd = 0; fd <= server->num_of_fd; fd++) {
                if (FD_ISSET(fd, &fd_set_read)) {
                    if (fd == server->fd) {
                        if ((clientfd = accept(server->fd, (struct sockaddr *) &from, &from_len)) == -1)
                            err(11, "Accept failed");
                        FD_SET(clientfd, &server->fdset);
                        set_fd_hwm(server, clientfd);
                        continue;
                    }
                    else
                        return fd;
                }
            }

        }
    } else {
        errx(3, "Unreachable code");
    }

}

int my_s_get_server_fd(Server *myserstr) {
    return myserstr->fd;
}

void send_header(Server *ssip, const char *message, off_t len, const char *path, int fd) {
    char buffer[1000], *dot;

    snprintf(buffer, sizeof(buffer), HEADER, message, (long) len);
    if ((myBigWrite(fd, buffer, strlen(buffer))) == -1) {
        err(10, "MyBigWrite failed");
    }
    dot = strrchr(path, '.');
    if (dot != NULL && (strcasecmp(dot, ".jpg") == 0 || strcasecmp(dot, ".jpeg") == 0)) {
        if ((myBigWrite(fd, FILE_JPEG, strlen(FILE_JPEG))) == -1) {
            err(10, "MyBigWrite failed");
        }
    }
    else if ((myBigWrite(fd, FILE_TEXT, strlen(FILE_TEXT))) == -1) {
        exit;
    }
    return;
}


int handle_request(Server *myserstr, char *s, int fd) {
    char *token, buffer[1000], path[500], path1[500];
    int ntoken;
    FILE *in;

    struct stat statbuffer;
    ssize_t nread;
    printf("Request: %s\n\n", s);
    for (ntoken = 1; (token = strtok(s, " ")) != NULL; ntoken++) {
        s = NULL;
        switch (ntoken) {
            case 1:
                if (strcasecmp(token, "get") != 0) {
                    printf("Unknown request\n");
                    return false;
                }
                continue;
            case 2:
                break;
        }
        printf("Token: %s\n", token);
        break;
    }
    snprintf(path, sizeof(path) - 1 - strlen(DEFAULT_DOC), "%s%s", WEB_ROOT, token);
    //strcpy(path,token);
    //printf("PATH: %s",path);
    /* dot = strrchr(path, '/');
    strncpy(path1, token, dot+1 );
    dir = opendir(path1);
    if (dir == NULL){
        perror("opendir:");
        printf("%s \n", path1);

        send_header(myserstr, "404 Not Found", strlen(HTML_NOTFOUND), "", fd);
        if ((myBigWrite(fd, HTML_NOTFOUND, strlen(HTML_NOTFOUND))) == -1) {
            err(10, "MyBigWrite failed");
        }
        return false;
    }
    //strcpy(path, token);*/
    if (stat(path, &statbuffer) == 0 && S_ISDIR(statbuffer.st_mode)) {
        if (path[strlen(path) - 1] != '/')
            strcat(path, "/");
        strcat(path, DEFAULT_DOC);
    }

    printf("File path: %s\n\n", path);
    if (stat(path, &statbuffer) == -1 || (in = fopen(path, "rb")) == NULL) {
        send_header(myserstr, "404 Not Found", strlen(HTML_NOTFOUND), "", fd);
        write_files(fd,WEB_ROOT);
        if ((myBigWrite(fd, HTML_NOTFOUND, strlen(HTML_NOTFOUND))) == -1) {
            err(10, "MyBigWrite failed");

        }
        return false;
    }
    send_header(myserstr, "200 OK", statbuffer.st_size, path, fd);
    while ((nread = fread(buffer, 1, sizeof(buffer), in)) > 0)
        if ((myBigWrite(fd, buffer, nread)) == -1) {
            err(10, "MyBigWrite failed");
        }
    if (fclose(in) == EOF) {
        err(12, "Fclose failed");
    }
    return true;
}

#define PORT ":8080"

int main(int argc, char* argv[]) {
    Server *server = NULL;
    char msg[1600];
    WEB_ROOT = argv[1];
    ssize_t nrcv;
    int fd;
    char host[100] = "//";

    if ((gethostname(&host[2], sizeof(host) - 2 - strlen(PORT))) == -1) {
        exit;
    }
    strcat(host, PORT);
    printf("Connecting to host \"%s\"\n", host);
    if ((server = mySerOpen(host, true)) == 0) { exit; };
    printf("\t...connected\n");
    while (true) {
        if ((fd = mySerWaitSer(server)) == -1) { exit; };
        switch (nrcv = read(fd, msg, sizeof(msg) - 1)) {
            case -1:
                fprintf(stderr, "Read error (nonfatal)\n");
            case 0:
                if ((mySerClose_fd(server, fd)) == false) { exit; };
                continue;
            default:
                msg[nrcv] = '\0';
                handle_request(server, msg, fd);
        }
    }
    if ((mySerClose(server)) == false) { exit; };
    printf("Done.\n");
    exit(EXIT_SUCCESS);

}

