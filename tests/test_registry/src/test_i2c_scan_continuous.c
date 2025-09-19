/**
 * @file test_i2c_scan_continuous.c
 * @brief OLED command transmission test for oscilloscope analysis
 * 
 * Phase 4.8.1: I2C Peripheral Validation - Oscilloscope Analysis
 * 
 * This test sends actual SSD1306 OLED commands and data to generate
 * realistic I2C traffic for oscilloscope analysis and signal validation.
 * 
 * Features:
 * - PC6 LED startup indication (3 blinks = test running)
 * - USART2 diagnostics framework integration
 * - Continuous OLED command transmission at 2000ms intervals
 * 
 * Hardware: I2C3 PA8=SCL, PC11=SDA @ 100kHz
 * Target: SSD1306 OLED at address 0x3C
 * Status LED: PC6 (WeAct Studio onboard LED)
 */

#include <stdint.h>
#include <stdbool.h>

#ifdef PLATFORM_STM32G4
#include "stm32g4xx_hal.h"
#endif

#include "host_interface/host_interface.h"

#ifdef GOLDEN_TRIANGLE_DIAG_ENABLED
#include "gt_diagnostics.h"

// GT DIAG output function with fallback
void test_print(const char* message)
{
    // Try GT DIAG first, then fallback to debug UART
    GT_DIAG_INFO(GT_MOD_I2C_TEST, "%s", message);
    
    // Also send via debug UART for comparison
    debug_uart_write_string("[FALLBACK] ");
    debug_uart_write_string(message);
    debug_uart_write_string("\r\n");
}

#else
// Fallback to simple UART output
void test_print(const char* message)
{
    // Flash LED to show function is called
    LED_Toggle();
    HAL_Delay(50);
    LED_Toggle();
    
    debug_uart_write_string(message);
    debug_uart_write_string("\r\n");
}
#endif

// =================================================================
// Hardware Configuration
// =================================================================

#define OLED_I2C_ADDRESS 0x3C
#define I2C_TIMEOUT 1000
#define SCAN_DELAY_MS 500

// LED Configuration (PC6 - WeAct Studio onboard LED)
#define STATUS_LED_PORT GPIOC
#define STATUS_LED_PIN GPIO_PIN_6

static I2C_HandleTypeDef hi2c3;

// Forward declarations
void LED_Toggle(void);
void I2C_Bus_Reset(void);

// =================================================================
// LED Status Functions
// =================================================================

void LED_Init(void) {
    __HAL_RCC_GPIOC_CLK_ENABLE();
    
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = STATUS_LED_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(STATUS_LED_PORT, &GPIO_InitStruct);
    
    // LED off initially
    HAL_GPIO_WritePin(STATUS_LED_PORT, STATUS_LED_PIN, GPIO_PIN_RESET);
}

void LED_On(void) {
    HAL_GPIO_WritePin(STATUS_LED_PORT, STATUS_LED_PIN, GPIO_PIN_SET);
}

void LED_Off(void) {
    HAL_GPIO_WritePin(STATUS_LED_PORT, STATUS_LED_PIN, GPIO_PIN_RESET);
}

void LED_Toggle(void) {
    HAL_GPIO_TogglePin(STATUS_LED_PORT, STATUS_LED_PIN);
}

void LED_StartupSequence(void) {
    test_print("💡 LED startup sequence: 3 blinks = test running");
    
    for (int i = 0; i < 3; i++) {
        LED_On();
        HAL_Delay(200);
        LED_Off();
        HAL_Delay(200);
    }
    
    test_print("✅ LED startup sequence complete");
}

// =================================================================
// I2C Peripheral Setup
// =================================================================

