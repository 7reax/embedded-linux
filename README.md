# Embedded Linux Projekt

## Connect on Windows

1) Check USB Serial Converter is shared

Command: `usbipd list`

2) WSL attach

Command: `usbipd attach --wsl --busid [BUS-ID]`

3) WSL & Picocom

Command: `picocom -b 115200 [USB Serial Port]

## Connect on Mac

1) List all USB Serial 

Command: `ls /dev/cu.usbserial-*`

2) Connect with picocom

Command: `picocom -b 115200 /dev/cu.usbserial-XYZ`

## User
Username: c4
PSW: 1234
WD: /root/home/embedded-linux

Command:
- `su` mit PSW (1234)