<h1 align=center>1s0lat3 - Run your programs in separate namespaces</h1>
<p align=center><img src="https://shields.io/badge/Made_With-C-green" />
<img src="https://shields.io/badge/Compiled_With-GCC-yellow" />
  <img src="https://shields.io/badge/Tested_On-Ubuntu_20.04.3_LTS_x86__64 -red" />
</p>

<h2>Why use Docker when you can just is0lat3?</h2>

Run a process in separate Namespaces to provide isolation for the processes. Namespaces isolation implemented so far:
- `uts`
- `user`
- `network`
- `pid`
-`mount`

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
/ # whoami
root
/ # ls -la /proc/$$/ns/
total 0
dr-x--x--x    2 root     root             0 Feb 11 16:44 .
dr-xr-xr-x    9 root     root             0 Feb 11 16:44 ..
lrwxrwxrwx    1 root     root             0 Feb 11 16:44 cgroup -> cgroup:[4026531835]
lrwxrwxrwx    1 root     root             0 Feb 11 16:44 ipc -> ipc:[4026531839]
lrwxrwxrwx    1 root     root             0 Feb 11 16:44 mnt -> mnt:[4026532250]
lrwxrwxrwx    1 root     root             0 Feb 11 16:44 net -> net:[4026532254]
lrwxrwxrwx    1 root     root             0 Feb 11 16:44 pid -> pid:[4026532252]
lrwxrwxrwx    1 root     root             0 Feb 11 16:44 pid_for_children -> pid:[4026532252]
lrwxrwxrwx    1 root     root             0 Feb 11 16:44 user -> user:[4026532249]
lrwxrwxrwx    1 root     root             0 Feb 11 16:44 uts -> uts:[4026532251]
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