void I2C3_GPIO_Init(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    // Enable GPIO clocks
    __HAL_RCC_GPIOA_CLK_ENABLE();  // PA8 (I2C3_SCL)
    __HAL_RCC_GPIOC_CLK_ENABLE();  // PC11 (I2C3_SDA)
    
    // Configure PA8 as I2C3_SCL
    GPIO_InitStruct.Pin = GPIO_PIN_8;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Pull = GPIO_NOPULL;  // External 4.7kΩ pull-ups
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF2_I2C3;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    
    // Configure PC11 as I2C3_SDA
    GPIO_InitStruct.Pin = GPIO_PIN_11;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Pull = GPIO_NOPULL;  // External 4.7kΩ pull-ups
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF2_I2C3;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
}

void I2C3_Peripheral_Init(void) {
    __HAL_RCC_I2C3_CLK_ENABLE();
    
    hi2c3.Instance = I2C3;
    hi2c3.Init.Timing = 0x30A0A7FB;  // 100kHz @ 160MHz PCLK1 (corrected for actual clock)
    hi2c3.Init.OwnAddress1 = 0;
    hi2c3.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    hi2c3.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c3.Init.OwnAddress2 = 0;
    hi2c3.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
    hi2c3.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c3.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
    
    HAL_StatusTypeDef result = HAL_I2C_Init(&hi2c3);
    if (result != HAL_OK) {
        debug_uart_write_string("[ERROR] I2C3 Init failed: ");
        if (result == HAL_ERROR) debug_uart_write_string("HAL_ERROR\r\n");
        else if (result == HAL_BUSY) debug_uart_write_string("HAL_BUSY\r\n");
        else if (result == HAL_TIMEOUT) debug_uart_write_string("HAL_TIMEOUT\r\n");
        else debug_uart_write_string("UNKNOWN\r\n");
        return;
    }
    
    result = HAL_I2CEx_ConfigAnalogFilter(&hi2c3, I2C_ANALOGFILTER_ENABLE);
    if (result != HAL_OK) {
        debug_uart_write_string("[ERROR] I2C3 Analog filter config failed\r\n");
    }
    
    result = HAL_I2CEx_ConfigDigitalFilter(&hi2c3, 0);
    if (result != HAL_OK) {
        debug_uart_write_string("[ERROR] I2C3 Digital filter config failed\r\n");
    }
    
    debug_uart_write_string("[DEBUG] I2C3 peripheral initialized successfully\r\n");
    
    // Check GPIO pin states after I2C init
    GPIO_PinState pa8_state = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_8);
    GPIO_PinState pc11_state = HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_11);
    
    debug_uart_write_string("[GPIO_CHECK] PA8 (SCL) state: ");
    if (pa8_state == GPIO_PIN_SET) debug_uart_write_string("HIGH");
    else debug_uart_write_string("LOW");
    debug_uart_write_string(" | PC11 (SDA) state: ");
    if (pc11_state == GPIO_PIN_SET) debug_uart_write_string("HIGH");
    else debug_uart_write_string("LOW");
    debug_uart_write_string("\r\n");
    
    // Check GPIO alternate function registers
    debug_uart_write_string("[GPIO_CHECK] Checking GPIO register configuration...\r\n");
    debug_uart_write_string("[GPIO_CHECK] GPIOA->MODER PA8 bits: ");
    uint32_t pa8_mode = (GPIOA->MODER >> (8 * 2)) & 0x3;
    if (pa8_mode == 0) debug_uart_write_string("INPUT");
    else if (pa8_mode == 1) debug_uart_write_string("OUTPUT");
    else if (pa8_mode == 2) debug_uart_write_string("ALTERNATE");
    else debug_uart_write_string("ANALOG");
    debug_uart_write_string("\r\n");
    
    debug_uart_write_string("[GPIO_CHECK] GPIOC->MODER PC11 bits: ");
    uint32_t pc11_mode = (GPIOC->MODER >> (11 * 2)) & 0x3;
    if (pc11_mode == 0) debug_uart_write_string("INPUT");
    else if (pc11_mode == 1) debug_uart_write_string("OUTPUT");
    else if (pc11_mode == 2) debug_uart_write_string("ALTERNATE");
    else debug_uart_write_string("ANALOG");
    debug_uart_write_string("\r\n");
}

