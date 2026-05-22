CC      = gcc
CFLAGS  = -Wall -Wextra -O2 -g
LIBS    = -lpthread -lgpiod -lm
TARGET  = thread_gpio
SRC     = thread_gpio.c

.PHONY: all clean install-deps

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $@ $< $(LIBS)

# Abhängigkeiten installieren
install-deps:
	sudo apt install -y libgpiod-dev libgpiod2

# Mit Root-Rechten ausführen (RT-Prioritäten)
run: $(TARGET)
	sudo ./$(TARGET)

clean:
	rm -f $(TARGET)
