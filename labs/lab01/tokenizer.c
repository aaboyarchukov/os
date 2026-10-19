#include <string.h>
#include <stdbool.h>

#include "tokenizer.h"

int tokenize(size_t len_of_line, char *line, char **out) {
    int n = 0;
    int is_separator = true;

    for (size_t i = 0; i < len_of_line; i++) {
        if (n >= MAX_ARGS) {
            return -1;
        }

        if (line[i] == ' ' || line[i] == '\t') {
            line[i] = '\0';
            is_separator = true;
            continue;
        }

        if (is_separator) {
            out[n++] = line + i;
            is_separator = false;
        }
    }

    return n;
}
