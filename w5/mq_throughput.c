/* w5 Aufgabe 3: Durchsatz einer System V Message Queue. Grosse Nachrichten werden in CHUNK-Bloecke aufgeteilt (msgmax-Limit). */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#define CHUNK 4096

typedef struct { long mtype; char data[CHUNK]; } msg_t;
typedef struct { struct timespec c1, c2, c3, c4; } ts_t;

static long long dns(const struct timespec *a, const struct timespec *b) {
    return (long long)(a->tv_sec - b->tv_sec) * 1000000000LL + (a->tv_nsec - b->tv_nsec);
}

static void run(int q, size_t n) {
    ts_t *t = mmap(NULL, sizeof *t, PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0);
    size_t chunks = (n + CHUNK - 1) / CHUNK;

    if (fork() == 0) {
        msg_t m; int first = 1;
        for (size_t i = 0; i < chunks; i++) {
            msgrcv(q, &m, CHUNK, 0, 0);
            if (first) { clock_gettime(CLOCK_MONOTONIC, &t->c3); first = 0; }
        }
        clock_gettime(CLOCK_MONOTONIC, &t->c4);
        _exit(0);
    }
    msg_t m = { .mtype = 1 };
    memset(m.data, 'A', CHUNK);
    clock_gettime(CLOCK_MONOTONIC, &t->c1);
    size_t rem = n;
    while (rem) {
        size_t c = rem > CHUNK ? CHUNK : rem;
        msgsnd(q, &m, c, 0);
        rem -= c;
    }
    clock_gettime(CLOCK_MONOTONIC, &t->c2);
    wait(NULL);

    printf("%7zu B  t_Kernel=%8lld ns  t_Uebertragung=%8lld ns\n",
           n, dns(&t->c3, &t->c2), dns(&t->c4, &t->c1));
    munmap(t, sizeof *t);
}

int main(void) {
    key_t k = ftok("/tmp", 'T');
    int q = msgget(k, IPC_CREAT | 0666);
    size_t sizes[] = { 100, 1024, 10*1024, 100*1024 };
    for (size_t i = 0; i < 4; i++) run(q, sizes[i]);
    msgctl(q, IPC_RMID, NULL);
    return 0;
}
