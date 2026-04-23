#include <cmath>
#include <cstdint>
#include <netinet/in.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/ip.h>

static void msg(const char *msg) {
    fprintf(stderr, "%s\n", msg);
}

static void die(const char *msg) {
    int err = errno;
    fprintf(stderr, "[%d] %s\n", err, msg);
    abort();
}

// struct in_addr{
//     uint32_t s_addr; //32 bit IP address (IPv4 in big-endian format)
// };


// struct sockaddr_in{
//     uint16_t sin_family; //address family
//     uint16_t sin_port;   //port number in big-endian format
//     struct in_addr sin_addr; //IP address
// };


// struct sockaddr_in6 {
//     uint16_t        sin6_family;     // Address family → AF_INET6
//     uint16_t        sin6_port;       // Port (network byte order)
//     uint32_t        sin6_flowinfo;   // Flow label (usually ignored)
//     struct in6_addr sin6_addr;       // IPv6 address
//     uint32_t        sin6_scope_id;   // Scope (used for link-local)
// };

// struct in6_addr {
//     uint8_t         s6_addr[16];   // IPv6
// };

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
        //msg("Failed reading length");
        return -1;
    }
    uint32_t bigen;
    memcpy(&bigen, lenchar, sizeof(bigen));
    uint32_t len = ntohl(bigen);
    char rbuf[1025];
    if (len > sizeof(rbuf) - 1) {
        //msg("Message too big");
        return -1;
    }
    int32_t rv2 = readfull(fd, rbuf, len);
    if (rv2 != 0) {
        //msg("Failed reading the message");
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

static void do_something(int connfd){

    /*
        ==============================
        TASK FOR DCs
        ==============================

        Implement complete client handling.

        You must:

        1) Create a buffer (char rbuf[64] or larger)

        2) Call read():
              ssize_t n = read(connfd, rbuf, sizeof(rbuf) - 1);

        3) Handle:
              n < 0  → error
              n == 0 → client closed connection
              n > 0  → valid data

        4) Null terminate:
              rbuf[n] = {0};

        5) Print what client sent.

        6) Send response using write().
    */


    // -------------------------------
    // Write your implementation here
    // -------------------------------
    int i=1;
    while(1){
        uint32_t rrv=readonce(connfd);
        if(rrv!=0){
            printf("Closing Connection\n");
            return;
        }
        char msg[32];
        snprintf(msg,sizeof(msg),"Server-side Message no.%d",i);
        writeonce(connfd,msg,strlen(msg));
        i++;
    }

    // -------------------------------
    // End of implementation
    // -------------------------------
}


int main(){

    //create socket
    //AF_INET -> IPv4 and AF_INET6 -> IPv6
    //SOCK_STREAM -> TCP and SOCK_DGRAM -> UDP
    //0 -> default protocol
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(socket_fd < 0){
        die("socket()");
    }

    //this is needed for most server applications
    int val=1;
    //used to modify the behaviour of sockets
    //tells the kernal which settings we want to change, at what level, and the new value
    //-->SO_REUSEADDR
        //tells the kernel to reuse a local socket in TIME_WAIT state,
        //without waiting for its natural timeout to expire.

    //The effect of SO_REUSEADDR is important: if it’s not set to 1, 
    //a server program cannot bind to the same IP:port it was using after a restart

    //you should enable SO_REUSEADDR for all listening sockets
    
    setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));

    //bind the socket to an IP and PORT
    //we will bind to a wildcard address 0.0.0.0:1234
    struct sockaddr_in addr={};
    addr.sin_family = AF_INET; //IPv4
    addr.sin_port = htons(1234); //port 1234, htons converts to network byte order, 16-bit port number
    addr.sin_addr.s_addr = htonl(0); //wildcard address 0.0.0.0, htonl converts to network byte order, 32-bit address

    //struct sockaddr_in holds an IPv4:port pair stored as big-endian numbers, converted by htons() and htonl().
    // For example, 1.2.3.4 is represented by htonl(0x01020304).

    int rv=bind(socket_fd,(const struct sockaddr*)&addr, sizeof(addr));
    if(rv){
        die("bind()");
    }

    //All the previous steps are just passing parameters. The socket is actually created after listen().
    rv = listen(socket_fd, SOMAXCONN);//second parameter is size of the backlog queue, usualyy 4096 in linux
    if (rv) { die("listen()"); }

    while(1){
        /*
            ==============================
            TASK FOR DCs
            ==============================

            Implement accept loop logic.

            You must:

            1) Create struct sockaddr_in client_addr
            2) Create socklen_t addrlen
            3) Call accept():
                  int connfd = accept(socket_fd,
                                      (struct sockaddr*)&client_addr,
                                      &addrlen);

            4) Handle:
                  connfd < 0 → error

            5) (Optional)
                  Print client IP and port using:
                      inet_ntop()
                      ntohs()

            6) Call:
                  do_something(connfd);

            7) Close connection:
                  close(connfd);
        */


        // -----------------------------------
        // Write your accept-loop logic here
        // -----------------------------------
        struct sockaddr_in client_addr={0};
        socklen_t addrlen = sizeof(client_addr);
        int connfd = accept(socket_fd,(struct sockaddr*)&client_addr,&addrlen);
        if(connfd<0){
            printf("connection error\n");
            continue;
        }
        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, sizeof(client_ip));
        printf("client IP:%s client port:%d\n",client_ip,ntohs(client_addr.sin_port));
        do_something(connfd);
        close(connfd);
        // -----------------------------------
        // End of DC implementation
        // -----------------------------------
    }
    return 0;
}
