/*
 *    SPDX-FileCopyrightText: 2021 Monaco F. J. <monaco@usp.br>
 *    SPDX-FileCopyrightText: 2024 Matheus Pereira Dias <matheuspd07@gmail.com>
 *   
 *    SPDX-License-Identifier: GPL-3.0-or-later
 *
 *  This file is a derivative work from SYSeg (https://gitlab.com/monaco/syseg)
 *  and contains modifications carried out by the following author(s):
 *  Matheus Pereira Dias <matheuspd07@gmail.com>
 */

#include "bios.h"
#include "opt.h"

#define PROMPT "$ "		/* Prompt sign. */
#define SIZE 20	/* Read buffer size. */

char buffer[SIZE];		/* Read buffer. */
int len = 0;

int main() {
  clear();  
  println("Boot Cmd 1.0");

  while (1) {    	
		print(PROMPT);		/* Show prompt. */
    	readln(buffer);		/* Read use input. */
    	if (buffer[0]) {		/* Execute built-in command. */
		    if (!strcmp(buffer,"strlen")) {				
		      	readln(buffer);
				print("Str: ");
				println(buffer);
				len = strlen(buffer);
				itoa(len, buffer);
				print("Len: ");
				println(buffer);
    	    }
		    else println("Not found.");
		}
  	}  
  	return 0;
}