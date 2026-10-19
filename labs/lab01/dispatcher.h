#ifndef DISPATCHER_H
#define DISPATCHER_H
#include <stddef.h>
#include "cmds.h"

cmd_fn get_cmd(cmd_name name);
void process_cmd(cmd_fn cmd, int argc, char **argv);
#endif
