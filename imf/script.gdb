file decode
set environment LD_LIBRARY_PATH ./
break *0x08049395
run secret.cry
set $eax = 0x1
c
