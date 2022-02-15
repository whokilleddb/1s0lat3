<h1 align=center>1s0lat3 - Run your programs in separate namespaces</h1>
<p align=center><img src="https://shields.io/badge/Made_With-C-green" />
<img src="https://shields.io/badge/Compiled_With-GCC-yellow" />
  <img src="https://shields.io/badge/Tested_On-Ubuntu_20.04.3_LTS_x86__64 -red" />
</p>

<p align=center>
<a href="https://asciinema.org/a/850Ja2A5ESQoVOGMMuqMKMTPh" target="_blank"><img src="https://asciinema.org/a/850Ja2A5ESQoVOGMMuqMKMTPh.svg" /></a>
</p>

<h2>Why use Docker when you can just is0lat3?</h2>


Run a process in separate Namespaces to provide isolation for the processes. Namespaces isolation implemented so far:
- `uts`
- `user`
- `network`
- `pid`
- `mount`

## How to compile?
```bash
$ git clone https://github.com/whokilleddb/1s0lat3
$ cd 1s0lat3
$ make
```

## Run a process
Syntax:
> ./isolate \<command with flags\>

Example: 
```bash
$ sudo ./isolate /bin/sh
[+] 1s0lat3 by @whokilleddb
[>] Command to be run in 1s0lati0n: /bin/sh 
[i] Successfully created UTS namespace
[i] Successfully created USER namespace
[i] Successfully created NETWORK namespace
[i] Successfully created PID namespace
[i] Successfully created MOUNT namespace
/ # cat /etc/os-release 
NAME="Alpine Linux"
ID=alpine
VERSION_ID=3.15.0
PRETTY_NAME="Alpine Linux v3.15"
HOME_URL="https://alpinelinux.org/"
BUG_REPORT_URL="https://bugs.alpinelinux.org/"
/ # exit
[+] Bye :D
```

## Cleanup
```bash
$ make clean
```

# Notes
- You will need `libnl` library, especially the routing family of functions

# To-Do
- Add routing inside Name-space
- Configure IP Tables to allow internet access
