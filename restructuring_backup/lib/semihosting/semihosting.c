/*
 * ARM Semihosting Implementation
 * Phase 1, Chunk 1.3: QEMU Integration Foundation
 */

#include "semihosting.h"

// Write single character via semihosting
void semihost_write_char(char c) {
    semihost_call(SYS_WRITEC, &c);
}

// Write null-terminated string via semihosting
void semihost_write_string(const char *str) {
    semihost_call(SYS_WRITE0, (void*)str);
}

// Write 32-bit hex value
void semihost_write_hex(uint32_t value) {
    char hex_chars[] = "0123456789ABCDEF";
    char buffer[11] = "0x";  // "0x" + 8 hex digits + null
    
    for (int i = 7; i >= 0; i--) {
        buffer[2 + (7-i)] = hex_chars[(value >> (i * 4)) & 0xF];
    }
    buffer[10] = '\0';
    
    semihost_write_string(buffer);
}

// Write 32-bit decimal value
void semihost_write_dec(uint32_t value) {
    if (value == 0) {
        semihost_write_char('0');
        return;
    }
    
    char buffer[12];  // Max 10 digits + sign + null for 32-bit
    int pos = 11;
    buffer[pos] = '\0';
    
    while (value > 0 && pos > 0) {
        buffer[--pos] = '0' + (value % 10);
        value /= 10;
    }
    
    semihost_write_string(&buffer[pos]);
}

// Exit with return code
void semihost_exit(int code) {
    uint32_t exit_code = (uint32_t)code;
    semihost_call(SYS_EXIT, &exit_code);
}

// High-level debug functions - conditionally compiled
#if SEMIHOSTING_ENABLED
void debug_print(const char *str) {
    semihost_write_string(str);
    semihost_write_char('\n');
}

void debug_print_hex(const char *prefix, uint32_t value) {
    semihost_write_string(prefix);
    semihost_write_string(": ");
    semihost_write_hex(value);
    semihost_write_char('\n');
}

void debug_print_dec(const char *prefix, uint32_t value) {
    semihost_write_string(prefix);
    semihost_write_string(": ");
    semihost_write_dec(value);
    semihost_write_char('\n');
}
#endif  // SEMIHOSTING_ENABLED