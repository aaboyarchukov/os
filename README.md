# OS course workspace

Edit on macOS in Zed, compile and run on Linux inside Docker.

## Layout

```
.clangd            clangd config — targets Linux via ./sysroot
.clang-format      formatting rules
.zed/tasks.json    build/run/debug tasks (cmd-shift-r)
.zed/settings.json project-local clangd + format-on-save
Dockerfile         Ubuntu 24.04 + gcc, gdb, valgrind, strace, man pages
docker-compose.yml mounts the repo at /work, keeps the container alive
Makefile           every target runs inside the container
tools/sync-sysroot.sh  copies Linux headers out of the container for clangd
labs/              one directory per lab
```

## First run

```bash
make up                  # build image, start container
./tools/sync-sysroot.sh  # populate ./sysroot for clangd
```

## Daily use

```bash
make run    F=labs/lab01/main.c          # compile + run on Linux
make run    F=labs/lab01/main.c ARGS='a b'
make asan   F=labs/lab01/main.c          # AddressSanitizer + UBSan
make tsan   F=labs/lab01/main.c          # ThreadSanitizer (data races)
make val    F=labs/lab01/main.c          # valgrind memcheck
make strace F=labs/lab01/main.c          # syscall trace
make dbg    F=labs/lab01/main.c          # gdb
make sh                                  # shell in the container
make fmt                                 # clang-format everything
make help                                # all targets
```

In Zed the same targets are tasks: `cmd-shift-r` -> pick one. They act on the
currently open file.

## Why the sysroot

clangd runs natively on macOS, so by default it sees the macOS SDK and marks
every Linux-only header as missing. `tools/sync-sysroot.sh` copies the
container's `/usr/include` and gcc builtin headers into `./sysroot`, and
`.clangd` points clang at them with `--target=aarch64-unknown-linux-gnu
-nostdinc`. Result: `epoll`, `inotify`, `futex`, `pthread_barrier_t`,
`/proc` helpers all resolve correctly in the editor.

Re-run the script after installing new `-dev` packages in the image, then
restart the language server in Zed (`cmd-shift-p` -> "editor: restart language
server").

## Notes

- The container is `aarch64` (Apple Silicon). If a lab needs x86-64 (inline
  asm, specific ABI behaviour), add `platform: linux/amd64` to the service in
  `docker-compose.yml` and re-run the sysroot script — it will be emulated and
  slower.
- `gdb`, `strace` and `ltrace` need `SYS_PTRACE` and an unconfined seccomp
  profile; both are already set in `docker-compose.yml`.
- `./sysroot`, `./build` and `compile_commands.json` are gitignored.
