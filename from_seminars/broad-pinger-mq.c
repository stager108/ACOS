#define _GNU_SOURCE
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <mqueue.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/epoll.h>

#define MSG_CNT 20
#define BUF_SZ 4096
#define QUEUE_NAME "/mq-broad-ping"

const char** create_servers_list() {
    static const char* servers[] = {
        "vk.com",
        "ya.ru",
        "yandex.ru",
        "mipt.ru",
        "google.ru",
        "yandex.com",
        "yandex.com.tr",
        "diht.mipt.ru",
        "localhost",
        "8.8.8.8",
        "lurkmore.co",
        NULL
    };
    return servers;
}

size_t get_list_size(const char** list) {
    size_t length = 0;
    while(list[length])
        ++length;
    return length;
}

typedef struct {
    int error_code;
    int received;
    char ipaddr[];
} ping_result_t;

size_t ping_result_length(ping_result_t* obj) {
    return sizeof(obj->error_code) + sizeof(obj->received) + strlen(obj->ipaddr) + 1;
}

ping_result_t* create_ping_result(int error_code, int received, const char* ipaddr, size_t ipaddr_len) {
    ping_result_t* retval = malloc(sizeof(error_code) + sizeof(received) + ipaddr_len + 1);
    if (retval) {
        retval->error_code = error_code;
        retval->received = received;
        strncpy(retval->ipaddr, ipaddr, ipaddr_len)[ipaddr_len] = 0;
    }
    return retval;
}

ping_result_t* parse_ping_output(const char* outdata) {
    const char* p_received = strstr(outdata, " received,");
    if (p_received == NULL)
        return create_ping_result(1, -1, "", 0);
    for (--p_received; p_received >= outdata && *p_received != ' '; --p_received)
        /*reverse scan*/;
    ++p_received;
    char* received_end;
    int received = strtol(p_received, &received_end, 10);
    if (received_end <= p_received)
        return create_ping_result(2, received, "", 0);
    const char* l_bracket_pos = strchr(outdata, '(');
    if (l_bracket_pos == NULL)
        return create_ping_result(3, received, "", 0);
    const char* r_bracket_pos = strchr(l_bracket_pos + 1, ')');
    if (r_bracket_pos == NULL)
        return create_ping_result(4, received, "", 0);
    return create_ping_result(0, received, l_bracket_pos + 1, r_bracket_pos - l_bracket_pos - 1);
}

char* read_all_data(int fd) {
    char* buffer = malloc(BUF_SZ);
    assert(buffer);
    int read_size = 0, alloc_size = BUF_SZ;
    int read_cnt;
    while (read_cnt = read(fd, buffer + read_size, alloc_size - read_size)) {
        if (read_cnt < 0) {
            perror("can't read from pipe:");
            exit(1);
        }
        read_size += read_cnt;
        if (alloc_size == read_size) {
            char* tmpbuf = realloc(buffer, alloc_size * 2);
            assert(tmpbuf);
            buffer = tmpbuf;
            alloc_size *= 2;
        }
    }
    buffer[read_size] = 0;
    return buffer;
}

void child_proc(const char* server, mqd_t mqdes, unsigned id) {
    pid_t child;
    int fds[2];

    assert_perror(pipe(fds));
    assert((child = fork()) >= 0);
    if (child == 0) {
        close(fds[0]);
        assert(1 == dup2(fds[1], 1));
        execl("/bin/ping", "ping", "-c", "1", "-W", "10", server, NULL);
        exit(1);
    } else {
        close(fds[1]);
        char* buffer = read_all_data(fds[0]);
        assert(wait(NULL) != -1);
        close(fds[0]);
        
        ping_result_t* pingres = parse_ping_output(buffer);
        assert(pingres);
        free(buffer);

        assert_perror(mq_send(mqdes, (char*)pingres, ping_result_length(pingres), id));
        free(pingres);
        mq_close(mqdes);
    }
}

mqd_t init_queue() {
    mqd_t mqdes = mq_open(QUEUE_NAME, O_CREAT | O_RDWR, 0777, NULL);
    if (mqdes == (mqd_t)-1) {
        perror("can't open message queue:");
        exit(1);
    }
    struct mq_attr attributes;
    assert_perror(mq_getattr(mqdes, &attributes));
    printf("mq attributes: flags=%ld; maxmsg=%ld; msgsize=%ld; curmsgs=%ld\n", 
        attributes.mq_flags, attributes.mq_maxmsg, attributes.mq_msgsize, attributes.mq_curmsgs
    );
    if (attributes.mq_maxmsg < MSG_CNT)
        attributes.mq_maxmsg = MSG_CNT;
    if (attributes.mq_msgsize < BUF_SZ)
        attributes.mq_msgsize = BUF_SZ;
    assert_perror(mq_setattr(mqdes, &attributes, NULL));
    return mqdes;
}

long mq_getmsgsize(mqd_t mqdes) {
    struct mq_attr attributes;
    assert_perror(mq_getattr(mqdes, &attributes));
    return attributes.mq_msgsize;
}

void parent_proc(mqd_t mqdes, const char** servers, size_t child_count) {
    size_t child_proceed;
    long msg_size = mq_getmsgsize(mqdes);
    char* buffer = malloc(msg_size);
    assert(buffer);
    for (child_proceed = 0; child_proceed < child_count;) {
        unsigned id;
        ssize_t read_cnt = mq_receive(mqdes, buffer, msg_size, &id);
        if (read_cnt == -1) {
            perror("can't read from message queue");
            exit(1);
        }
        ping_result_t* pingres = (ping_result_t*)buffer;
        if (pingres->error_code) 
            printf("can't parse result from child #%02d: error_code=%d\n", id, pingres->error_code);
        else
            printf("result from child #%02d: ip=%s(%s), received=%d\n", id, pingres->ipaddr, servers[id], pingres->received);
        ++child_proceed;
    }
    free(buffer);
}

int main(int argc, const char* argv[]) {
    const char** servers = create_servers_list();
    size_t servers_cnt = get_list_size(servers);

    mqd_t mqdes = init_queue();
    size_t i;
    for (i = 0; i < servers_cnt; ++i) {
        pid_t child;
        assert((child = fork()) >= 0);
        if (child == 0) {
            child_proc(servers[i], mqdes, i);
            exit(0);
        } 
    }

    parent_proc(mqdes, servers, servers_cnt);
    for (;-1 != wait(NULL);)
        /*wait children loop*/;
    mq_close(mqdes);
    mq_unlink(QUEUE_NAME);
    return 0;
}
