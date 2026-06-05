/* w5 Aufgabe 4: Prozess 1 - fragt die Uhrzeit. */

#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include "mq_proto.h"

int main(void) {
    int q = msgget(ftok(KEY_PATH, KEY_PROJ), 0666);
    req_t req = { REQ_TIME, getpid() };
    msgsnd(q, &req, sizeof req - sizeof(long), 0);
    rep_t rep;
    msgrcv(q, &rep, sizeof rep.data, getpid(), 0);
    printf("Uhrzeit: %s\n", rep.data);
    return 0;
}
