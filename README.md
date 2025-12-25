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

[ OVERVIEW ]
--------------------------------------------------
Terminal-based Matrix rain animation using raw TTY control, ANSI escape sequences, and direct POSIX syscalls.

[ METHODS USED ]
--------------------------------------------------
termios raw mode | ANSI escape rendering | SIGWINCH handling | nanosleep timing | ioctl(TIOCGWINSZ)

[ COMPILE ]
--------------------------------------------------
gcc matrix-rain.c -o matrix-rain

--------------------------------------------------
