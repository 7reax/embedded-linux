# w4 — Hello World Kernel-Modul & Char-Device-Treiber

## Übersicht

| Datei | Inhalt |
|---|---|
| `hello_param.c` | Minimales Hello-World-Modul mit `name`-Parameter |
| `hello_chardev.c` | Char-Device-Treiber — erzeugt `/dev/hello` |
| `Makefile` | Baut beide Module |

---

## Voraussetzungen

```bash
sudo apt install build-essential linux-headers-$(uname -r)
```

---

## Kompilieren

```bash
make
```

Ergebnis: `hello_param.ko` und `hello_chardev.ko`

---

## hello_param — Hello World mit Parameter

### Laden

```bash
sudo insmod hello_param.ko name=Fulda
```

### Prüfen (dmesg)

```
hello_param: Hello Fulda from the Kernel!
```

### Entfernen

```bash
sudo rmmod hello_param
```

---

## hello_chardev — Char-Device-Treiber

### Konzept

```
Userspace          Kernelspace
─────────          ───────────
open("/dev/hello") → hello_open()
read(fd)           → hello_read()   → "Hello Fulda from Kernel!\n"
write(fd, "Linux") → hello_write()  → name_buf = "Linux"
close(fd)          → hello_release()
```

Der Treiber reserviert dynamisch eine **Major-Nummer** (`alloc_chrdev_region`),
registriert das Gerät (`cdev_add`) und lässt **udev** automatisch den
Device-Node `/dev/hello` anlegen (`class_create` + `device_create`).

### Laden

```bash
sudo insmod hello_chardev.ko name=Fulda
```

Prüfen:

```bash
dmesg | tail
# hello_chardev: loaded  Major=<X> Minor=0  /dev/hello
# hello_chardev: Hello Fulda from Kernel!
ls -l /dev/hello
```

### Lesen

```bash
cat /dev/hello
# Hello Fulda from Kernel!
```

### Schreiben (neuen Namen setzen)

```bash
echo "Hochschule" | sudo tee /dev/hello
cat /dev/hello
# Hello Hochschule from Kernel!
```

### Major/Minor-Nummer prüfen

```bash
cat /proc/devices | grep hello
```

### Entfernen

```bash
sudo rmmod hello_chardev
# hello_chardev: unloaded — Goodbye Hochschule!
```

---

## Unterschied w3 → w4

| Merkmal | w3 (`hello_module`) | w4 (`hello_chardev`) |
|---|---|---|
| Schnittstelle | `/proc/hello_proc` | `/dev/hello` |
| API | `proc_ops` | `file_operations` + `cdev` |
| Major-Nummer | keine | dynamisch via `alloc_chrdev_region` |
| udev-Node | nein | ja (`/dev/hello`) |
| Schreiben möglich | nein | ja (`echo "..." > /dev/hello`) |
