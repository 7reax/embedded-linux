CC      = gcc
CFLAGS  = -Wall -Wextra -O0 -g
TARGETS = shm_source shm_sink

SHM_KEY = 0xDEADBEEF

.PHONY: all clean ipc-clean run

all: $(TARGETS)

shm_source: shm_source.c shm_common.h
	$(CC) $(CFLAGS) -o $@ shm_source.c

shm_sink: shm_sink.c shm_common.h
	$(CC) $(CFLAGS) -o $@ shm_sink.c

run: all
	@echo "Starte shm_source im Hintergrund ..."
	@./shm_source &
	@sleep 0.3
	@./shm_sink
	@wait

ipc-clean:
	-ipcrm -M $(SHM_KEY) 2>/dev/null; true
	@echo "IPC-Ressourcen bereinigt."

clean: ipc-clean
	rm -f $(TARGETS)
