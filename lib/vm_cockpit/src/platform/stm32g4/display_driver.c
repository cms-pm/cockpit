/*
 * STM32G4 Platform Layer - Display Driver Implementation
 * Phase 4.8.2: Pure I2C OLED peripheral implementation
 * 
 * Features:
 * - Direct SSD1306 I2C communication via STM32 HAL
 * - 128x32 pixel buffer management
 * - Character grid text rendering (21x4)
 * - Deferred update pattern (accumulate changes, flush on command)
 */

#include "display_driver.h"
#include "stm32g4xx_hal.h"
#include <string.h>

// =================================================================
// Internal State
// =================================================================
static I2C_HandleTypeDef hi2c1;
static uint8_t display_buffer[DISPLAY_BUFFER_SIZE];
static bool initialized = false;

// Simple 5x8 font (A-Z, 0-9, space, basic symbols)
static const uint8_t font_5x8[][5] = {
    {0x7E, 0x09, 0x09, 0x09, 0x7E}, // A
    {0x7F, 0x49, 0x49, 0x49, 0x36}, // B
    {0x3E, 0x41, 0x41, 0x41, 0x22}, // C
    {0x7F, 0x41, 0x41, 0x22, 0x1C}, // D
    {0x7F, 0x49, 0x49, 0x49, 0x41}, // E
    {0x7F, 0x09, 0x09, 0x09, 0x01}, // F
    {0x3E, 0x41, 0x49, 0x49, 0x7A}, // G
    {0x7F, 0x08, 0x08, 0x08, 0x7F}, // H
    {0x00, 0x41, 0x7F, 0x41, 0x00}, // I
    {0x20, 0x40, 0x41, 0x3F, 0x01}, // J
    {0x7F, 0x08, 0x14, 0x22, 0x41}, // K
    {0x7F, 0x40, 0x40, 0x40, 0x40}, // L
    {0x7F, 0x02, 0x0C, 0x02, 0x7F}, // M
    {0x7F, 0x04, 0x08, 0x10, 0x7F}, // N
    {0x3E, 0x41, 0x41, 0x41, 0x3E}, // O
    {0x7F, 0x09, 0x09, 0x09, 0x06}, // P
    {0x3E, 0x41, 0x51, 0x21, 0x5E}, // Q
    {0x7F, 0x09, 0x19, 0x29, 0x46}, // R
    {0x46, 0x49, 0x49, 0x49, 0x31}, // S
    {0x01, 0x01, 0x7F, 0x01, 0x01}, // T
    {0x3F, 0x40, 0x40, 0x40, 0x3F}, // U
    {0x1F, 0x20, 0x40, 0x20, 0x1F}, // V
    {0x3F, 0x40, 0x38, 0x40, 0x3F}, // W
    {0x63, 0x14, 0x08, 0x14, 0x63}, // X
    {0x07, 0x08, 0x70, 0x08, 0x07}, // Y
    {0x61, 0x51, 0x49, 0x45, 0x43}, // Z
    {0x3E, 0x51, 0x49, 0x45, 0x3E}, // 0
    {0x00, 0x42, 0x7F, 0x40, 0x00}, // 1
    {0x42, 0x61, 0x51, 0x49, 0x46}, // 2
    {0x21, 0x41, 0x45, 0x4B, 0x31}, // 3
    {0x18, 0x14, 0x12, 0x7F, 0x10}, // 4
    {0x27, 0x45, 0x45, 0x45, 0x39}, // 5
    {0x3C, 0x4A, 0x49, 0x49, 0x30}, // 6
    {0x01, 0x71, 0x09, 0x05, 0x03}, // 7
    {0x36, 0x49, 0x49, 0x49, 0x36}, // 8
    {0x06, 0x49, 0x49, 0x29, 0x1E}, // 9
    {0x00, 0x00, 0x00, 0x00, 0x00}, // space
    {0x14, 0x14, 0x14, 0x14, 0x14}, // -
    {0x08, 0x08, 0x2A, 0x1C, 0x08}, // *
    {0x22, 0x14, 0x08, 0x14, 0x22}, // %
    {0x00, 0x60, 0x60, 0x00, 0x00}, // .
    {0x02, 0x04, 0x08, 0x10, 0x20}, // /
    {0x36, 0x36, 0x00, 0x00, 0x00}, // :
};

// Character to font index mapping
static int char_to_font_index(char c) {
    if (c >= 'A' && c <= 'Z') return c - 'A';
    if (c >= 'a' && c <= 'z') return c - 'a'; // Convert lowercase to uppercase
    if (c >= '0' && c <= '9') return c - '0' + 26;
    if (c == ' ') return 36;
    if (c == '-') return 37;
    if (c == '*') return 38;
    if (c == '%') return 39;
    if (c == '.') return 40;
    if (c == '/') return 41;
    if (c == ':') return 42;
    return 36; // Default to space
}

// =================================================================
// I2C Low-Level Functions
// =================================================================

static void i2c1_init(void) {
    hi2c1.Instance = I2C1;
    hi2c1.Init.Timing = 0x10909CEC; // 100kHz @ 170MHz PCLK1
    hi2c1.Init.OwnAddress1 = 0;
    hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
    
    HAL_I2C_Init(&hi2c1);
}

static bool send_command(uint8_t cmd) {
    uint8_t data[2] = {SSD1306_CMD_PREFIX, cmd};
    HAL_StatusTypeDef status = HAL_I2C_Master_Transmit(&hi2c1, SSD1306_I2C_ADDRESS << 1, 
                                                      data, 2, SSD1306_I2C_TIMEOUT);
    return (status == HAL_OK);
}

