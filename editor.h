#ifndef _VAULT_EDITOR

#define _VAULT_EDITOR

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <poll.h>
#include <errno.h>

static const char *empty_argv[] = { NULL };

#define MAX_IO_SIZE 0x200000000

#define CHILD_PROCESS_INIT { NULL }

struct child_process
{
  const char **argv;
  pid_t pid;
  int in: 1;
  int out: 1;
  int error: 1;
};

int launch_editor(char const* path);

#endif /* _VAULT_EDITOR */
