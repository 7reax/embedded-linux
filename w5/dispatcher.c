/* w5 Aufgabe 4: Date/Time-Dispatcher.
 *   Beantwortet REQ_TIME und REQ_DATE Requests. Antwort mtype = client PID.
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include "mq_proto.h"

static int q;
static void cleanup(int s) { (void)s; msgctl(q, IPC_RMID, NULL); _exit(0); }

int main(void) {
    signal(SIGINT, cleanup);
    q = msgget(ftok(KEY_PATH, KEY_PROJ), IPC_CREAT | 0666);

    for (;;) {
        req_t req;
        msgrcv(q, &req, sizeof req - sizeof(long), 0, 0);
        rep_t rep = { .mtype = req.pid };
        time_t now = time(NULL);
        struct tm tm; localtime_r(&now, &tm);
        if (req.mtype == REQ_TIME)
            strftime(rep.data, sizeof rep.data, "%H:%M:%S", &tm);
        else
            strftime(rep.data, sizeof rep.data, "%Y-%m-%d", &tm);
        msgsnd(q, &rep, strlen(rep.data) + 1, 0);
        printf("PID %d  %s -> %s\n", req.pid,
               req.mtype == REQ_TIME ? "TIME" : "DATE", rep.data);
    }
}
