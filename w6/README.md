# Übung 06 – Shared Memory

Zwei Prozesse kommunizieren über System-V Shared Memory.  
**Prozess A (Source)** schreibt abwechselnd Bild 1 und Bild 2 in den gemeinsamen Speicher.  
**Prozess B (Sink)** liest alle 200 ms daraus und gibt das aktuelle Bild im Terminal aus.

```
  Prozess A (Source)          Shared Memory          Prozess B (Sink)
  ─────────────────     ┌───────────────────────┐    ────────────────
  schreibt Bild 1/2 ──▶ │ image_id | write_count │ ──▶ liest alle 200 ms
  alle 1000 ms          │ data[2048]             │    gibt Bild aus
                        └───────────────────────┘
                          gesichert durch Semaphor (Mutex)
```

## Ausführen

### Option 1 – Zwei separate Terminals (empfohlen)

**Terminal 1 – Source starten:**
```bash
./shm_source
```

**Terminal 2 – Sink starten:**
```bash
./shm_sink
```

> Die Source muss zuerst gestartet werden – sie legt den Shared Memory und die Semaphore an.  
> Der Sink wartet automatisch, bis die Ressourcen verfügbar sind.

### Option 2 – Beide Prozesse auf einmal

```bash
make run
```

Source läuft im Hintergrund, Sink im Vordergrund. `Ctrl+C` beendet den Sink;  
die Source muss danach separat mit `Ctrl+C` beendet werden.

---

## Beenden & Aufräumen

Beide Prozesse mit `Ctrl+C` beenden. Die Source räumt Shared Memory und Semaphore automatisch auf.

Falls ein Prozess abstürzt und IPC-Ressourcen zurückbleiben:

```bash
make ipc-clean
```

Oder manuell:
```bash
ipcrm -M 0x1A2B3C4D   # Shared Memory entfernen
ipcrm -S 0x5E6F7A8B   # Semaphore entfernen
```

Aktuelle IPC-Ressourcen anzeigen:
```bash
ipcs -a
```

---

## Konfiguration

| Konstante | Datei | Standardwert | Beschreibung |
|---|---|---|---|
| `SWITCH_INTERVAL_MS` | `shm_source.c` | `1000` ms | Wie oft Source das Bild wechselt |
| `READ_INTERVAL_MS` | `shm_sink.c` | `200` ms | Wie oft Sink den Speicher ausliest |
| `SHM_KEY` | `shm_common.h` | `0x1A2B3C4D` | System-V IPC-Schlüssel für den Shared Memory |
| `SEM_KEY` | `shm_common.h` | `0x5E6F7A8B` | System-V IPC-Schlüssel für die Semaphore |

---

## Verwendete Syscalls

| Syscall | Zweck |
|---|---|
| `shmget` | Shared-Memory-Segment anlegen / öffnen |
| `shmat` | Segment in den Adressraum einblenden |
| `shmdt` | Segment aus dem Adressraum ausblenden |
| `shmctl(IPC_RMID)` | Segment löschen |
| `semget` | Semaphore-Menge anlegen / öffnen |
| `semctl(SETVAL)` | Semaphore initialisieren |
| `semop` | P-Operation (sperren) / V-Operation (freigeben) |
| `semctl(IPC_RMID)` | Semaphore-Menge löschen |
