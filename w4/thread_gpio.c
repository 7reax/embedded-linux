/**
 *
 * Kompilieren (normal):
 *   gcc -o thread_gpio thread_gpio.c -lpthread -lgpiod -lm
 *
 * Kompilieren (RT, mit Optimierung):
 *   gcc -O2 -o thread_gpio thread_gpio.c -lpthread -lgpiod -lm
 *
 * Ausführen (RT-Prioritäten benötigen root oder CAP_SYS_NICE):
 *   sudo ./thread_gpio
 * 
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <time.h>
#include <math.h>
#include <signal.h>
#include <errno.h>
#include <gpiod.h>

/* ─────────────────────────────────────────────
 * Konfiguration
 * ───────────────────────────────────────────── */

#define GPIO_CHIP        "gpiochip0"
#define GPIO_LINE_T1     77          /* PIN_8  – Thread 1 (100 ms) */
#define GPIO_LINE_T2     78          /* PIN_10 – Thread 2 (1 s)    */
#define GPIO_LINE_T3     81          /* PIN_12 – Thread 3 (1 ms)   */

#define CYCLE_T1_NS      100000000LL /* 100 ms */
#define CYCLE_T2_NS     1000000000LL /* 1 s    */
#define CYCLE_T3_NS        1000000LL /* 1 ms   */

/* Anzahl Messzyklen pro Thread für Statistik */
#define STAT_SAMPLES     1000

/* RT-Prioritäten (SCHED_FIFO, höher = mehr Priorität) */
#define RT_PRIO_T3       90   /* schnellster Thread – höchste Prio */
#define RT_PRIO_T1       80
#define RT_PRIO_T2       70

/* ─────────────────────────────────────────────
 * Typen
 * ───────────────────────────────────────────── */

typedef struct {
    int          thread_id;
    long long    cycle_ns;
    unsigned int gpio_line;
    /* Statistik */
    double       jitter_sum;
    double       jitter_sq_sum;
    double       jitter_max;
    long         sample_count;
} thread_args_t;

/* ─────────────────────────────────────────────
 * Globale Variablen
 * ───────────────────────────────────────────── */

static volatile int         g_running = 1;
static struct gpiod_chip   *g_chip    = NULL;
static struct gpiod_line   *g_lines[3] = {NULL, NULL, NULL};

/* ─────────────────────────────────────────────
 * Hilfsfunktionen
 * ───────────────────────────────────────────── */

/** Addiert nanoseconds zu einem timespec (überlaufsicher). */
static inline void timespec_add_ns(struct timespec *ts, long long ns)
{
    ts->tv_nsec += ns;
    while (ts->tv_nsec >= 1000000000LL) {
        ts->tv_nsec -= 1000000000LL;
        ts->tv_sec  += 1;
    }
}

/** Differenz zweier timespec in Nanosekunden (a - b). */
static inline long long timespec_diff_ns(const struct timespec *a,
                                         const struct timespec *b)
{
    return ((long long)(a->tv_sec  - b->tv_sec)  * 1000000000LL)
         +  (long long)(a->tv_nsec - b->tv_nsec);
}

/** GPIO-Linie anfordern. Gibt NULL zurück bei Fehler. */
static struct gpiod_line *gpio_request_output(unsigned int line_num,
                                              const char *consumer)
{
    struct gpiod_line *line = gpiod_chip_get_line(g_chip, line_num);
    if (!line) {
        fprintf(stderr, "[FEHLER] Linie %u nicht gefunden\n", line_num);
        return NULL;
    }
    if (gpiod_line_request_output(line, consumer, 0) < 0) {
        fprintf(stderr, "[FEHLER] Linie %u: request_output fehlgeschlagen: %s\n",
                line_num, strerror(errno));
        return NULL;
    }
    return line;
}