// =================================================================
// I2C Bus Reset Function
// =================================================================

void I2C_Bus_Reset(void) {
    debug_uart_write_string("[DEBUG] Performing I2C bus reset...\r\n");
    
    // Disable I2C peripheral
    __HAL_I2C_DISABLE(&hi2c3);
    
    // Configure pins as GPIO outputs to manually toggle
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    // PA8 (SCL) as output
    GPIO_InitStruct.Pin = GPIO_PIN_8;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    
    // PC11 (SDA) as output  
    GPIO_InitStruct.Pin = GPIO_PIN_11;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
    
    // Generate 9 clock pulses to clear any stuck transaction
    for (int i = 0; i < 9; i++) {
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_RESET);  // SCL low
        HAL_Delay(1);
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_SET);    // SCL high
        HAL_Delay(1);
    }
    
    // Generate STOP condition
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_11, GPIO_PIN_RESET);  // SDA low
    HAL_Delay(1);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_SET);     // SCL high
    HAL_Delay(1);
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_11, GPIO_PIN_SET);    // SDA high
    HAL_Delay(1);
    
    // Reconfigure pins back to I2C alternate function
    I2C3_GPIO_Init();
    
    // Re-enable I2C peripheral
    __HAL_I2C_ENABLE(&hi2c3);
    
    debug_uart_write_string("[DEBUG] I2C bus reset complete\r\n");
}

// =================================================================
// OLED Command Functions
// =================================================================

// SSD1306 Command Sequences
static const uint8_t oled_init_sequence[] = {
    0x00, 0xAE,  // Display OFF
    0x00, 0xD5, 0x00, 0x80,  // Set display clock divide ratio/oscillator frequency
    0x00, 0xA8, 0x00, 0x1F,  // Set multiplex ratio (32-1)
    0x00, 0xD3, 0x00, 0x00,  // Set display offset
    0x00, 0x40,  // Set start line address
    0x00, 0x8D, 0x00, 0x14,  // Enable charge pump
    0x00, 0x20, 0x00, 0x00,  // Set memory addressing mode (horizontal)
    0x00, 0xA1,  // Set segment re-map
    0x00, 0xC8,  // Set COM output scan direction
    0x00, 0xDA, 0x00, 0x02,  // Set COM pins hardware configuration
    0x00, 0x81, 0x00, 0x8F,  // Set contrast control
    0x00, 0xD9, 0x00, 0xF1,  // Set pre-charge period
    0x00, 0xDB, 0x00, 0x40,  // Set VCOMH deselect level
    0x00, 0xA4,  // Entire display ON (resume to RAM content)
    0x00, 0xA6,  // Set normal display
    0x00, 0xAF   // Display ON
};

static const uint8_t oled_test_pattern[] = {
    0x40, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00,  // Test pattern data
    0x40, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55   // Checkerboard pattern
};

void print_i2c_status(const char* operation, HAL_StatusTypeDef status) {
    debug_uart_write_string("[I2C_STATUS] ");
    debug_uart_write_string(operation);
    debug_uart_write_string(": ");
    
    switch(status) {
        case HAL_OK:
            debug_uart_write_string("HAL_OK");
            break;
        case HAL_ERROR:
            debug_uart_write_string("HAL_ERROR");
            break;
        case HAL_BUSY:
            debug_uart_write_string("HAL_BUSY");
            break;
        case HAL_TIMEOUT:
            debug_uart_write_string("HAL_TIMEOUT");
            break;
        default:
            debug_uart_write_string("UNKNOWN_STATUS");
            break;
    }
    
    // Check I2C peripheral state
    uint32_t i2c_state = hi2c3.State;
    debug_uart_write_string(" | I2C_State: ");
    if (i2c_state == HAL_I2C_STATE_RESET) debug_uart_write_string("RESET");
    else if (i2c_state == HAL_I2C_STATE_READY) debug_uart_write_string("READY");
    else if (i2c_state == HAL_I2C_STATE_BUSY) debug_uart_write_string("BUSY");
    else if (i2c_state == HAL_I2C_STATE_BUSY_TX) debug_uart_write_string("BUSY_TX");
    else if (i2c_state == HAL_I2C_STATE_BUSY_RX) debug_uart_write_string("BUSY_RX");
    else if (i2c_state == HAL_I2C_STATE_LISTEN) debug_uart_write_string("LISTEN");
    else if (i2c_state == HAL_I2C_STATE_BUSY_TX_LISTEN) debug_uart_write_string("BUSY_TX_LISTEN");
    else if (i2c_state == HAL_I2C_STATE_BUSY_RX_LISTEN) debug_uart_write_string("BUSY_RX_LISTEN");
    else if (i2c_state == HAL_I2C_STATE_ABORT) debug_uart_write_string("ABORT");
    else debug_uart_write_string("UNKNOWN");
    
    debug_uart_write_string("\r\n");
}

