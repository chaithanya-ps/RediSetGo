#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <string>
#include <vector>

// ===================== UTIL =====================

static void msg(const char *msg) {
    fprintf(stderr, "%s\n", msg);
}

static void die(const char *msg) {
    int err = errno;
    fprintf(stderr, "[%d] %s\n", err, msg);
    abort();
}

// ===================== IO HELPERS =====================

// Read exactly n bytes
static int32_t read_full(int fd, char *buf, size_t n) {
    while (n > 0) {
        ssize_t rv = read(fd, buf, n);
        if (rv <= 0) {
            return -1;
        }
        n -= (size_t)rv;
        buf += rv;
    }
    return 0;
}

// Write exactly n bytes
static int32_t write_all(int fd, const char *buf, size_t n) {
    while (n > 0) {
        ssize_t rv = write(fd, buf, n);
        if (rv <= 0) {
            return -1;
        }
        n -= (size_t)rv;
        buf += rv;
    }
    return 0;
}

// ===================== PROTOCOL =====================

const size_t k_max_msg = 4096;

// ===================== REQUEST BUILD (NEW) =====================

static int32_t send_req(int fd, const std::vector<std::string> &cmd) {

    // ===================== TODO =====================
    // Build request in binary format and send it

    // Protocol:
    // [len][nstr][len][str1][len][str2]...

    // Steps:
    // 1. Compute total length:
    //    len = 4 (for nstr) + sum(4 + string size)
    // 2. Check len <= k_max_msg
    // 3. Create buffer
    // 4. Write:
    //      - total length
    //      - number of strings (cmd.size())
    //      - for each string:
    //          length + data
    // 5. Send using write_all()

    uint32_t len = 4;
    for (const std::string &s : cmd) {
        len += 4 + s.size();
    }
    if (len > k_max_msg) 
        return -1;

    std::vector<uint8_t> buf;
    buf.resize(4 + len);
    uint32_t wlen = htonl(len);
    memcpy(buf.data(), &wlen, 4);
    uint32_t nstr = htonl((uint32_t)cmd.size());
    memcpy(buf.data() + 4, &nstr, 4);

    size_t cur = 8;
    for (const std::string &s : cmd) {
        uint32_t slen = htonl((uint32_t)s.size());
        memcpy(buf.data() + cur, &slen, 4);
        memcpy(buf.data() + cur + 4, s.data(), s.size());
        cur += 4 + s.size();
    }
    return write_all(fd, (const char*)buf.data(), 4 + len);

    //return -1;
}

// ===================== RESPONSE READ (NEW) =====================

static int32_t read_res(int fd) {

    // ===================== TODO =====================
    // Read response from server

    // Protocol:
    // [len][status][data]

    // Steps:
    // 1. Read 4 bytes → length
    // 2. Validate length <= k_max_msg
    // 3. Read remaining bytes
    // 4. Extract:
    //      status (first 4 bytes)
    //      data (remaining bytes)
    // 5. Print result

    uint32_t len = 0;
    if (read_full(fd, (char*)&len, 4) < 0) 
        return -1;
    len = ntohl(len);
    if (len > k_max_msg) 
        return -1;
    std::vector<uint8_t> buf(len);
    if (read_full(fd, (char*)buf.data(), len) < 0) 
        return -1;
    uint32_t status = 0;
    memcpy(&status, buf.data(), 4);
    if (len > 4) {
        printf("Data: %.*s\n", (int)(len - 4), (char*)(buf.data() + 4));
    }
    return 0;
}

// ===================== MAIN =====================

int main(int argc, char **argv) {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        die("socket()");
    }

    struct sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(1234);
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);  // 127.0.0.1

    int rv = connect(fd, (const struct sockaddr *)&addr, sizeof(addr));
    if (rv) {
        die("connect");
    }

    std::vector<std::string> cmd;

    // Command from CLI
    for (int i = 1; i < argc; ++i) {
        cmd.push_back(argv[i]);
    }

    int32_t err = send_req(fd, cmd);
    if (err) {
        goto L_DONE;
    }

    err = read_res(fd);
    if (err) {
        goto L_DONE;
    }

L_DONE:
    close(fd);
    return 0;
}