static void measure_gpio_overhead(struct gpiod_line *line)
{
    const int N = 10000;
    struct timespec t0, t1;
    long long sum = 0, min_t = LLONG_MAX, max_t = 0;

    printf("\n=== GPIO-Overhead-Messung (%d Wiederholungen) ===\n", N);

    for (int i = 0; i < N; i++) {
        clock_gettime(CLOCK_MONOTONIC, &t0);
        gpiod_line_set_value(line, 1);
        gpiod_line_set_value(line, 0);
        clock_gettime(CLOCK_MONOTONIC, &t1);

        long long dt = timespec_diff_ns(&t1, &t0);
        sum += dt;
        if (dt < min_t) min_t = dt;
        if (dt > max_t) max_t = dt;
    }

    printf("  Toggle-Dauer (set HIGH + set LOW):\n");
    printf("    Mittelwert : %7.2f µs\n", (double)sum / N / 1e3);
    printf("    Minimum    : %7.2f µs\n", (double)min_t / 1e3);
    printf("    Maximum    : %7.2f µs\n", (double)max_t / 1e3);
    printf("  -> Dieser Overhead ist der systematische Fehler\n");
    printf("     der GPIO-Messung und wird protokolliert.\n\n");
}

/* ─────────────────────────────────────────────
 * Thread-Funktion (generisch für alle drei Threads)
 * ───────────────────────────────────────────── */
static void *thread_func(void *arg)
{
    thread_args_t   *ta   = (thread_args_t *)arg;
    struct gpiod_line *line = g_lines[ta->thread_id - 1];
    struct timespec   next, now;
    int               gpio_val = 0;

    /* Startzeit: jetzt */
    clock_gettime(CLOCK_MONOTONIC, &next);

    printf("[Thread %d] gestartet | Zykluszeit: %lld ns | GPIO-Linie: %u\n",
           ta->thread_id, ta->cycle_ns, ta->gpio_line);

    while (g_running) {
        /* Nächste Aktivierungszeit berechnen */
        timespec_add_ns(&next, ta->cycle_ns);

        /* Präziser Wecker: schläft bis zur absoluten Zeit */
        clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &next, NULL);

        /* Ist-Zeit messen */
        clock_gettime(CLOCK_MONOTONIC, &now);

        /* Jitter = Differenz zwischen Soll- und Ist-Zeit */
        long long jitter = timespec_diff_ns(&now, &next);

        /* GPIO toggeln (Messung mit Logikanalysator / Oszilloskop) */
        if (line) {
            gpio_val ^= 1;
            gpiod_line_set_value(line, gpio_val);
        }

        /* Statistik aktualisieren */
        if (ta->sample_count < STAT_SAMPLES) {
            double j = (double)jitter;
            ta->jitter_sum    += j;
            ta->jitter_sq_sum += j * j;
            if (j > ta->jitter_max) ta->jitter_max = j;
            ta->sample_count++;

            /* Nach STAT_SAMPLES Statistik ausgeben */
            if (ta->sample_count == STAT_SAMPLES) {
                double mean = ta->jitter_sum / STAT_SAMPLES;
                double var  = (ta->jitter_sq_sum / STAT_SAMPLES) - (mean * mean);
                double std  = sqrt(var < 0 ? 0 : var);
                printf("\n[Thread %d] ─── Statistik nach %d Zyklen ───\n",
                       ta->thread_id, STAT_SAMPLES);
                printf("  Soll-Zykluszeit : %8.3f ms\n",
                       ta->cycle_ns / 1e6);
                printf("  Jitter Mittel   : %8.3f µs\n", mean / 1e3);
                printf("  Jitter StdAbw   : %8.3f µs\n", std  / 1e3);
                printf("  Jitter Maximum  : %8.3f µs\n",
                       ta->jitter_max / 1e3);
                printf("─────────────────────────────────────────\n");
            }
        }
    }

    /* GPIO zurücksetzen */
    if (line) gpiod_line_set_value(line, 0);

    printf("[Thread %d] beendet.\n", ta->thread_id);
    return NULL;
}

/* ─────────────────────────────────────────────
 * Signal-Handler (Ctrl+C sauber beenden)
 * ───────────────────────────────────────────── */
static void signal_handler(int sig)
{
    (void)sig;
    g_running = 0;
}

/* ─────────────────────────────────────────────
 * RT-Thread erstellen (SCHED_FIFO)
 * ───────────────────────────────────────────── */
