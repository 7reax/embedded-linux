#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Verwendung: %s <PID>\n", argv[0]);
        return 1;
    }

    int pid = atoi(argv[1]);
    char path[64];
    char line[256];

    // Heap und Stack aus /proc/<pid>/maps
    snprintf(path, sizeof(path), "/proc/%d/maps", pid);
    FILE *maps = fopen(path, "r");
    if (!maps) { perror("/proc/<pid>/maps öffnen fehlgeschlagen"); return 1; }

    long heap_kb = 0, stack_kb = 0;
    while (fgets(line, sizeof(line), maps)) {
        unsigned long start, end;
        char name[64] = "";
        sscanf(line, "%lx-%lx %*s %*s %*s %*s %63s", &start, &end, name);
        long size = (end - start) / 1024;
        if (strcmp(name, "[heap]")  == 0) heap_kb  = size;
        if (strcmp(name, "[stack]") == 0) stack_kb = size;
    }
    fclose(maps);

    // Threadanzahl aus /proc/<pid>/status
    snprintf(path, sizeof(path), "/proc/%d/status", pid);
    FILE *status = fopen(path, "r");
    if (!status) { perror("/proc/<pid>/status öffnen fehlgeschlagen"); return 1; }

    int threads = 0;
    while (fgets(line, sizeof(line), status)) {
        if (sscanf(line, "Threads: %d", &threads) == 1) break;
    }
    fclose(status);

    // Ausgabe
    printf("Heap:    %ld kB\n", heap_kb);
    printf("Stack:   %ld kB\n", stack_kb);
    printf("Threads: %d\n", threads);

    return 0;
}