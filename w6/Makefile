CC      = gcc
CFLAGS  = -Wall -Wextra -O2 -g
TARGETS = shm_source shm_sink

# IPC-Schlüssel (müssen mit shm_common.h übereinstimmen)
SHM_KEY = 0x1A2B3C4D
SEM_KEY = 0x5E6F7A8B

.PHONY: all clean ipc-clean run-source run-sink

all: $(TARGETS)

shm_source: shm_source.c shm_common.h
	$(CC) $(CFLAGS) -o $@ shm_source.c

shm_sink: shm_sink.c shm_common.h
	$(CC) $(CFLAGS) -o $@ shm_sink.c

# Starte Source in Hintergrund und Sink im Vordergrund
run: all
	@echo "Starte shm_source im Hintergrund ..."
	@./shm_source &
	@sleep 0.3
	@./shm_sink
	@wait

run-source: shm_source
	./shm_source

run-sink: shm_sink
	./shm_sink

# IPC-Ressourcen manuell bereinigen (nach unbeabsichtigtem Absturz)
ipc-clean:
	-ipcrm -M $(SHM_KEY) 2>/dev/null; true
	-ipcrm -S $(SEM_KEY) 2>/dev/null; true
	@echo "IPC-Ressourcen bereinigt."

clean: ipc-clean
	rm -f $(TARGETS)
