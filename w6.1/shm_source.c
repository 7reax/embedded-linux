/**
 * Übung 06.1 – Shared Memory OHNE Mutex: Prozess A (Source)
 *
 * Schreibt abwechselnd Bild 1 und Bild 2 zeichenweise in den
 * Shared Memory (1 ms Pause pro Zeichen). Ein vollständiges Bild
 * dauert so ~500 ms — lang genug, damit Prozess B mittendrin liest.
 *
 * Kompilieren:  make
 * Ausführen:    ./shm_source   (zuerst, dann shm_sink)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#include "shm_common.h"

static volatile int g_running = 1;

static void signal_handler(int sig) { (void)sig; g_running = 0; }

static void us_sleep(long us)
{
    struct timespec ts = { us / 1000000L, (us % 1000000L) * 1000L };
    nanosleep(&ts, NULL);
}

/* Schreibt src zeichenweise mit delay_us µs Pause pro Zeichen */
static void slow_copy(char *dst, const char *src, int delay_us)
{
    while (*src) {
        *dst++ = *src++;
        us_sleep(delay_us);
    }
    *dst = '\0';
}

int main(void)
{
    signal(SIGINT,  signal_handler);
    signal(SIGTERM, signal_handler);

    printf("╔════════════════════════════════════════════╗\n");
    printf("║  Übung 06.1 – Shared Memory OHNE Mutex    ║\n");
    printf("║            Prozess A: Source               ║\n");
    printf("╚════════════════════════════════════════════╝\n\n");
    printf("[Source] KEIN Mutex! Schreibe zeichenweise (~1 ms/Zeichen).\n\n");

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
    printf("[Source] Shared Memory angelegt (shmid=%d, %zu Byte)\n\n",
           shmid, sizeof(shm_data_t));

    shm_data_t *shm = shmat(shmid, NULL, 0);
    if (shm == (void *)-1) {
        perror("[Source] shmat");
        shmctl(shmid, IPC_RMID, NULL);
        return EXIT_FAILURE;
    }
    memset(shm, 0, sizeof(*shm));

    const char *images[2] = { IMAGE_1, IMAGE_2 };
    int img_idx = 0;

    while (g_running) {
        shm->image_id    = img_idx + 1;
        shm->write_count++;

        printf("[Source] Beginne Schreiben von Bild %d  (Zähler=%d) ...\n",
               shm->image_id, shm->write_count);

        /* Zeichenweises Schreiben — kein Lock, kein Mutex */
        slow_copy(shm->data, images[img_idx], 1000 /* 1 ms/Zeichen */);

        printf("[Source] Bild %d fertig geschrieben.\n\n", shm->image_id);

        img_idx ^= 1;

        /* Kurze Pause, damit Sink auch mal ein vollständiges Bild sieht */
        us_sleep(300000); /* 300 ms */
    }

    printf("\n[Source] Räume auf ...\n");
    shmdt(shm);
    shmctl(shmid, IPC_RMID, NULL);
    printf("[Source] Fertig.\n");
    return EXIT_SUCCESS;
}
