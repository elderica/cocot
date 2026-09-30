/*
 * Register & Remove SIGWINCH handler
 *
 * Copyright (c) 2002  IWAMURO Motonori
 * All rights reserved.
 */

#include <config.h>
#include <unistd.h>
#include <termios.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <errno.h>

#include "sigwinch.h"

static struct sigaction oact;
static int master_fileno;

static void
sigwinch(int unused)
{
    struct winsize win;
    int saved_errno = errno;

    ioctl(STDIN_FILENO,  TIOCGWINSZ, &win);
    ioctl(master_fileno, TIOCSWINSZ, &win);
    errno = saved_errno;
}

int
reg_sigwinch(int fd)
{
    struct sigaction act;

    master_fileno = fd;
    act.sa_handler = sigwinch;
    sigemptyset(&act.sa_mask);
    act.sa_flags = SA_RESTART;
    return sigaction(SIGWINCH, &act, &oact);
}

int
rm_sigwinch(void)
{
    struct sigaction act;
    return sigaction(SIGWINCH, &oact, &act);
}
