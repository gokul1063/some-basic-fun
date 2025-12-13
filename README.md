PROJECT TITLE
--------------------------------------------------
Freestanding Linux User-Space Experiments (x86-64)


[ OVERVIEW ]
--------------------------------------------------
Low-level user-space programs without libc, standard
headers, or CRT, using direct Linux syscalls and the
System V AMD64 ABI.


[ HEADERS REPLACED ]
--------------------------------------------------
stdio.h | stdlib.h | unistd.h | time.h | crt0


[ COMPILE ]
--------------------------------------------------
gcc sleep.c -nostdlib -fno-stack-protector -o sleep

--------------------------------------------------