void send_oled_commands(void) {
    HAL_StatusTypeDef status;
    
    debug_uart_write_string("[DEBUG] Starting OLED initialization sequence\r\n");
    
    // Send initialization sequence
    for (int i = 0; i < sizeof(oled_init_sequence); i += 2) {
        status = HAL_I2C_Master_Transmit(&hi2c3, OLED_I2C_ADDRESS << 1, 
                                        (uint8_t*)&oled_init_sequence[i], 2, I2C_TIMEOUT);
        
        if (i == 0) { // First command - detailed status
            print_i2c_status("First OLED command", status);
        }
        
        if (status != HAL_OK) {
            debug_uart_write_string("[ERROR] OLED init command failed at index ");
            // Simple number output
            if (i < 10) {
                debug_uart_write_string("0");
                debug_uart_write_string((char[]){i + '0', '\0'});
            }
            debug_uart_write_string("\r\n");
            print_i2c_status("Failed command", status);
        }
        HAL_Delay(10);  // Longer delay for debugging
    }
    
    debug_uart_write_string("[DEBUG] OLED initialization sequence complete\r\n");
}

void send_oled_test_data(void) {
    HAL_StatusTypeDef status;
    
    // Set column address range (0 to 127)
    uint8_t col_cmd[] = {0x00, 0x21, 0x00, 0x00, 0x00, 0x7F};
    status = HAL_I2C_Master_Transmit(&hi2c3, OLED_I2C_ADDRESS << 1, col_cmd, 6, I2C_TIMEOUT);
    
    // Set page address range (0 to 3 for 128x32)
    uint8_t page_cmd[] = {0x00, 0x22, 0x00, 0x00, 0x00, 0x03};
    status = HAL_I2C_Master_Transmit(&hi2c3, OLED_I2C_ADDRESS << 1, page_cmd, 6, I2C_TIMEOUT);
    
    // Send test pattern data
    for (int i = 0; i < sizeof(oled_test_pattern); i += 9) {
        status = HAL_I2C_Master_Transmit(&hi2c3, OLED_I2C_ADDRESS << 1, 
                                        (uint8_t*)&oled_test_pattern[i], 9, I2C_TIMEOUT);
        if (status != HAL_OK) {
            debug_uart_write_string("[DEBUG] OLED data transmission failed\r\n");
        }
        HAL_Delay(1);
    }
}

// =================================================================
// Main Test Function
// =================================================================

