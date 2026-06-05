/**
 * Übung 06.1 – Shared Memory OHNE Mutex: Prozess B (Sink)
 *
 * Liest alle 200 ms direkt aus dem Shared Memory — ohne Lock.
 * Da Source zeichenweise schreibt (~500 ms pro Bild), wird Sink
 * regelmäßig einen halbfertigen Puffer sehen: die ersten Zeichen
 * des neuen Bildes, gefolgt vom Rest des alten Bildes.
 *
 * Kompilieren:  make
 * Ausführen:    ./shm_sink   (nachdem shm_source läuft)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#include "shm_common.h"

#define READ_INTERVAL_MS 200

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
    printf("║  Übung 06.1 – Shared Memory OHNE Mutex    ║\n");
    printf("║            Prozess B: Sink                 ║\n");
    printf("╚════════════════════════════════════════════╝\n\n");
    printf("[Sink] KEIN Mutex! Lese alle %d ms direkt aus Shared Memory.\n\n",
           READ_INTERVAL_MS);
    printf("[Sink] Warte auf shm_source ...\n");

    /* ── Shared Memory suchen ──────────────────────────── */
    int shmid = -1;
    while (shmid < 0 && g_running) {
        shmid = shmget(SHM_KEY, sizeof(shm_data_t), 0666);
        if (shmid < 0) ms_sleep(100);
    }
    if (!g_running) return EXIT_SUCCESS;

    /* Schreibzugriff nötig — SHM_RDONLY würde keinen echten Torn-Read zeigen */
    shm_data_t *shm = shmat(shmid, NULL, 0);
    if (shm == (void *)-1) {
        perror("[Sink] shmat");
        return EXIT_FAILURE;
    }
    printf("[Sink] Shared Memory gefunden (shmid=%d)\n\n", shmid);

    int read_nr = 0;

    while (g_running) {
        ms_sleep(READ_INTERVAL_MS);

        /* Snapshot ohne Lock — potentiell zerrissener Puffer */
        char buf[IMG_BUF_SIZE];
        int  id    = shm->image_id;
        int  count = shm->write_count;
        memcpy(buf, shm->data, IMG_BUF_SIZE);

        read_nr++;

        /* Immer ausgeben — auch wenn count gleich geblieben ist,
         * damit torn reads nicht verpasst werden */
        printf("\033[2J\033[H");   /* Terminal löschen */
        printf("╔════════════════════════════════════════════╗\n");
        printf("║  Übung 06.1 – Shared Memory OHNE Mutex    ║\n");
        printf("║            Prozess B: Sink                 ║\n");
        printf("╚════════════════════════════════════════════╝\n\n");
        printf("  Lesung #%d | Bild-ID: %d | Schreibzähler: %d\n",
               read_nr, id, count);
        printf("  (kein Mutex → Torn Read möglich!)\n\n");
        printf("--- Pufferinhalt (roh aus Shared Memory) ---\n");
        printf("%s", buf);
        printf("--- Ende Puffer ---\n");
    }

    printf("\n[Sink] Beendet.\n");
    shmdt(shm);
    return EXIT_SUCCESS;
}
