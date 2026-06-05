#pragma once

#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>

/* ── IPC-Schlüssel ─────────────────────────────────────── */
#define SHM_KEY      ((key_t)0x1A2B3C4D)
#define SEM_KEY      ((key_t)0x5E6F7A8B)

/* ── Semaphor-Indizes ──────────────────────────────────── */
#define SEM_MUTEX    0
#define NUM_SEMS     1

/* ── Puffergröße für ein Bild ──────────────────────────── */
#define IMG_BUF_SIZE 2048

/* ── Shared-Memory-Layout ──────────────────────────────── */
typedef struct {
    int  image_id;
    int  write_count;
    char data[IMG_BUF_SIZE];
} shm_data_t;

/* ── semun (auf Linux muss die Anwendung sie selbst definieren) */
#ifndef __APPLE__
union semun {
    int              val;
    struct semid_ds *buf;
    unsigned short  *array;
};
#endif

/* ── Semaphor P (sperren) / V (freigeben) ─────────────── */
static inline int sem_p(int semid)
{
    struct sembuf op = { SEM_MUTEX, -1, 0 };
    return semop(semid, &op, 1);
}

static inline int sem_v(int semid)
{
    struct sembuf op = { SEM_MUTEX, +1, 0 };
    return semop(semid, &op, 1);
}

/* ── ASCII-Bilder ──────────────────────────────────────── */
#define IMAGE_1 \
    "+------------------------------------------+\n" \
    "|                                          |\n" \
    "|          *    *    *    *    *           |\n" \
    "|        *    \\    * | *   /    *         |\n" \
    "|      *   *   \\   * | *   /   *   *     |\n" \
    "|   * * * * *----(   O   )----* * * * *   |\n" \
    "|      *   *   /   * | *   \\   *   *     |\n" \
    "|        *    /    * | *    \\    *        |\n" \
    "|          *    *    *    *    *           |\n" \
    "|                                          |\n" \
    "|         ~~~~ BILD 1: SONNE ~~~~         |\n" \
    "|                                          |\n" \
    "+------------------------------------------+\n"

#define IMAGE_2 \
    "+------------------------------------------+\n" \
    "|                                          |\n" \
    "|               . - - - .                 |\n" \
    "|            .-'         '-.              |\n" \
    "|           /    *     *    \\            |\n" \
    "|          |    *  ---  *    |            |\n" \
    "|          |                 |            |\n" \
    "|           \\               /            |\n" \
    "|            ' -.______.- '              |\n" \
    "|                                          |\n" \
    "|         ~~~~ BILD 2: MOND  ~~~~         |\n" \
    "|                                          |\n" \
    "+------------------------------------------+\n"
