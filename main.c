#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int main() {
    printf("Hello World!\n");
    printf("PID: %d\n", getpid());
    printf("Programm läuft... (Ctrl+C zum Beenden)\n");

    // Etwas Heap-Speicher allozieren damit /proc interessanter wird
    char *heap_data = malloc(1024 * 1024); // 1 MB
    if (heap_data) {
        for (int i = 0; i < 1024 * 1024; i++) heap_data[i] = i % 256;
    }

    // Endlosschleife damit /proc analysiert werden kann
    while (1) {
        sleep(1);
    }

    free(heap_data);
    return 0;
}