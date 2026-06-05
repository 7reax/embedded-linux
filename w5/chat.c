/* w5 Aufgabe 2: Ping-Pong-Chat ueber zwei Named Pipes.
 *   Aufruf: ./chat A    (in einem Terminal)
 *           ./chat B    (in einem anderen)
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <poll.h>
#include <sys/stat.h>

#define A2B "/tmp/chat_a2b"
#define B2A "/tmp/chat_b2a"

int main(int argc, char **argv) {
    if (argc != 2 || (argv[1][0] != 'A' && argv[1][0] != 'B')) {
        fprintf(stderr, "Aufruf: %s A|B\n", argv[0]); return 1;
    }
    mkfifo(A2B, 0666); mkfifo(B2A, 0666);

    int send, recv;
    if (argv[1][0] == 'A') {
        send = open(A2B, O_WRONLY);
        recv = open(B2A, O_RDONLY);
    } else {
        recv = open(A2B, O_RDONLY);
        send = open(B2A, O_WRONLY);
    }

    struct pollfd pf[2] = {{0, POLLIN, 0}, {recv, POLLIN, 0}};
    char buf[1024];
    for (;;) {
        if (poll(pf, 2, -1) < 0) break;
        if (pf[0].revents & POLLIN) {
            ssize_t r = read(0, buf, sizeof buf);
            if (r <= 0) break;
            write(send, buf, r);
        }
        if (pf[1].revents & POLLIN) {
            ssize_t r = read(recv, buf, sizeof buf - 1);
            if (r <= 0) break;
            buf[r] = 0;
            printf("> %s", buf);
            fflush(stdout);
        }
    }
    return 0;
}
