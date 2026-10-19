#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "cmds.h"
#include "dispatcher.h"

#include "tokenizer.h"

int main(void) {
    /* cat и cp пишут в stdout напрямую через write(), а ls и stat — через
       printf, который буферизован. При перенаправлении вывода в файл stdout
       буферизуется блоками, и вывод cat обгонял бы вывод ls. Отключаем
       буферизацию, чтобы порядок строк совпадал с порядком команд.
       (буферизация stdio — week03, часть 2) */
    setvbuf(stdout, NULL, _IONBF, 0);

    char *line = NULL;
    size_t cap = 0;
    ssize_t len;

    // необходимо считать строку
    // затем спарсить аргументы
    // 0 - команда
    // 1... - аргументы которые с командой
    // у каждой функции разные аргументы

    while ((len = getline(&line, &cap, stdin)) != -1) {
        if (len > 0 && line[len - 1] == '\n') {
            line[len - 1] = '\0';
            len = len - 1;
        }

        char *cmd_buff[MAX_ARGS];
        int cells_count = tokenize((size_t)len, line, cmd_buff);

        if (cells_count < 0) {
            fprintf(stderr, "слишком много аргументов (максимум %d)\n", MAX_ARGS);
            continue;
        }

        if (cells_count == 0) {
            continue;
        }

        cmd_name name = convert_to_enum(cmd_buff[0]);
        cmd_fn cmd = get_cmd(name);
        process_cmd(cmd, cells_count, cmd_buff);
    }

    free(line);

    return 0;
}
