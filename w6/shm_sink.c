/**
 * Übung 06 – Shared Memory: Prozess B (Sink)
 *
 * Liest alle 200 ms aus dem Shared Memory und gibt das Bild aus.
 * Sichert den Zugriff mit einem System-V-Semaphor (Mutex).
 *
 * Kompilieren:
 *   make shm_sink
 *
 * Ausführen (nach shm_source, in einem zweiten Terminal):
 *   ./shm_sink
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

#define READ_INTERVAL_MS  200   /* Leseintervall: 200 ms */

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
    printf("║            Prozess B: Sink                 ║\n");
    printf("╚════════════════════════════════════════════╝\n\n");
    printf("[Sink] Warte auf Shared Memory  (shm_source zuerst starten!) ...\n");

    /* ── Shared Memory suchen (warten bis Source ihn anlegt) ── */
    int shmid = -1;
    while (shmid < 0 && g_running) {
        shmid = shmget(SHM_KEY, sizeof(shm_data_t), 0666);
        if (shmid < 0) ms_sleep(100);
    }
    if (!g_running) return EXIT_SUCCESS;

    /* Nur lesend anhängen */
    shm_data_t *shm = shmat(shmid, NULL, SHM_RDONLY);
    if (shm == (void *)-1) {
        perror("[Sink] shmat");
        return EXIT_FAILURE;
    }
    printf("[Sink] Shared Memory gefunden     (shmid=%d)\n", shmid);

    /* ── Semaphore suchen ────────────────────────────────── */
    int semid = -1;
    while (semid < 0 && g_running) {
        semid = semget(SEM_KEY, NUM_SEMS, 0666);
        if (semid < 0) ms_sleep(100);
    }
    if (!g_running) {
        shmdt(shm);
        return EXIT_SUCCESS;
    }
    printf("[Sink] Semaphore gefunden         (semid=%d)\n", semid);
    printf("[Sink] Lese alle %d ms. Beenden mit Ctrl+C.\n\n", READ_INTERVAL_MS);

    int last_count = -1;

    while (g_running) {
        /* ── Kritischer Abschnitt: Bild lesen ── */
        if (sem_p(semid) < 0) {
            /* Semaphore wurde entfernt (Source hat beendet) */
            printf("\n[Sink] Semaphore nicht mehr verfügbar – Source beendet.\n");
            break;
        }

        int  id    = shm->image_id;
        int  count = shm->write_count;
        char buf[IMG_BUF_SIZE];
        memcpy(buf, shm->data, IMG_BUF_SIZE);

        if (sem_v(semid) < 0) {
            printf("\n[Sink] Semaphore nicht mehr verfügbar – Source beendet.\n");
            break;
        }

        /* Ausgabe nur wenn sich das Bild geändert hat */
        if (count != last_count && count > 0) {
            /* Terminal löschen (ANSI Escape: Cursor nach oben-links) */
            printf("\033[2J\033[H");
            printf("╔════════════════════════════════════════════╗\n");
            printf("║  Embedded Linux – Übung 06: Shared Memory  ║\n");
            printf("║            Prozess B: Sink                 ║\n");
            printf("╚════════════════════════════════════════════╝\n\n");
            printf("  Bild-ID: %d  |  Schreibzähler: %d  |"
                   "  Leseintervall: %d ms\n\n",
                   id, count, READ_INTERVAL_MS);
            printf("%s\n", buf);
            last_count = count;
        }

        ms_sleep(READ_INTERVAL_MS);
    }

    printf("\n[Sink] Beendet.\n");
    shmdt(shm);
    return EXIT_SUCCESS;
}
