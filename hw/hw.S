#    SPDX-FileCopyrightText: 2024 Matheus Pereira Dias <matheuspd07@gmail.com>
#   
#    SPDX-License-Identifier: GPL-3.0-or-later
#
#    This file is part of SYSeg, available at https://gitlab.com/monaco/syseg.

# Exercise 1: Bare Metal Hello World

	.code16                 # Set 16-bit code	
	.global _start	        # Default entry point (for the linker)
	
_start:                # Beginning of the program 

	mov    $0xe, %ah	        # Select BIOS tty mode	
	mov    $0x0, %si	        # Iteration index

loop:			      

	mov    msg(%si), %al       # Offset to each character in the message
	cmp     $0x0, %al	        # Loop while char is not 0x0 (end of string)
	je      halt                # Jump to halt if last cmp was true
	int     $0x10	            # Call BIOS video service
	add     $0x1, %si	        # Point to the next character
	jmp     loop	            # Repeat until we find a character 0x0
	
halt:

	hlt		                # Halt
	jmp     halt	        # Safeguard

msg:

	.string     "Hello World"           # C-like NULL terminated string
	.fill   510 - (. - _start), 1, 0    # Pad with zeros
	.word   0xaa55			            # Boot signature
