<h1 align=center>1s0lat3 - Run your programs in separate namespaces</h1>
<p align=center><img src="https://shields.io/badge/Made_With-C-green" />
<img src="https://shields.io/badge/Compiled_With-GCC-yellow" />
  <img src="https://shields.io/badge/Tested_On-Ubuntu_20.04.3_LTS_x86__64 -red" />
</p>

<h2>Why use Docker when you can just is0lat3?</h2>

Run a process in separate Namespaces to provide isolation for the processes. Namespaces isolation implemented so far:
- `uts`: Unix Timesharing (UTS) namespaces provide isolation for the hostname and domain name.

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
# ./bin/isolate /bin/zsh 
[+] 1s0lat3 by @whokilleddb
[>] Command to be run in 1s0lati0n: /bin/sh 
# ls -la /proc/$$/ns/uts
lrwxrwxrwx 1 root root 0 Jan 21 22:35 /proc/6804/ns/uts -> 'uts:[4026532250]'
# exit
[+] Bye :D
```

## Cleanup
```bash
$ make clean
```