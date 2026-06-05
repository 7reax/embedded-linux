/**
 * Übung 06 – Shared Memory: Prozess A (Source)
 *
 * Kopiert abwechselnd Bild 1 und Bild 2 in den Shared Memory.
 * Sichert den Zugriff mit einem System-V-Semaphor (Mutex).
 *
 * Kompilieren:
 *   make shm_source
 *
 * Ausführen (zuerst starten, dann shm_sink in zweitem Terminal):
 *   ./shm_source
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>

#include "shm_common.h"

#define SWITCH_INTERVAL_MS  1000   /* Bildwechsel alle 1 Sekunde */

static volatile int g_running = 1;

static void signal_handler(int sig) { (void)sig; g_running = 0; }

static void ms_sleep(int ms)
{
    struct timespec ts = { ms / 1000, (long)(ms % 1000) * 1000000L };
    nanosleep(&ts, NULL);
}

int main(void)
{
    signal(SIGINT,  signal_handler);
    signal(SIGTERM, signal_handler);

    printf("╔════════════════════════════════════════════╗\n");
    printf("║  Embedded Linux – Übung 06: Shared Memory  ║\n");
    printf("║            Prozess A: Source               ║\n");
    printf("╚════════════════════════════════════════════╝\n\n");

    /* ── Shared Memory anlegen ─────────────────────────── */
    int shmid = shmget(SHM_KEY, sizeof(shm_data_t), IPC_CREAT | IPC_EXCL | 0666);
    if (shmid < 0) {
        if (errno == EEXIST) {
            fprintf(stderr, "[Source] FEHLER: Shared Memory existiert bereits.\n");
            fprintf(stderr, "         Bitte bereinigen: make ipc-clean\n");
        } else {
            perror("[Source] shmget");
        }
        return EXIT_FAILURE;
    }
    printf("[Source] Shared Memory angelegt   (shmid=%d, %zu Byte)\n",
           shmid, sizeof(shm_data_t));

    shm_data_t *shm = shmat(shmid, NULL, 0);
    if (shm == (void *)-1) {
        perror("[Source] shmat");
        shmctl(shmid, IPC_RMID, NULL);
        return EXIT_FAILURE;
    }
    memset(shm, 0, sizeof(*shm));

    /* ── Semaphore anlegen ─────────────────────────────── */
    int semid = semget(SEM_KEY, NUM_SEMS, IPC_CREAT | IPC_EXCL | 0666);
    if (semid < 0) {
        if (errno == EEXIST) {
            fprintf(stderr, "[Source] FEHLER: Semaphore existiert bereits.\n");
            fprintf(stderr, "         Bitte bereinigen: make ipc-clean\n");
        } else {
            perror("[Source] semget");
        }
        shmdt(shm);
        shmctl(shmid, IPC_RMID, NULL);
        return EXIT_FAILURE;
    }

    /* Mutex mit Wert 1 initialisieren (= freigegeben) */
    union semun arg;
    arg.val = 1;
    if (semctl(semid, SEM_MUTEX, SETVAL, arg) < 0) {
        perror("[Source] semctl SETVAL");
        shmdt(shm);
        shmctl(shmid, IPC_RMID, NULL);
        semctl(semid, 0, IPC_RMID);
        return EXIT_FAILURE;
    }
    printf("[Source] Semaphore angelegt       (semid=%d, Mutex=1)\n\n", semid);

    const char *images[2] = { IMAGE_1, IMAGE_2 };
    int img_idx = 0;

    printf("[Source] Bildwechsel alle %d ms. Beenden mit Ctrl+C.\n\n",
           SWITCH_INTERVAL_MS);

    while (g_running) {
        /* ── Kritischer Abschnitt: Bild schreiben ── */
        sem_p(semid);

        shm->image_id = img_idx + 1;
        shm->write_count++;
        strncpy(shm->data, images[img_idx], IMG_BUF_SIZE - 1);
        shm->data[IMG_BUF_SIZE - 1] = '\0';

        sem_v(semid);

        printf("[Source] Bild %d in Shared Memory geschrieben"
               "  (Schreibzähler: %d)\n",
               shm->image_id, shm->write_count);

        img_idx ^= 1;   /* 0 → 1 → 0 → ... */
        ms_sleep(SWITCH_INTERVAL_MS);
    }

    /* ── IPC-Ressourcen freigeben ──────────────────────── */
    printf("\n[Source] Räume IPC-Ressourcen auf ...\n");
    shmdt(shm);
    shmctl(shmid, IPC_RMID, NULL);   /* Shared Memory löschen */
    semctl(semid, 0, IPC_RMID);      /* Semaphore löschen */
    printf("[Source] Fertig.\n");
    return EXIT_SUCCESS;
}
