#!/usr/bin/env bash
# Copy the container's Linux headers to ./sysroot so clangd on macOS can resolve
# Linux-only headers (sys/epoll.h, sys/inotify.h, linux/*, glibc internals).
#
# Re-run after installing new -dev packages in the image.
set -euo pipefail

cd "$(dirname "$0")/.."
CONTAINER=os-course

docker compose up -d >/dev/null

GCC_INC=$(docker compose exec -T os gcc -print-file-name=include | tr -d '\r')
GCC_FIXED=$(docker compose exec -T os gcc -print-file-name=include-fixed | tr -d '\r')

rm -rf sysroot
mkdir -p sysroot/usr sysroot/gcc

echo "==> copying /usr/include"
docker cp "$CONTAINER:/usr/include" sysroot/usr/include

echo "==> copying gcc builtin headers ($GCC_INC)"
docker cp "$CONTAINER:$GCC_INC" sysroot/gcc/include

if docker compose exec -T os test -d "$GCC_FIXED" 2>/dev/null; then
  echo "==> copying gcc include-fixed"
  docker cp "$CONTAINER:$GCC_FIXED" sysroot/gcc/include-fixed
fi

echo "==> done: $(du -sh sysroot | cut -f1) in ./sysroot"
echo "    restart clangd in Zed:  cmd-shift-p -> 'editor: restart language server'"
