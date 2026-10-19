#ifndef CMDS_H
#define CMDS_H

enum cmd_name {
    CMD_ECHO,
    CMD_CAT,
    CMD_CP,
    CMD_CD,
    CMD_LS,
    CMD_MKDIR,
    CMD_RM,
    CMD_STAT,
    CMD_STOP,
    CMD_DEFAULT,
};

typedef enum cmd_name cmd_name;

typedef int (*cmd_fn)(int argc, char **argv);

struct cmd {
    cmd_name name;
    cmd_fn fn;
    const char *usage;
};

cmd_name convert_to_enum(char arg[]);

int cmd_echo(int argc, char **argv);
int cmd_cat(int argc, char **argv);
int cmd_cp(int argc, char **argv);
int cmd_cd(int argc, char **argv);
int cmd_ls(int argc, char **argv);
int cmd_mkdir(int argc, char **argv);
int cmd_rm(int argc, char **argv);
int cmd_stat(int argc, char **argv);
int cmd_nullable(int argc, char **argv);

#endif
