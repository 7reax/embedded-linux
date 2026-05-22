/* w5 Aufgabe 4: Prozess 2 - fragt das Datum. */

#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include "mq_proto.h"

int main(void) {
    int q = msgget(ftok(KEY_PATH, KEY_PROJ), 0666);
    req_t req = { REQ_DATE, getpid() };
    msgsnd(q, &req, sizeof req - sizeof(long), 0);
    rep_t rep;
    msgrcv(q, &rep, sizeof rep.data, getpid(), 0);
    printf("Datum: %s\n", rep.data);
    return 0;
}
