/*
 * Temporary stub for bootloader diagnostics to avoid compilation errors
 * This provides no-op implementations of all DIAG functions
 */

#ifndef BOOTLOADER_DIAGNOSTICS_STUB_H
#define BOOTLOADER_DIAGNOSTICS_STUB_H

#ifdef __cplusplus
extern "C" {
#endif

// Stub function declarations 
static inline bool bootloader_diag_init(bool x) { (void)x; return true; }
static inline bool nanopb_run_verification(void) { return true; }

// No-op macros for all DIAG calls
#define DIAG_INFO(comp, msg) do {} while(0)
#define DIAG_ERROR(comp, msg) do {} while(0)
#define DIAG_DEBUG(comp, msg) do {} while(0)
#define DIAG_WARN(comp, msg) do {} while(0)
#define DIAG_ERRORF(comp, fmt, ...) do {} while(0)
#define DIAG_DEBUGF(comp, fmt, ...) do {} while(0)
#define DIAG_FLOW(step, msg) do {} while(0)
#define DIAG_BUFFER(level, comp, name, data, len) do {} while(0)

// Stub component names
#define DIAG_COMPONENT_PROTOCOL_ENGINE      0
#define DIAG_COMPONENT_NANOPB_DECODE        1
#define DIAG_COMPONENT_NANOPB_ENCODE        2
#define DIAG_COMPONENT_MESSAGE_HANDLER      3
#define DIAG_COMPONENT_FRAME_PARSER         4

// Stub flow IDs
#define DIAG_FLOW_A_FRAME_START                 0
#define DIAG_FLOW_B_FRAME_LENGTH                1
#define DIAG_FLOW_C_FRAME_PAYLOAD               2
#define DIAG_FLOW_D_FRAME_CRC_OK                3
#define DIAG_FLOW_E_PROTOBUF_DECODE_START       4
#define DIAG_FLOW_F_PROTOBUF_DECODE_OK          5
#define DIAG_FLOW_G_MESSAGE_PROCESSING          6
#define DIAG_FLOW_H_RESPONSE_GENERATION         7
#define DIAG_FLOW_I_RESPONSE_ENCODE_OK          8
#define DIAG_FLOW_J_RESPONSE_TRANSMITTED        9

// Stub levels
#define DIAG_LEVEL_DEBUG    0

#ifdef __cplusplus
}
#endif

#endif // BOOTLOADER_DIAGNOSTICS_STUB_H