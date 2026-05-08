#include <stdio.h>
#include <unistd.h>

int main() {
    while (1) {
        FILE *fp = fopen("/proc/hello_proc", "r");

        if (fp) {
            char buffer[128];
            fgets(buffer, sizeof(buffer), fp);
            printf("%s", buffer);
            fclose(fp);
        } else {
            perror("Fehler beim Lesen von /proc");
        }

        sleep(5);
    }

    return 0;
}