static int create_rt_thread(pthread_t *tid, void *(*func)(void *),
                             void *arg, int rt_prio)
{
    pthread_attr_t      attr;
    struct sched_param  param;

    pthread_attr_init(&attr);

    /* Scheduling-Policy: FIFO (Realzeit) */
    if (pthread_attr_setschedpolicy(&attr, SCHED_FIFO) != 0) {
        fprintf(stderr, "[WARN] SCHED_FIFO nicht verfügbar – normaler Thread\n");
    } else {
        param.sched_priority = rt_prio;
        pthread_attr_setschedparam(&attr, &param);
        /* Priorität explizit übernehmen (nicht vererben) */
        pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
    }

    int ret = pthread_create(tid, &attr, func, arg);
    pthread_attr_destroy(&attr);
    return ret;
}

/* ─────────────────────────────────────────────
 * main
 * ───────────────────────────────────────────── */
int main(void)
{
    pthread_t     tids[3];
    thread_args_t args[3];

    signal(SIGINT,  signal_handler);
    signal(SIGTERM, signal_handler);

    printf("╔══════════════════════════════════════════════════╗\n");
    printf("║  Embedded Linux – Übung 04: Threads & Scheduling ║\n");
    printf("╚══════════════════════════════════════════════════╝\n\n");

    /* ── GPIO-Chip öffnen ── */
    g_chip = gpiod_chip_open_by_name(GPIO_CHIP);
    if (!g_chip) {
        fprintf(stderr, "[FEHLER] Kann %s nicht öffnen: %s\n",
                GPIO_CHIP, strerror(errno));
        fprintf(stderr, "         Bitte auf dem Ziel-Board ausführen!\n");
        /* Ohne GPIO trotzdem weiter (Simulation) */
    }

    /* ── GPIO-Leitungen anfordern ── */
    unsigned int line_nums[3] = {GPIO_LINE_T1, GPIO_LINE_T2, GPIO_LINE_T3};
    const char  *consumers[3] = {"thread1_100ms", "thread2_1s", "thread3_1ms"};

    if (g_chip) {
        for (int i = 0; i < 3; i++) {
            g_lines[i] = gpio_request_output(line_nums[i], consumers[i]);
            if (!g_lines[i]) {
                fprintf(stderr, "[WARN] Thread %d: GPIO deaktiviert\n", i + 1);
            }
        }

        /* ── Systematischen GPIO-Fehler messen ── */
        if (g_lines[0]) {
            measure_gpio_overhead(g_lines[0]);
        }
    }

    /* ── Thread-Argumente initialisieren ── */
    thread_args_t cfg[3] = {
        {1, CYCLE_T1_NS, GPIO_LINE_T1, 0, 0, 0, 0},
        {2, CYCLE_T2_NS, GPIO_LINE_T2, 0, 0, 0, 0},
        {3, CYCLE_T3_NS, GPIO_LINE_T3, 0, 0, 0, 0},
    };
    int rt_prios[3] = {RT_PRIO_T1, RT_PRIO_T2, RT_PRIO_T3};
    memcpy(args, cfg, sizeof(cfg));

    /* ── Threads erstellen ── */
    for (int i = 0; i < 3; i++) {
        if (create_rt_thread(&tids[i], thread_func, &args[i], rt_prios[i]) != 0) {
            fprintf(stderr, "[FEHLER] Thread %d konnte nicht erstellt werden\n", i + 1);
            g_running = 0;
            break;
        }
    }

    printf("\nAlle Threads laufen. Beenden mit Ctrl+C.\n\n");

    /* ── Warten bis Ctrl+C ── */
    for (int i = 0; i < 3; i++) {
        pthread_join(tids[i], NULL);
    }

    /* ── Aufräumen ── */
    if (g_chip) {
        for (int i = 0; i < 3; i++) {
            if (g_lines[i]) gpiod_line_release(g_lines[i]);
        }
        gpiod_chip_close(g_chip);
    }

    printf("\nProgramm sauber beendet.\n");
    return 0;
}
