#ifndef TOKENIZER_H
#define TOKENIZER_H
#include <stddef.h>
#define MAX_ARGS 64

int(tokenize)(size_t len, char *line, char **out);

#endif
