/*
 * ARM Semihosting Support for Debug Output
 * Phase 1, Chunk 1.3: QEMU Integration Foundation
 */

#ifndef SEMIHOSTING_H
#define SEMIHOSTING_H

#include <stdint.h>
#include <stddef.h>

// Semihosting Debug Control
// Define DISABLE_SEMIHOSTING to eliminate all semihosting calls
// This is critical for USART testing to avoid timing interference
#ifndef DISABLE_SEMIHOSTING
#define SEMIHOSTING_ENABLED 1
#else
#define SEMIHOSTING_ENABLED 0
#endif

// ARM semihosting operation codes
#define SYS_OPEN    0x01
#define SYS_CLOSE   0x02
#define SYS_WRITEC  0x03
#define SYS_WRITE0  0x04
#define SYS_WRITE   0x05
#define SYS_READ    0x06
#define SYS_READC   0x07
#define SYS_ISERROR 0x08
#define SYS_ISTTY   0x09
#define SYS_SEEK    0x0A
#define SYS_FLEN    0x0C
#define SYS_TMPNAM  0x0D
#define SYS_REMOVE  0x0E
#define SYS_RENAME  0x0F
#define SYS_CLOCK   0x10
#define SYS_TIME    0x11
#define SYS_SYSTEM  0x12
#define SYS_ERRNO   0x13
#define SYS_EXIT    0x18

// Semihosting call wrapper
static inline int __attribute__((always_inline)) 
semihost_call(int op, void *arg) {
    int result;
    __asm__ volatile (
        "mov r0, %1\n"
        "mov r1, %2\n"
        "bkpt #0xAB\n"
        "mov %0, r0"
        : "=r" (result)
        : "r" (op), "r" (arg)
        : "r0", "r1", "memory"
    );
    return result;
}

// High-level functions
void semihost_write_char(char c);
void semihost_write_string(const char *str);
void semihost_write_hex(uint32_t value);
void semihost_write_dec(uint32_t value);
void semihost_exit(int code);

// Debug print functions with conditional compilation
#if SEMIHOSTING_ENABLED
void debug_print(const char *str);
void debug_print_hex(const char *prefix, uint32_t value);
void debug_print_dec(const char *prefix, uint32_t value);
#else
// No-op macros when semihosting is disabled
#define debug_print(str) ((void)0)
#define debug_print_hex(prefix, value) ((void)0)
#define debug_print_dec(prefix, value) ((void)0)
#endif

#endif // SEMIHOSTING_H