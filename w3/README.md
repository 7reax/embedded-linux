
# Hello World Kernel Module mit Parameter

## 📁 Projektverzeichnis

```
embedded_linux/w3
```

## 📦 Inhalt des Projekts

- `hello_module.c`  
  Kernel-Modul mit Parameter und `/proc`-Eintrag  

- `Makefile`  
  Zum Kompilieren des Kernel-Moduls  

- `embedded_linux.c`  
  Userspace-Programm zum Lesen von `/proc`  

- `hello.service`  
  systemd Service-Datei  

---

## ⚙️ Voraussetzungen

- Linux-System  
- installierte Kernel-Header  
- `gcc`, `make`  
- Root-Rechte  

Installation (falls notwendig):

```bash
sudo apt install build-essential linux-headers-$(uname -r)
```

---

## 🔨 Kernel-Modul kompilieren

Im Verzeichnis `embedded_linux/w3`:

```bash
make
```

Ergebnis:

```
hello_module.ko
```

---

## 🚀 Kernel-Modul laden

```bash
sudo insmod hello_module.ko name=Fulda
```

Überprüfung:

```bash
dmesg | tail
```

---

## 📄 /proc Eintrag testen

```bash
cat /proc/hello_proc
```

Beispielausgabe:

```
Hello Fulda from Kernel!
```

---

## 👨‍💻 Userspace-Programm kompilieren

```bash
gcc embedded_linux.c -o embedded_linux
```

Programm installieren:

```bash
sudo mv embedded_linux /usr/local/bin/
```

Testlauf:

```bash
/usr/local/bin/embedded_linux
```

---

## ⚙️ systemd Service einrichten

Service kopieren:

```bash
sudo cp hello.service /etc/systemd/system/
```

Systemd neu laden:

```bash
sudo systemctl daemon-reload
```

Service aktivieren:

```bash
sudo systemctl enable hello.service
```

Service starten:

```bash
sudo systemctl start hello.service
```

Status prüfen:

```bash
systemctl status hello.service
```

---

## 🔁 Systemverhalten

- Kernel-Modul erstellt `/proc/hello_proc`  
- Parameter `name` beeinflusst die Ausgabe  
- Userspace-Programm liest regelmäßig aus `/proc`  
- systemd startet das Programm automatisch beim Boot  

---

## 🧹 Aufräumen

Kernel-Modul entfernen:

```bash
sudo rmmod hello_module
```

Build-Dateien löschen:

```bash
make clean
```

Service stoppen:

```bash
sudo systemctl stop hello.service
sudo systemctl disable hello.service
```

---

## ✅ Beispiel

```bash
sudo insmod hello_module.ko name=Hochschule
cat /proc/hello_proc
```

Ausgabe:

```
Hello Hochschule from Kernel!
```
