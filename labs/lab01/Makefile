# minibox — сборка проекта.
#
# Бинарник должен запускаться в образе `FROM scratch`, поэтому линковка
# статическая: в scratch нет ни libc, ни динамического загрузчика
# ld-linux.so, и обычный динамический бинарник там просто не стартует.

CC      ?= gcc
# _POSIX_C_SOURCE нужен обязательно: со строгим -std=c11 glibc прячет
# POSIX-объявления (getline, lstat, ssize_t, PATH_MAX), и сборка падает.
CFLAGS  ?= -std=c11 -D_POSIX_C_SOURCE=200809L \
           -Wall -Wextra -Wpedantic -Wshadow -Wconversion -O2
LDFLAGS ?= -static

BIN      := minibox
SRC      := main.c dispatcher.c cmds.c consts.c tokenizer.c
OBJ      := $(SRC:.c=.o)

TEST_BIN := tokenizer_test
TEST_SRC := tokenizer_test.c tokenizer.c

# -MMD -MP заставляют компилятор выписать зависимости от заголовков в .d;
# без них правка consts.h не вызвала бы пересборку.
DEPS := $(OBJ:.o=.d)

.PHONY: all test check-static clean

all: $(BIN)

$(BIN): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $@ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

# Тесты линкуются динамически: это инструмент разработчика, а не артефакт.
test: $(TEST_SRC)
	$(CC) $(CFLAGS) $(TEST_SRC) -o $(TEST_BIN)
	./$(TEST_BIN)

# Проверка главного требования ТЗ: зависимостей быть не должно.
check-static: $(BIN)
	@if ldd $(BIN) 2>&1 | grep -q 'not a dynamic executable'; then \
	    echo "OK: $(BIN) статический"; \
	else \
	    echo "ОШИБКА: $(BIN) динамический, в FROM scratch не запустится"; \
	    ldd $(BIN); \
	    exit 1; \
	fi

clean:
	rm -f $(BIN) $(OBJ) $(DEPS) $(TEST_BIN)

-include $(DEPS)