void run_test_i2c_scan_continuous_main(void) {
    // Initialize hardware
    HAL_Init();
    
    // Initialize host interface (required for UART operations)
    host_interface_init();
    
    // Initialize status LED first for immediate feedback
    LED_Init();
    
#ifdef GOLDEN_TRIANGLE_DIAG_ENABLED
    // Initialize GT DIAG framework
    if (!gt_diag_init(NULL, 115200)) {
        // GT DIAG init failed - use fallback debug UART
        debug_uart_begin(115200);
        debug_uart_write_string("GT DIAG init failed - using fallback\r\n");
    } else {
        // GT DIAG init succeeded - should see banner
        debug_uart_write_string("GT DIAG init succeeded\r\n");
    }
#else
    // Initialize USART2 diagnostics (fallback)
    debug_uart_begin(115200);
    debug_uart_write_string("Using fallback debug UART\r\n");
#endif
    
    test_print("");
    test_print("🚀 CockpitVM I2C Continuous Scan Test");
    test_print("Phase 4.8.1: Oscilloscope Analysis");
    test_print("Hardware: STM32G474 WeAct Studio CoreBoard");
    test_print("Target: SSD1306 OLED @ I2C address 0x3C");
    test_print("I2C3: PA8=SCL, PC11=SDA @ 100kHz");
    test_print("Status LED: PC6 (onboard LED)");
    test_print("");
    
    // LED startup sequence for visual confirmation
    LED_StartupSequence();
    HAL_Delay(500);
    
    // Initialize I2C peripheral
    test_print("⚙️  Initializing I2C3 peripheral...");
#ifdef GOLDEN_TRIANGLE_DIAG_ENABLED
    GT_DIAG_FLOW('A', "I2C3 GPIO initialization");
#endif
    I2C3_GPIO_Init();
#ifdef GOLDEN_TRIANGLE_DIAG_ENABLED
    GT_DIAG_FLOW('B', "I2C3 peripheral initialization");
#endif
    I2C3_Peripheral_Init();
    
    // Perform bus reset to clear any stuck state
    I2C_Bus_Reset();
    
    test_print("✅ I2C3 peripheral ready");
#ifdef GOLDEN_TRIANGLE_DIAG_ENABLED
    GT_DIAG_FLOW('C', "I2C3 initialization complete");
#endif
    test_print("");
    
    test_print("📡 OSCILLOSCOPE SETUP INSTRUCTIONS:");
    test_print("   • Connect Ch1 to PA8 (SCL) - I2C Clock");  
    test_print("   • Connect Ch2 to PC11 (SDA) - I2C Data");
    test_print("   • Use 3.3V logic levels");
    test_print("   • Trigger on SCL falling edge");
    test_print("");
    
    test_print("🔍 Starting OLED command transmission...");
    test_print("⏱️  Command cycle interval: 2000ms");
    test_print("💡 LED will blink on each command cycle");
    test_print("");
    
    uint32_t cycle_count = 0;
    
    // Check device presence first
    test_print("🔍 Checking OLED device presence...");
    HAL_StatusTypeDef device_status = HAL_I2C_IsDeviceReady(&hi2c3, OLED_I2C_ADDRESS << 1, 3, I2C_TIMEOUT);
    print_i2c_status("Device Ready Check", device_status);
    
    if (device_status == HAL_OK) {
        test_print("✅ OLED device detected");
    } else {
        test_print("❌ OLED device not detected - proceeding anyway for scope analysis");
    }
    
    // Initial OLED setup
    test_print("📺 Sending OLED initialization sequence...");
    send_oled_commands();
    test_print("✅ OLED initialization complete");
    
    // Continuous command cycle loop
    while(1) {
        cycle_count++;
        
        // Send OLED commands and test data
        if (cycle_count == 1) {
            debug_uart_write_string("[DEBUG] Starting first OLED command cycle\r\n");
        }
        
        // Send test pattern data
        send_oled_test_data();
        
        // LED indication of activity
        LED_Toggle();
        
        // Status output every 5th cycle to avoid flooding
        if (cycle_count % 5 == 0) {
#ifdef GOLDEN_TRIANGLE_DIAG_ENABLED
            GT_DIAG_INFO(GT_MOD_I2C_TEST, "📺 OLED command cycle #%lu complete", cycle_count);
            GT_DIAG_FLOW('D', "OLED data transmission cycle");
#else
            test_print("📺 OLED command cycle complete");
#endif
        }
        
        // Wait between command cycles (longer for scope analysis)
        HAL_Delay(2000);
    }
}