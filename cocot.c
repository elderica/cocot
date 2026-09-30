/*
 * cocot - COde COnverter on Tty
 *
 * Copyright (c) 2002, 2004, 2005, 2008  IWAMURO Motonori
 * All rights reserved.
 */

#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <getopt.h>
/* login_tty() */
#if HAVE_UTMP_H
#  include <utmp.h>
#endif
#if HAVE_LIBUTIL_H
#  include <libutil.h>
#endif
#if HAVE_UTIL_H
#  include <util.h>
#endif
#ifndef HAVE_LOGIN_TTY
#  include <sys/ioctl.h>
#endif

#include "init.h"
#include "loop.h"
#include "suspend.h"

static const struct option long_options[] = {
    { "help",    no_argument, NULL, 'h' },
    { "version", no_argument, NULL, 'v' },
    { NULL, 0, NULL, 0 }
};

#if DEBUG
#define DEBUG_LOG "debug.log"
FILE *debug = NULL;
#endif

static void
show_version(void)
{
    fprintf(stderr, "%s, version %s\n", PACKAGE_NAME, PACKAGE_VERSION);
    exit(1);
}

static void
usage(const char *progname)
{
    fprintf(stderr,
	    "Usage: %s [OPTIONS] [--] COMMAND ARG1 ARG2 ...\n"
	    "\n"
	    "Options:\n"
	    "    -o LOGFILE     logging all output of command.\n"
	    "    -a             append log file.\n"
	    "    -t TERM_CODE\n"
	    "    -t TERM_INPUT_CODE,TERM_OUTPUT_CODE\n"
            "                   character encoding(s) for terminal. (default is %s)\n"
	    "    -p PROC_CODE\n"
	    "    -p PROC_INPUT_CODE,PROC_OUTPUT_CODE\n"
	    "                   character encoding(s) in command process. (default is %s)\n"
	    "    -i             ignore ISO-2022-JP escape sequence.\n"
	    "    -n             no conversion. (like script(1))\n"
	    "    -h, --help     show this message.\n"
	    "    -v, --version  show version.\n",
	    progname,
	    DEFAULT_TERM_CODE,
	    DEFAULT_PROC_CODE);
    exit(1);
}

int
main(int argc, char *argv[])
{
    int c;
    const char *logfn = NULL;
    const char *logmd = "w";
    FILE *logfp = NULL;
    const char *term_input_code  = DEFAULT_TERM_CODE;
    const char *term_output_code = DEFAULT_TERM_CODE;
    const char *proc_input_code  = DEFAULT_PROC_CODE;
    const char *proc_output_code = DEFAULT_PROC_CODE;
    char *p;
    int dec_jis = 1;

    int mfd, sfd;
    int status;
    pid_t ppid, cpid, gcpid;

    if (argc == 1)
	usage(argv[0]);
    /* "+": stop at the first non-option so COMMAND's options are kept */
    while ((c = getopt_long(argc, argv, "+ao:t:p:inhv",
			    long_options, NULL)) != -1) {
	switch (c) {
	case 'a':
	    logmd = "a";
	    break;
	case 'o':
	    logfn = optarg;
	    break;
	case 't':
	    if ((p = strchr(optarg, ',')) != NULL) {
		term_input_code = optarg;
		*p++ = '\0';
		term_output_code = p;
	    } else {
		term_input_code = term_output_code = optarg;
	    }
	    break;
	case 'p':
	    if ((p = strchr(optarg, ',')) != NULL) {
		proc_input_code = optarg;
		*p++ = '\0';
		proc_output_code = p;
	    } else {
		proc_input_code = proc_output_code = optarg;
	    }
	    break;
	case 'i':
	    dec_jis = 0;
	    break;
	case 'n':
	    term_input_code = term_output_code =
	    proc_input_code = proc_output_code = NULL;
	    break;
	case 'h':
	    usage(argv[0]);
	    break;
	case 'v':
	    show_version();
	    break;
	default:
	    usage(argv[0]);
	    break;
	}
    }
    if (optind >= argc)
	usage(argv[0]);
    if (logfn) {
	if ((logfp = fopen(logfn, logmd)) == NULL)
	    fatal("Can't open file '%s' (%s).", logfn, strerror(errno));
	setvbuf(logfp, NULL, _IONBF, 0);
    }
    init(&mfd, &sfd);
    ppid = getpid();
    if ((cpid = fork()) < 0) {
	/* error */
	fatal("Can't fork process");
    } else if (cpid == 0) {
	/* child */
	close(mfd);
	if (logfp)
	    fclose(logfp);
#ifdef HAVE_LOGIN_TTY
	login_tty(sfd);
#else
	setsid();
	ioctl(sfd, TIOCSCTTY, 0);
	dup2(sfd, 0);
	dup2(sfd, 1);
	dup2(sfd, 2);
	if (sfd > 2)
	    close(sfd);
#endif
	if ((gcpid = fork()) < 0) {
	    /* error */
	    fatal("Can't fork subprocess");
	} else if (gcpid == 0) {
	    /* grandchild */
	    setfg();
	    execvp(argv[optind], argv + optind);
	    fatal("Can't exec process");
	}
	/* child */
	do {
	    if (waitpid(gcpid, &status, WUNTRACED) < 0)
		fatal("waitpid");
	    if (WIFSTOPPED(status))
		kill(ppid, SIGTSTP);
	    kill(gcpid, SIGCONT);
	} while (!WIFEXITED(status) && !WIFSIGNALED(status));
	exit(0);
    }
    /* parent */
    close(sfd);
#ifdef DEBUG
    if ((debug = fopen(DEBUG_LOG, "a")) == NULL)
	fatal("Can't open file '%s' (%s).", DEBUG_LOG, strerror(errno));
    setvbuf(debug, NULL, _IONBF, 0);
#endif
    loop(mfd, logfp,
	 term_input_code, term_output_code,
	 proc_input_code, proc_output_code, dec_jis);
    close(mfd);
    if (logfp)
	fclose(logfp);
    wait(&status);
    reset_tty();
    return 0;
}
