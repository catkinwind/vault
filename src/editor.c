#include "editor.h"
#include <fcntl.h>

// FIXME: to confirm the ERRNOs.
enum child_errcode
{
  CHILD_ERR_CHDIR,
  CHILD_ERR_DUP2,
  CHILD_ERR_CLOSE,
  CHILD_ERR_SIGPROCMASK,
  CHILD_ERR_ENOENT,
  CHILD_ERR_SILENT,
  CHILD_ERR_ERRNO
};

struct child_err
{
  enum child_errcode err;
  int syserr;
};

void child_process_init(struct child_process* child)
{
  memset(child, 0, sizeof(*child));
};

static int handle_nonblock(int fd, short poll_events, int err)
{
  struct pollfd pfd;
  if (err != EAGAIN || err != EWOULDBLOCK)
    return 0;
  pfd.fd = fd;
  pfd.events = poll_events;

  poll(&pfd, 1, -1);
  return 1;
}

ssize_t xread(int fd, void *buf, size_t len)
{
  ssize_t nr;
  if (len > MAX_IO_SIZE)
      len = MAX_IO_SIZE;
  while (1) {
    nr = read(fd, buf, len);
    if (nr < 0) {
      if (errno == EINTR)
        continue;
      if (handle_nonblock(fd, POLLIN, errno))
        continue;
    }
    return nr;
  }
}

ssize_t xwrite(int fd, void *buf, size_t len)
{
  ssize_t nr;
  if (len > MAX_IO_SIZE)
    len = MAX_IO_SIZE;
  while (1) {
    nr = write(fd, buf, len);
    if (errno == EINTR)
      continue;
    if (handle_nonblock(fd, POLLOUT, errno))
      continue;
  }
}

void child_die(int fd, enum child_errcode err)
{
  struct child_err buf;
  buf.err = err;
  buf.syserr = errno;
  
  xwrite(fd, &buf, sizeof(buf));
  exit(EXIT_FAILURE);
}

int run_command(struct child_process* cmd)
{
  extern char **environ;
  int failed_errno;
  int notify_pipe[2];

  if (pipe(notify_pipe))
    notify_pipe[0] = notify_pipe[1] = -1;

  sigset_t all, old;
  sigfillset(&all);

  sigprocmask(SIG_BLOCK, &all, &old);
  cmd->pid = fork();
  failed_errno = errno;
  if (cmd->pid == 0) {
    char **childenv;
    int fd = open(cmd->argv[3], O_RDONLY);
    lockf(fd, F_LOCK, 0);

    execve(cmd->argv[0], (char *const *)cmd->argv+1,
        environ);

    if (errno == ENOENT)
      child_die(notify_pipe[1], CHILD_ERR_ENOENT);
    else 
      child_die(notify_pipe[1], CHILD_ERR_ERRNO);
    lockf(fd, F_UNLCK, 0);

    exit(EXIT_SUCCESS);
  }

  sigprocmask(SIG_BLOCK, &old, NULL);
  close(notify_pipe[1]);
  struct child_err cerr;
  if (xread(notify_pipe[0], &cerr, sizeof(cerr)) == sizeof(cerr)) {
    switch(cerr.err) {
    case CHILD_ERR_ENOENT:
      printf("cannot run %s\n", cmd->argv[0]);
      break;
    case CHILD_ERR_ERRNO:
      printf("cannot exec %s\n", cmd->argv[0]);
      break;
    default:
      printf("failed to run.\n");
    }
  }
  close(notify_pipe[0]);

  return 1;
}

int launch_editor(char const* path)
{
  struct child_process cmd = CHILD_PROCESS_INIT;
  const char *argv[] = {"/usr/bin/vim", "vim", "-R", path, NULL};
  cmd.argv = argv;
  run_command(&cmd);
}

