/*
 *    SPDX-FileCopyrightText: 2024 Matheus Pereira Dias <matheuspd07@gmail.com>
 *   
 *    SPDX-License-Identifier: GPL-3.0-or-later
 *
 *    This file is part of SYSeg, available at https://gitlab.com/monaco/syseg.
 */

#ifndef OPT_H
#define OPT_H

int __attribute__((fastcall)) strcmp(const char *, const char *);
int __attribute__((fastcall)) strlen(const char *);
int __attribute__((fastcall)) itoa(unsigned int, char *);

#endif  /* OPT_H  */
