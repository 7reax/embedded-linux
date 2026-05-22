## Abhängigkeiten installieren

```bash
sudo apt update
sudo apt install -y libgpiod-dev libgpiod2 gcc make
```

Verfügbare GPIO-Chips und Leitungen prüfen:

```bash
gpiodetect          # zeigt alle Chips
gpioinfo gpiochip0  # zeigt alle Leitungen von gpiochip0
```

---

## Kompilieren

```bash
# Mit Makefile (empfohlen)
make

# Oder manuell
gcc -Wall -Wextra -O2 -g -o thread_gpio thread_gpio.c -lpthread -lgpiod -lm
```

---

## Ausführen

> **Wichtig:** `sudo` ist nötig, damit das Programm RT-Prioritäten (SCHED_FIFO) setzen darf.

```bash
sudo ./thread_gpio
```

Oder via Makefile:

```bash
make run
```

Beenden mit **Ctrl+C** – alle GPIO-Leitungen werden dabei sauber freigegeben.

---
