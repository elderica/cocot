/*
 * Main Loop
 *
 * Copyright (c) 2002  IWAMURO Motonori
 * All rights reserved.
 */

#ifndef LOOP_H
#define LOOP_H

#include <stdio.h>

void
loop(int mfd, FILE *fp,
     const char *term_input_code, const char *term_output_code,
     const char *proc_input_code, const char *proc_output_code,
     int dec_jis);

#endif /* LOOP_H */