static bool send_data_buffer(const uint8_t* data, uint16_t length) {
    // Create buffer with data prefix
    uint8_t* tx_buffer = malloc(length + 1);
    if (!tx_buffer) return false;
    
    tx_buffer[0] = SSD1306_DATA_PREFIX;
    memcpy(&tx_buffer[1], data, length);
    
    HAL_StatusTypeDef status = HAL_I2C_Master_Transmit(&hi2c1, SSD1306_I2C_ADDRESS << 1,
                                                      tx_buffer, length + 1, SSD1306_I2C_TIMEOUT);
    free(tx_buffer);
    return (status == HAL_OK);
}

// =================================================================
// SSD1306 Initialization Sequence
// =================================================================

static bool ssd1306_init(void) {
    // SSD1306 initialization for 128x32 display
    if (!send_command(0xAE)) return false; // Display OFF
    if (!send_command(0xD5)) return false; // Set display clock
    if (!send_command(0x80)) return false; // Clock divide ratio
    if (!send_command(0xA8)) return false; // Set multiplex ratio
    if (!send_command(0x1F)) return false; // 32-1
    if (!send_command(0xD3)) return false; // Set display offset
    if (!send_command(0x00)) return false; // No offset
    if (!send_command(0x40)) return false; // Set display start line
    if (!send_command(0x8D)) return false; // Charge pump enable
    if (!send_command(0x14)) return false; // Enable charge pump
    if (!send_command(0x20)) return false; // Memory addressing mode
    if (!send_command(0x00)) return false; // Horizontal addressing
    if (!send_command(0xA1)) return false; // Set segment remap
    if (!send_command(0xC8)) return false; // Set COM output scan direction
    if (!send_command(0xDA)) return false; // Set COM pins hardware config
    if (!send_command(0x02)) return false; // Sequential COM pin, disable remap
    if (!send_command(0x81)) return false; // Set contrast
    if (!send_command(0x8F)) return false; // Contrast value
    if (!send_command(0xD9)) return false; // Set pre-charge period
    if (!send_command(0xF1)) return false; // Pre-charge value
    if (!send_command(0xDB)) return false; // Set VCOMH deselect level
    if (!send_command(0x40)) return false; // VCOMH value
    if (!send_command(0xA4)) return false; // Display resume from RAM
    if (!send_command(0xA6)) return false; // Normal display (not inverted)
    if (!send_command(0xAF)) return false; // Display ON
    
    return true;
}

// =================================================================
// Buffer Management Functions
// =================================================================

static void set_pixel_in_buffer(uint8_t x, uint8_t y, uint8_t color) {
    if (x >= DISPLAY_WIDTH || y >= DISPLAY_HEIGHT) return;
    
    uint16_t byte_index = x + (y / 8) * DISPLAY_WIDTH;
    uint8_t bit_mask = 1 << (y % 8);
    
    if (color) {
        display_buffer[byte_index] |= bit_mask;  // Set pixel
    } else {
        display_buffer[byte_index] &= ~bit_mask; // Clear pixel
    }
}

static void draw_char_at_pixel(uint8_t x, uint8_t y, char c) {
    int font_idx = char_to_font_index(c);
    
    for (int col = 0; col < 5; col++) {
        uint8_t column_data = font_5x8[font_idx][col];
        for (int row = 0; row < 8; row++) {
            uint8_t pixel = (column_data >> row) & 1;
            set_pixel_in_buffer(x + col, y + row, pixel);
        }
    }
}

// =================================================================
// Public API Implementation
// =================================================================

bool display_driver_init(void) {
    if (initialized) return true;
    
    // Initialize I2C peripheral
    i2c1_init();
    
    // Initialize SSD1306 OLED
    if (!ssd1306_init()) {
        return false;
    }
    
    // Clear display buffer
    memset(display_buffer, 0, DISPLAY_BUFFER_SIZE);
    
    initialized = true;
    return true;
}

bool display_driver_clear(void) {
    if (!initialized) return false;
    
    memset(display_buffer, 0, DISPLAY_BUFFER_SIZE);
    return true;
}

bool display_driver_text(uint8_t col, uint8_t row, const char* text) {
    if (!initialized) return false;
    if (col >= DISPLAY_COLS || row >= DISPLAY_ROWS) return false;
    if (!text) return false;
    
    uint8_t x = col * DISPLAY_CHAR_WIDTH;
    uint8_t y = row * DISPLAY_CHAR_HEIGHT;
    
    while (*text && x < DISPLAY_WIDTH) {
        draw_char_at_pixel(x, y, *text);
        text++;
        x += DISPLAY_CHAR_WIDTH;
    }
    
    return true;
}

bool display_driver_update(void) {
    if (!initialized) return false;
    
    // Set column address range (0-127)
    if (!send_command(0x21)) return false; // Column address
    if (!send_command(0x00)) return false; // Start column
    if (!send_command(0x7F)) return false; // End column
    
    // Set page address range (0-3 for 32 pixel height)
    if (!send_command(0x22)) return false; // Page address
    if (!send_command(0x00)) return false; // Start page
    if (!send_command(0x03)) return false; // End page
    
    // Send display buffer
    return send_data_buffer(display_buffer, DISPLAY_BUFFER_SIZE);
}

bool display_driver_set_pixel(uint8_t x, uint8_t y, uint8_t color) {
    if (!initialized) return false;
    
    set_pixel_in_buffer(x, y, color);
    return true;
}

const uint8_t* display_driver_get_buffer(void) {
    return display_buffer;
}