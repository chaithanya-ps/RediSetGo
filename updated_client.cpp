#include <cstdint>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/ip.h>


// Print errno and message, then abort the program
static void die(const char *msg) {
    int err = errno;
    fprintf(stderr, "[%d] %s\n", err, msg);
    abort();
}

static void msg(const char *msg) {
    fprintf(stderr, "%s\n", msg);
}

static int32_t readfull(int fd, char* buf, size_t n) {
    while (n > 0) {
        ssize_t rn = read(fd, buf, n);
        if (rn <= 0) return -1;
        n -= (size_t)rn;
        buf += rn;
    }
    return 0;
}

static int32_t writefull(int fd, const char* buf, size_t n) {
    while (n > 0) {
        ssize_t wn = write(fd, buf, n);
        if (wn <= 0) return -1;
        n -= (size_t)wn;
        buf += wn;
    }
    return 0;
}

static int32_t readonce(int fd) {
    char lenchar[4];
    int32_t rv1 = readfull(fd, lenchar, 4);
    if (rv1 != 0) {
        msg("Failed reading length");
        return -1;
    }
    uint32_t bigen;
    memcpy(&bigen, lenchar, sizeof(bigen));
    uint32_t len = ntohl(bigen);
    char rbuf[1025];
    if (len > sizeof(rbuf) - 1) {
        msg("Message too big");
        return -1;
    }
    int32_t rv2 = readfull(fd, rbuf, len);
    if (rv2 != 0) {
        msg("Failed reading the message");
        return -1;
    }
    rbuf[len] = '\0';
    printf("Server says: %s\n", rbuf);
    return 0;
}

static int32_t writeonce(int fd, const char* buf, uint32_t n) {
    uint32_t len = 4 + n;
    char finbuf[len];
    uint32_t nlen = htonl(n);
    memcpy(finbuf, &nlen, 4);
    memcpy(finbuf + 4, buf, n);
    int32_t rv = writefull(fd, finbuf, len);
    if (rv != 0) {
        msg("Write error");
        return -1;
    }
    return 0;
}

int main() {

    /*
     * Create a socket
     */
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        die("socket()");
    }

    /*
     * Prepare server address
     */
    struct sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(1234);
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    /*
     * Connect to server
     */
    int rv = connect(socket_fd, (const struct sockaddr *)&addr, sizeof(addr));
    if (rv < 0) {
        die("connect()");
    }

    /*
     * STEP 1 (DC Task):
     * Create a message buffer.
     *
     * Example:
     * char msg[] = "hello";
     */
    char msg[] = "Nithils Client";


    /*
     * STEP 2 (DC Task):
     * Send the message using write().
     *
     * write() arguments:
     * write(socket_fd, buffer, number_of_bytes);
     *
     * For now:
     * - Just call write once.
     * - We will handle partial writes later.
     */
    writeonce(socket_fd, msg, (uint32_t)strlen(msg));


    /*
     * STEP 3 (DC Task):
     * Create a receive buffer.
     *
     * Example:
     * char rbuf[64];
     */
    //char rbuf[64]={0};


    /*
     * STEP 4 (DC Task):
     * Call read() to receive data from server.
     *
     * read(socket_fd, rbuf, sizeof(rbuf) - 1);
     *
     * For now:
     * - Just call read once.
     * - Store return value.
     * - We will handle edge cases later.
     */
    //ssize_t n=read(socket_fd,rbuf,sizeof(rbuf)-1);


    /*
     * STEP 5 (DC Task):
     * Null-terminate the buffer.CCCC
     * Then print:
     *
     * printf("server says: %s\n", rbuf);
     */
    //rbuf[n]='\0';
    //printf("server says: %s\n",rbuf);
    readonce(socket_fd);
    for(int i=0;i<11;i++){
        writeonce(socket_fd, msg, (uint32_t)strlen(msg));
        readonce(socket_fd);
    }
    /*
     * Close socket
     */
    close(socket_fd);

    return 0;
}