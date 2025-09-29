/*
 * STM32G4 Platform Layer - Display Driver Interface
 * Phase 4.8.2: Pure I2C OLED peripheral implementation
 * 
 * Hardware: 128x32 SSD1306 OLED via I2C1 (PC11=SCL, PA8=SDA)
 * Approach: Direct I2C control, no external library dependencies
 */

#ifndef DISPLAY_DRIVER_H
#define DISPLAY_DRIVER_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// =================================================================
// Display Specifications
// =================================================================
#define DISPLAY_WIDTH  128
#define DISPLAY_HEIGHT 32
#define DISPLAY_BUFFER_SIZE (DISPLAY_WIDTH * DISPLAY_HEIGHT / 8)  // 512 bytes

#define DISPLAY_CHAR_WIDTH  6   // 6 pixels per character (5 + 1 spacing)
#define DISPLAY_CHAR_HEIGHT 8   // 8 pixels per character
#define DISPLAY_COLS (DISPLAY_WIDTH / DISPLAY_CHAR_WIDTH)   // 21 columns
#define DISPLAY_ROWS (DISPLAY_HEIGHT / DISPLAY_CHAR_HEIGHT) // 4 rows

// =================================================================
// SSD1306 I2C Configuration
// =================================================================
#define SSD1306_I2C_ADDRESS 0x3C  // 7-bit address
#define SSD1306_I2C_TIMEOUT 1000  // ms

// SSD1306 Command prefixes
#define SSD1306_CMD_PREFIX  0x00  // Command mode
#define SSD1306_DATA_PREFIX 0x40  // Data mode

// =================================================================
// Display Driver API
// =================================================================

/**
 * Initialize I2C peripheral and OLED display
 * @return true on success
 */
bool display_driver_init(void);

/**
 * Clear display buffer (does not update hardware)
 * @return true on success
 */
bool display_driver_clear(void);

/**
 * Draw text at character grid position (does not update hardware)
 * @param col Column position (0-20)
 * @param row Row position (0-3)
 * @param text Null-terminated string
 * @return true on success
 */
bool display_driver_text(uint8_t col, uint8_t row, const char* text);

/**
 * Flush buffer to OLED hardware via I2C
 * @return true on success
 */
bool display_driver_update(void);

/**
 * Set individual pixel in buffer (does not update hardware)
 * @param x X coordinate (0-127)
 * @param y Y coordinate (0-31)
 * @param color 0=clear, 1=set
 * @return true on success
 */
bool display_driver_set_pixel(uint8_t x, uint8_t y, uint8_t color);

/**
 * Get current display buffer (for debugging/testing)
 * @return Pointer to display buffer (512 bytes)
 */
const uint8_t* display_driver_get_buffer(void);

#ifdef __cplusplus
}
#endif

#endif // DISPLAY_DRIVER_H