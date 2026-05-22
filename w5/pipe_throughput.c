/* w5 Aufgabe 1: Durchsatz einer anonymen Pipe. */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/wait.h>
#include <sys/mman.h>

typedef struct { struct timespec c1, c2, c3, c4; } ts_t;

static long long dns(const struct timespec *a, const struct timespec *b) {
    return (long long)(a->tv_sec - b->tv_sec) * 1000000000LL + (a->tv_nsec - b->tv_nsec);
}

static void run(size_t n) {
    ts_t *t = mmap(NULL, sizeof *t, PROT_READ|PROT_WRITE,
                   MAP_SHARED|MAP_ANONYMOUS, -1, 0);
    int p[2]; pipe(p);
    char *buf = malloc(n); memset(buf, 'A', n);

    if (fork() == 0) {
        close(p[1]);
        char *r = malloc(n);
        size_t got = 0; int first = 1;
        while (got < n) {
            ssize_t k = read(p[0], r + got, n - got);
            if (first) { clock_gettime(CLOCK_MONOTONIC, &t->c3); first = 0; }
            if (k <= 0) break;
            got += k;
        }
        clock_gettime(CLOCK_MONOTONIC, &t->c4);
        _exit(0);
    }
    close(p[0]);
    clock_gettime(CLOCK_MONOTONIC, &t->c1);
    size_t s = 0;
    while (s < n) { ssize_t k = write(p[1], buf + s, n - s); if (k <= 0) break; s += k; }
    clock_gettime(CLOCK_MONOTONIC, &t->c2);
    close(p[1]);
    wait(NULL);

    printf("%7zu B  t_Kernel=%8lld ns  t_Uebertragung=%8lld ns\n",
           n, dns(&t->c3, &t->c2), dns(&t->c4, &t->c1));
    free(buf);
    munmap(t, sizeof *t);
}

int main(void) {
    size_t sizes[] = { 100, 1024, 10*1024, 100*1024 };
    for (size_t i = 0; i < 4; i++) run(sizes[i]);
    return 0;
}
