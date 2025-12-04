#ifndef _H_COMMAND
#define _H_COMMAND

#include <fcntl.h>
#include <signal.h>
#include <sys/wait.h>
#include <util.h>

#define MAX_BREAKPOINTS 256
#define MAX_CMD_LEN 256
#define MAX_CMD_ARGC 8
#define MAX_INSTRUCTIONS 11
#define MAX_READS 6
#define INSTRUCTIONS_BUF (MAX_READS * 19) + 1

typedef struct {
    Elf64_Addr addr[MAX_BREAKPOINTS];
    long bytes[MAX_BREAKPOINTS];
    bool active[MAX_BREAKPOINTS];
    int len;
} breakpoint_t;

typedef struct {
    char *target;
    int argc;
    char *argv[MAX_CMD_ARGC];
    fn_t *fns;
    pid_t pid;
} cmd_args_t;

typedef void (*cmd_t)(cmd_args_t *cmd_args);

typedef struct {
    const char *name;
    cmd_t cmd;
} cmd_name_t;

extern cmd_name_t cmd_names[];

void breakpoint(cmd_args_t *cmd_args);
void list(cmd_args_t *cmd_args);
void delete_break(cmd_args_t *cmd_args);
void run(cmd_args_t *cmd_args);
void cont(cmd_args_t *cmd_args);
void quit(cmd_args_t *cmd_args);
void disas_wrapper(cmd_args_t *cmd_args);
void single_instruction(cmd_args_t *cmd_args);
#endif
