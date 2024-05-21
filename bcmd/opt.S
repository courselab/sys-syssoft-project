#    SPDX-FileCopyrightText: 2024 Matheus Pereira Dias <matheuspd07@gmail.com>
#    SPDX-FileCopyrightText: 2024 Monaco F. J. <monaco@usp.br>
#   
#    SPDX-License-Identifier: GPL-3.0-or-later
#
#  This file is a derivative work from SYSeg (https://gitlab.com/monaco/syseg)
#  and contains modifications carried out by the following author(s):
#  Matheus Pereira Dias <matheuspd07@gmail.com>

	.code16
	.global strcmp
	.global strlen
	.globl itoa

	## Compare two strings
	##
	## This is a handcrafted memory-optized version of the strcmp()
	## function's C counterpart in utils.c. You may prefer this one
	## if running out of space in the 512-byte program.
	##
	## PS: can you think of an even shorter version? If so, contribute 
	## your thoughts (see CONTRIBUTING.md)
	
strcmp:
	pusha	           /* Save all GP registers.                         */
	xor %ax, %ax       /* Zero %ax to contain the result.                */
	mov %cx, %si       /* Copy string1 to %si.                           */
	mov %dx, %di       /* Copy string2 to %si.                           */
strcmp_loop:
	cmpb $0, (%si)    /* Compare byte at address pointed by %si with 0.  */
	je strcmp_end     /* If it's 0, we've reached the end of the string. */
	cmpb $0, (%di)    /* Compare byte at address pointed by %di with 0.  */
	je strcmp_end     /* If it's 0, we've reached the end of the string. */
	movb (%si), %bl   /* Load byte from address pointed by %si into %bl. */
	movb (%di), %cl   /* Load byte from address pointed by %di into %cl. */
	cmpb %cl, %bl     /* Compare the bytes in %bl and %cl.               */
	je strcmp_eq      /* If they're equal, skip 'inc %ax'.               */
	inc %ax
strcmp_eq:
	inc %si           /* Move to the next byte in string1.               */
	inc %di           /* Move to the next byte in string2.               */
	jmp strcmp_loop   /* Repeat the loop.                                */
strcmp_end:
	mov %ax, strcmp_tmp  /* Save %ax before popa.                        */
	popa	             /* Restore all GP registers.                    */
	mov strcmp_tmp, %ax  /* Restore %ax after popa.                      */
	ret               /* Return.                                         */


	# Convert unsigned integer in %ax to ASCII string
	# %bx = buffer to store string (must be large enough)

itoa:
    push %bx              # Save %bx, %si, %di
    push %si
    push %di

	mov %cx, %ax          # Move the integer parameter from %cx to %ax
    mov %dx, %bx          # Move the buffer parameter from %dx to %bx

    mov %bx, %di          # %di points to the start of the buffer
    mov $0, %cx           # %cx is the counter for digits

itoa_loop:
    xor %dx, %dx          # Clear %dx
    mov $10, %si          # Divide by 10
    div %si               # %ax = %ax / 10, %dx = %ax % 10
    add $'0', %dl         # Convert the remainder to ASCII
    push %dx              # Push ASCII character onto stack
    inc %cx               # Increment digit count
    test %ax, %ax         # Check if %ax is zero
    jnz itoa_loop         # If not, repeat

# Now pop characters in reverse order to the buffer
itoa_store:
    pop %ax               # Pop a character
    mov %al, (%di)        # Store it in the buffer
    inc %di               # Move buffer pointer
    loop itoa_store       # Repeat for all characters
    movb $0, (%di)        # Null-terminate the string
    pop %di               # Restore registers
    pop %si
    pop %bx
    ret

	## String length

strlen:
	pusha			/* Save all GP registers.              */
    mov %cx, %bx		/* Copy argument into base register.   */
	mov $0x0, %si		/* Initialize buffer index.            */
strlen_loop:
    mov (%bx, %si), %al	/* Get each ASCII character.           */
    cmp $0x0, %al		/* End writing on 0.                   */
    je strlen_end
    inc %si			/* Point to the next character.        */
    jmp strlen_loop
strlen_end:
    mov %si, strlen_tmp  /* Save %ax before popa.                        */
	popa	             /* Restore all GP registers.                    */
	mov strlen_tmp, %ax  /* Restore %ax after popa.                      */
	ret

	.section .data

strcmp_tmp:	          	/* Temporary memory for %ax.                       */
	.byte 0x0

strlen_tmp:			/* Temporary memory for %ax.                       */
	.byte 0x0