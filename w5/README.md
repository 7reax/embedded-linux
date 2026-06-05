# w5 — Pipes, Named Pipes, Message Queues

## Bauen

```bash
make
```

## Zeitmessung (C1..C4)

| Marker | Bedeutung                          |
|--------|------------------------------------|
| C1     | Sender ruft 1. write/msgsnd auf    |
| C2     | Sender hat letzten Block uebergeben|
| C3     | Empfaenger erhaelt 1. Block        |
| C4     | Empfaenger hat letzten Block       |

`t_Kernel = C3 - C2`, `t_Uebertragung = C4 - C1`. Bei groesseren Datenmengen
kann `t_Kernel` negativ werden, weil der Empfaenger zu lesen beginnt, bevor
der Sender fertig ist (Pipe/Queue-Puffer ist voll).

---

## 1) `pipe_throughput` — anonyme Pipe

```bash
./pipe_throughput
```

Parent forkt ein Child, schickt 100 B / 1 KiB / 10 KiB / 100 KiB ueber eine
anonyme Pipe. Zeitstempel liegen in Shared-Memory (`mmap MAP_ANONYMOUS|MAP_SHARED`),
damit Parent nach `wait()` alle vier Werte sieht.

---

## 2) `chat` — Ping-Pong-Chat ueber Named Pipes

```bash
# Terminal 1
./chat A

# Terminal 2
./chat B
```

Legt `/tmp/chat_a2b` und `/tmp/chat_b2a` an. `poll()` ueberwacht stdin und
die Empfangs-FIFO. Beenden mit Ctrl+C; FIFOs werden mit
`make clean` entfernt.

---

## 3) `mq_throughput` — System V Message Queue

```bash
./mq_throughput
```

Gleiche Methodik wie Pipe-Test, nur ueber `msgsnd`/`msgrcv`. Nutzdaten
werden in 4 KiB-Bloecke zerlegt, damit sie unter `msgmax` bleiben. Die
Queue wird am Ende mit `IPC_RMID` entfernt.

Diagnose: `ipcs -q`, `cat /proc/sys/kernel/msgmax`.

---

## 4) Dispatcher (Datum / Uhrzeit)

```bash
# Terminal 1
./dispatcher

# Terminal 2
./client_time      # -> Uhrzeit: HH:MM:SS
./client_date      # -> Datum:   YYYY-MM-DD
```

Eine gemeinsame Queue, Routierung ueber `mtype`:

| Richtung              | mtype           |
|-----------------------|-----------------|
| Client -> Dispatcher  | `REQ_TIME` (1) / `REQ_DATE` (2) |
| Dispatcher -> Client  | `client_pid`    |

So holt jeder Client mit `msgrcv(q, ..., getpid(), 0)` nur seine eigene
Antwort. Dispatcher raeumt die Queue bei Ctrl+C mit `IPC_RMID`.

---

## Aufraeumen

```bash
make clean       # Binaries + /tmp/chat_*
ipcrm -Q 0x...   # falls eine Queue haengen bleibt (Key in `ipcs -q`)
```
