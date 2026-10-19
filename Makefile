# OS course helper.
#
# Edit on macOS, build and run on Linux inside Docker.
# Every target below transparently re-enters the container when run from the host.

F      ?= main.c
BIN    ?= build/$(basename $(notdir $(F)))
# make predefines CC=cc, so ?= would not take effect here
ifeq ($(origin CC),default)
  CC := gcc
endif
STD    ?= c11
CFLAGS ?= -std=$(STD) -Wall -Wextra -Wpedantic -Wshadow -Wconversion -g3 -O0 -D_GNU_SOURCE
LDLIBS ?= -pthread
ARGS   ?=

COMPOSE := docker compose
SVC     := os

# Are we already inside the container?
ifneq (,$(wildcard /.dockerenv))
  IN := 
else
  IN := $(COMPOSE) exec -T $(SVC)
endif

.PHONY: help up down sh rebuild build run asan tsan dbg val strace cc fmt clean nbuild nrun nasan

help: ## show this help
	@grep -hE '^[a-z-]+:.*?## ' $(MAKEFILE_LIST) | \
	  awk -F':.*?## ' '{printf "  \033[36m%-10s\033[0m %s\n", $$1, $$2}'
	@echo ""
	@echo "  Pick the file with F=, pass program args with ARGS=:"
	@echo "    make run F=labs/lab01/main.c ARGS='1 2 3'"

up: ## start the Linux container (idempotent)
	$(COMPOSE) up -d --build

down: ## stop and remove the container
	$(COMPOSE) down

rebuild: ## rebuild the image from scratch
	$(COMPOSE) build --no-cache && $(COMPOSE) up -d

sh: ## interactive shell inside the container
	$(COMPOSE) exec $(SVC) bash

build: ## compile $(F)
	@$(IN) mkdir -p $(dir $(BIN))
	$(IN) $(CC) $(CFLAGS) $(F) -o $(BIN) $(LDLIBS)

run: build ## compile and run $(F)
	@echo "--- $(BIN) ---"
	@$(IN) $(BIN) $(ARGS)

asan: ## run $(F) under AddressSanitizer + UBSan
	@$(IN) mkdir -p $(dir $(BIN))
	$(IN) $(CC) $(CFLAGS) -fsanitize=address,undefined -fno-omit-frame-pointer $(F) -o $(BIN).asan $(LDLIBS)
	@$(IN) $(BIN).asan $(ARGS)

tsan: ## run $(F) under ThreadSanitizer (data races)
	@$(IN) mkdir -p $(dir $(BIN))
	$(IN) $(CC) $(CFLAGS) -fsanitize=thread $(F) -o $(BIN).tsan $(LDLIBS)
	@$(IN) $(BIN).tsan $(ARGS)

val: build ## run $(F) under Valgrind memcheck
	$(COMPOSE) exec $(SVC) valgrind --leak-check=full --track-origins=yes --show-leak-kinds=all $(BIN) $(ARGS)

dbg: build ## debug $(F) in gdb
	$(COMPOSE) exec $(SVC) gdb --args $(BIN) $(ARGS)

strace: build ## trace syscalls of $(F)
	$(COMPOSE) exec $(SVC) strace -f -tt -T $(BIN) $(ARGS)

# --- native macOS build (no Docker) ------------------------------------------
# Only for portable POSIX code. Linux-only APIs (epoll, statx, mount, /proc,
# getdents64) will not compile here — use `make run` for those.
NCC     ?= clang
NBIN    ?= build/native/$(basename $(notdir $(F)))
NCFLAGS ?= -std=$(STD) -Wall -Wextra -Wpedantic -Wshadow -Wconversion -g3 -O0 -D_DARWIN_C_SOURCE

nbuild: ## compile $(F) natively on macOS
	@mkdir -p $(dir $(NBIN))
	$(NCC) $(NCFLAGS) $(F) -o $(NBIN)

nrun: nbuild ## compile and run $(F) natively on macOS
	@echo "--- $(NBIN) (native) ---"
	@$(NBIN) $(ARGS)

nasan: ## run $(F) natively under AddressSanitizer + UBSan
	@mkdir -p $(dir $(NBIN))
	$(NCC) $(NCFLAGS) -fsanitize=address,undefined -fno-omit-frame-pointer $(F) -o $(NBIN).asan
	@$(NBIN).asan $(ARGS)

cc: ## regenerate compile_commands.json for clangd
	$(IN) bear --output compile_commands.json -- $(CC) $(CFLAGS) $(F) -o /tmp/cc_probe $(LDLIBS)

fmt: ## format all C sources on the host
	@find . -name '*.c' -o -name '*.h' | grep -v '^./build' | xargs clang-format -i
	@echo "formatted"

clean: ## remove build artifacts
	rm -rf build
