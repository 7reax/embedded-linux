#pragma once

#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

/* Andere Schlüssel als w6, damit beide parallel laufen können */
#define SHM_KEY      ((key_t)0xDEADBEEF)

/* Puffergröße für ein Bild */
#define IMG_BUF_SIZE 2048

/* Shared-Memory-Layout (kein Mutex!) */
typedef struct {
    int  image_id;
    int  write_count;
    char data[IMG_BUF_SIZE];
} shm_data_t;

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
