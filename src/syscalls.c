/*
 * System Call Stubs for Bare Metal Embedded Systems
 * Required for C++ standard library linking in embedded environments
 */

#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

// Heap management for dynamic memory allocation
// Disable dynamic allocation for embedded safety
void *_sbrk(int incr) {
    (void)incr;
    errno = ENOMEM;
    return (void *)-1;
}

// _exit - terminate program (hang in embedded)
void _exit(int status) {
    (void)status;
    while (1) {
        // Hang indefinitely
    }
}

// _kill - send signal (not supported)
int _kill(int pid, int sig) {
    (void)pid;
    (void)sig;
    errno = EINVAL;
    return -1;
}

// _getpid - get process ID (always 1 in embedded)
int _getpid(void) {
    return 1;
}

// _write - write to file descriptor (redirect to semihosting)
int _write(int file, char *ptr, int len) {
    (void)file;
    (void)ptr;
    (void)len;
    return len; // Assume success for now
}

// _close - close file descriptor (not supported)
int _close(int file) {
    (void)file;
    return -1;
}

// _fstat - get file status (not supported)
int _fstat(int file, struct stat *st) {
    (void)file;
    st->st_mode = S_IFCHR;
    return 0;
}

// _isatty - check if file descriptor is a terminal
int _isatty(int file) {
    (void)file;
    return 1; // Always a terminal in embedded
}

// _lseek - seek in file (not supported)
int _lseek(int file, int ptr, int dir) {
    (void)file;
    (void)ptr;
    (void)dir;
    return 0;
}

// _read - read from file descriptor (not supported)
int _read(int file, char *ptr, int len) {
    (void)file;
    (void)ptr;
    (void)len;
    return 0;
}
