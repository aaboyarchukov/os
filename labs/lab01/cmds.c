#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdbool.h>

#include "cmds.h"
#include "consts.h"

#define COPY_BUF_SIZE 65536

cmd_name convert_to_enum(char arg[]) {
    if (strcmp(arg, ECHO) == 0) {
        return CMD_ECHO;
    }

    if (strcmp(arg, CAT) == 0) {
        return CMD_CAT;
    }

    if (strcmp(arg, CP) == 0) {
        return CMD_CP;
    }

    if (strcmp(arg, CD) == 0) {
        return CMD_CD;
    }

    if (strcmp(arg, LS) == 0) {
        return CMD_LS;
    }

    if (strcmp(arg, MKDIR) == 0) {
        return CMD_MKDIR;
    }

    if (strcmp(arg, RM) == 0) {
        return CMD_RM;
    }

    if (strcmp(arg, STAT) == 0) {
        return CMD_STAT;
    }

    return CMD_DEFAULT;
}

static int writen(int fd, const char *buf, size_t n) {
    while (n > 0) {
        ssize_t w = write(fd, buf, n);
        if (w < 0) {
            if (errno == EINTR) {
                continue;
            }
            return -1;
        }
        buf += w;
        n -= (size_t) w;
    }

    return 0;
}

/* Перелить всё из src_fd в dst_fd. Работает с бинарными данными: никакого
   stdio и никакой интерпретации '\0' или переводов строк. */
static int copy_fd(int src_fd, int dst_fd, const char *cmd, const char *src_name) {
    char buf[COPY_BUF_SIZE];

    while (true) {
        ssize_t r = read(src_fd, buf, sizeof buf);
        if (r == 0) {
            return 0;
        }
        if (r < 0) {
            if (errno == EINTR) {
                continue;
            }
            fprintf(stderr, "%s: %s: %s\n", cmd, src_name, strerror(errno));
            return -1;
        }
        if (writen(dst_fd, buf, (size_t) r) < 0) {
            fprintf(stderr, "%s: ошибка записи: %s\n", cmd, strerror(errno));
            return -1;
        }
    }
}

/* echo ARG... — вывести аргументы через пробел и перевод строки. */
int cmd_echo(int argc, char **argv) {
    for (int i = 1; i < argc; i++) {
        if (i > 1 && writen(STDOUT_FILENO, " ", 1) < 0) {
            fprintf(stderr, "%s: %s\n", argv[0], strerror(errno));
            return 1;
        }
        if (writen(STDOUT_FILENO, argv[i], strlen(argv[i])) < 0) {
            fprintf(stderr, "%s: %s\n", argv[0], strerror(errno));
            return 1;
        }
    }

    if (writen(STDOUT_FILENO, "\n", 1) < 0) {
        fprintf(stderr, "%s: %s\n", argv[0], strerror(errno));
        return 1;
    }

    return 0;
}

/* cat FILE... — вывести содержимое файлов на стандартный вывод. */
int cmd_cat(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "использование: cat FILE...\n");
        return 1;
    }

    int status = 0;

    for (int i = 1; i < argc; i++) {
        int fd = open(argv[i], O_RDONLY);
        if (fd < 0) {
            fprintf(stderr, "cat: %s: %s\n", argv[i], strerror(errno));
            status = 1;
            continue;
        }

        if (copy_fd(fd, STDOUT_FILENO, "cat", argv[i]) < 0) {
            status = 1;
        }

        if (close(fd) < 0) {
            fprintf(stderr, "cat: %s: %s\n", argv[i], strerror(errno));
            status = 1;
        }
    }

    return status;
}

/* cp SOURCE DESTINATION — скопировать файл, сохранив права доступа. */
int cmd_cp(int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr, "использование: cp SOURCE DESTINATION\n");
        return 1;
    }

    const char *src_name = argv[1];
    const char *dst_name = argv[2];

    int src_fd = open(src_name, O_RDONLY);
    if (src_fd < 0) {
        fprintf(stderr, "cp: %s: %s\n", src_name, strerror(errno));
        return 1;
    }

    struct stat st;
    mode_t mode = 0644;
    if (fstat(src_fd, &st) == 0) {
        mode = st.st_mode & 07777;
    }

    int dst_fd = open(dst_name, O_WRONLY | O_CREAT | O_TRUNC, mode);
    if (dst_fd < 0) {
        fprintf(stderr, "cp: %s: %s\n", dst_name, strerror(errno));
        close(src_fd);
        return 1;
    }

    int status = copy_fd(src_fd, dst_fd, "cp", src_name) < 0 ? 1 : 0;

    close(src_fd);

    if (close(dst_fd) < 0) {
        fprintf(stderr, "cp: %s: %s\n", dst_name, strerror(errno));
        status = 1;
    }

    return status;
}

/* cd PATH — сменить текущий каталог.
   Рабочий каталог — атрибут процесса, поэтому между командами он
   сохраняется сам собой, хранить его в переменной не нужно. */
int cmd_cd(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "использование: cd PATH\n");
        return 1;
    }

    if (chdir(argv[1]) < 0) {
        fprintf(stderr, "cd: %s: %s\n", argv[1], strerror(errno));
        return 1;
    }

    return 0;
}

/* ls [PATH] — вывести все файлы каталога, включая скрытые.
   Записи "." и ".." тоже выводятся: по ТЗ нужны все. */
int cmd_ls(int argc, char **argv) {
    if (argc > 2) {
        fprintf(stderr, "использование: ls [PATH]\n");
        return 1;
    }

    const char *path = (argc == 2) ? argv[1] : ".";

    DIR *dir = opendir(path);
    if (dir == NULL) {
        fprintf(stderr, "ls: %s: %s\n", path, strerror(errno));
        return 1;
    }

    int status = 0;

    while (true) {
        errno = 0;
        struct dirent *entry = readdir(dir);
        if (entry == NULL) {
            if (errno != 0) {
                fprintf(stderr, "ls: %s: %s\n", path, strerror(errno));
                status = 1;
            }
            break;
        }

        printf("%s\n", entry->d_name);
    }

    if (closedir(dir) < 0) {
        fprintf(stderr, "ls: %s: %s\n", path, strerror(errno));
        status = 1;
    }

    return status;
}

/* mkdir PATH — создать каталог.
   Абсолютный или относительный путь ядро разбирает само. */
int cmd_mkdir(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "использование: mkdir PATH\n");
        return 1;
    }

    if (mkdir(argv[1], 0777) < 0) {
        fprintf(stderr, "mkdir: %s: %s\n", argv[1], strerror(errno));
        return 1;
    }

    return 0;
}

/* rm [-r] PATH — удалить файл, с флагом -r — каталог рекурсивно. */
int cmd_rm(int argc, char **argv) {
    (void) argc;
    (void) argv;

    return 0;
}

/* stat PATH — размер, тип, число жёстких ссылок, права в восьмеричном виде. */
int cmd_stat(int argc, char **argv) {
    (void) argc;
    (void) argv;

    return 0;
}

/* Заглушка для нераспознанной команды. Имя команды доступно в argv[0] —
   ровно поэтому буфер передаётся целиком, вместе с нулевым токеном. */
int cmd_nullable(int argc, char **argv) {
    (void) argc;
    fprintf(stderr, "%s: команда не найдена\n", argv[0]);

    return 127;
}
