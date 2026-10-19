#include "dispatcher.h"
#include "cmds.h"

static const struct cmd commands[] = {
    {CMD_ECHO, cmd_echo, "echo ARG..."},
    {CMD_CAT, cmd_cat, "cat FILE..."},
    {CMD_CP, cmd_cp, "cp SOURCE DESTINATION"},
    {CMD_CD, cmd_cd, "cd PATH"},
    {CMD_LS, cmd_ls, "ls [PATH]"},
    {CMD_MKDIR, cmd_mkdir, "mkdir PATH"},
    {CMD_RM, cmd_rm, "rm [-r] PATH"},
    {CMD_STAT, cmd_stat, "stat PATH"},
};

cmd_fn get_cmd(cmd_name name) {
    size_t amount_commands = sizeof commands / sizeof commands[0];
    for (size_t i = 0; i < amount_commands; i++) {
        if (commands[i].name == name) {

            return commands[i].fn;
        }
    }

    return cmd_nullable;
}

void(process_cmd)(cmd_fn cmd, int argc, char **argv) {
    cmd(argc, argv);
}
