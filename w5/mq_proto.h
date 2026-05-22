/* Gemeinsames Protokoll fuer Dispatcher + Clients. */
#ifndef MQ_PROTO_H
#define MQ_PROTO_H
#include <sys/types.h>

#define KEY_PATH "/tmp"
#define KEY_PROJ 'D'
#define REQ_TIME 1L
#define REQ_DATE 2L

typedef struct { long mtype; pid_t pid; } req_t;
typedef struct { long mtype; char data[64]; } rep_t;

#endif
