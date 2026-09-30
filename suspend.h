/*
 * Suspend & Resume
 */

#ifndef SUSPEND_H
#define SUSPEND_H

#include <signal.h>

extern volatile sig_atomic_t do_tstp;

int
reg_sigtstp(void);

int
rm_sigtstp(void);

void
setfg(void);

#endif /* SUSPEND_H */
