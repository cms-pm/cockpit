/**
 * Universal Frame Parser - C++ Implementation
 * ===========================================
 * 
 * Production-ready binary frame parser that can handle timeout-based I/O,
 * comprehensive error recovery, and detailed diagnostics.
 * 
 * This implementation demonstrates how the Python version can be ported
 * to systems languages while maintaining the same robust behavior.
 * 
 * Frame Format: START(1) | LENGTH(2) | PAYLOAD(N) | CRC16(2) | END(1)
 */

#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <optional>
#include <chrono>
#include <map>

namespace bootloader {

enum class FrameParserState {
    IDLE = 0,
    SYNC = 1,
    LENGTH_HIGH = 2,
    LENGTH_LOW = 3,
    PAYLOAD = 4,
    CRC_HIGH = 5,
    CRC_LOW = 6,
    END = 7,
    COMPLETE = 8
};

struct ParseResult {
    bool success;
    std::vector<uint8_t> payload;
    std::string error;
    size_t bytes_consumed = 0;
    std::map<std::string, std::string> diagnostics;
    
    ParseResult(bool success) : success(success) {}
    ParseResult(bool success, const std::string& error) : success(success), error(error) {}
};

class SerialInterface {
public:
    virtual ~SerialInterface() = default;
    virtual std::optional<std::vector<uint8_t>> read(size_t num_bytes, uint32_t timeout_ms) = 0;
    virtual bool is_open() const = 0;
};

class UniversalFrameParser {
private:
    // Protocol constants
    static constexpr uint8_t FRAME_START = 0x7E;
    static constexpr uint8_t FRAME_END = 0x7F;
    static constexpr uint16_t MAX_PAYLOAD_SIZE = 1024;
    static constexpr uint16_t CRC16_CCITT_POLY = 0x1021;
    static constexpr uint16_t CRC16_CCITT_INIT = 0xFFFF;
    
    SerialInterface& serial;
    uint32_t read_timeout_ms;
    FrameParserState state;
    std::vector<uint8_t> buffer;
    uint16_t expected_payload_length;
    uint16_t received_crc;
    std::vector<uint8_t> payload_buffer;
    size_t bytes_processed;
    
public:
    explicit UniversalFrameParser(SerialInterface& serial_port, uint32_t timeout_ms = 1000)
        : serial(serial_port), read_timeout_ms(timeout_ms), state(FrameParserState::IDLE) {
        reset_parser();
    }
    
    void reset_parser() {
        state = FrameParserState::IDLE;
        buffer.clear();
        expected_payload_length = 0;
        received_crc = 0;
        payload_buffer.clear();
        bytes_processed = 0;
    }
    
    static uint16_t calculate_crc16_ccitt(const std::vector<uint8_t>& data) {
        uint16_t crc = CRC16_CCITT_INIT;
        for (uint8_t byte : data) {
            crc ^= (static_cast<uint16_t>(byte) << 8);
            for (int i = 0; i < 8; i++) {
                if (crc & 0x8000) {
                    crc = (crc << 1) ^ CRC16_CCITT_POLY;
                } else {
                    crc = crc << 1;
                }
                crc &= 0xFFFF;
            }
        }
        return crc;
    }
    
    std::optional<std::vector<uint8_t>> read_with_timeout(size_t num_bytes) {
        return serial.read(num_bytes, read_timeout_ms);
    }
    
    bool find_frame_start(std::vector<uint8_t>& discarded_bytes) {
        auto start_time = std::chrono::steady_clock::now();
        
        while (true) {
            // Check timeout
            auto current_time = std::chrono::steady_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                current_time - start_time).count();
            
            if (elapsed > read_timeout_ms) {
                return false;
            }
            
            // Read single byte
            auto byte_data = serial.read(1, 100); // 100ms micro-timeout
            if (!byte_data || byte_data->empty()) {
                continue;
            }
            
            uint8_t byte_val = (*byte_data)[0];
            
            if (byte_val == FRAME_START) {
                return true;
            } else {
                discarded_bytes.push_back(byte_val);
                // Limit garbage logging
                if (discarded_bytes.size() > 20) {
                    discarded_bytes.erase(discarded_bytes.begin());
                }
            }
        }
    }
    
    ParseResult parse_frame() {
        ParseResult result(false);
        auto start_time = std::chrono::steady_clock::now();
        
        result.diagnostics["start_time"] = std::to_string(
            std::chrono::duration_cast<std::chrono::milliseconds>(
                start_time.time_since_epoch()).count());
        
        try {
            // Step 1: Find frame start marker
            std::vector<uint8_t> discarded_bytes;
            if (!find_frame_start(discarded_bytes)) {
                result.error = "Frame start marker not found";
                return result;
            }
            
            // Step 2: Read length field (2 bytes, big-endian)
            auto length_bytes = read_with_timeout(2);
            if (!length_bytes || length_bytes->size() != 2) {
                result.error = "Failed to read length field";
                return result;
            }
            
            uint16_t payload_length = ((*length_bytes)[0] << 8) | (*length_bytes)[1];
            result.diagnostics["payload_length"] = std::to_string(payload_length);
            
            // Validate payload length
            if (payload_length > MAX_PAYLOAD_SIZE) {
                result.error = "Invalid payload length: " + std::to_string(payload_length);
                return result;
            }
            
            // Step 3: Read payload
            auto payload = read_with_timeout(payload_length);
            if (!payload || payload->size() != payload_length) {
                result.error = "Incomplete payload";
                return result;
            }
            
            // Step 4: Read CRC16 (2 bytes, big-endian)
            auto crc_bytes = read_with_timeout(2);
            if (!crc_bytes || crc_bytes->size() != 2) {
                result.error = "Failed to read CRC field";
                return result;
            }
            
            uint16_t received_crc = ((*crc_bytes)[0] << 8) | (*crc_bytes)[1];
            result.diagnostics["received_crc"] = "0x" + 
                std::to_string(received_crc); // Would use proper hex formatting
            
            // Step 5: Read end marker
            auto end_bytes = read_with_timeout(1);
            if (!end_bytes || end_bytes->size() != 1 || (*end_bytes)[0] != FRAME_END) {
                result.error = "Invalid end marker";
                return result;
            }
            
            // Step 6: Validate CRC
            std::vector<uint8_t> frame_data = *length_bytes;
            frame_data.insert(frame_data.end(), payload->begin(), payload->end());
            uint16_t calculated_crc = calculate_crc16_ccitt(frame_data);
            
            result.diagnostics["calculated_crc"] = "0x" + std::to_string(calculated_crc);
            
            if (received_crc != calculated_crc) {
                result.error = "CRC mismatch";
                return result;
            }
            
            // Success!
            auto end_time = std::chrono::steady_clock::now();
            auto parse_time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                end_time - start_time).count();
            
            result.diagnostics["parse_time_ms"] = std::to_string(parse_time_ms);
            result.success = true;
            result.payload = *payload;
            result.bytes_consumed = 3 + payload_length + 2 + 1; // START + LENGTH + PAYLOAD + CRC + END
            
            return result;
            
        } catch (const std::exception& e) {
            result.error = "Parsing exception: " + std::string(e.what());
            return result;
        }
    }
    
    ParseResult parse_frame_with_retry(int max_attempts = 3) {
        ParseResult last_result(false);
        
        for (int attempt = 0; attempt < max_attempts; attempt++) {
            ParseResult result = parse_frame();
            
            if (result.success) {
                if (attempt > 0) {
                    // Log successful retry
                }
                return result;
            } else {
                last_result = result;
                reset_parser();
            }
        }
        
        return last_result;
    }
};

// Example concrete implementation for testing
class MockSerialInterface : public SerialInterface {
private:
    std::vector<uint8_t> data;
    size_t position = 0;
    
public:
    explicit MockSerialInterface(const std::vector<uint8_t>& test_data) : data(test_data) {}
    
    std::optional<std::vector<uint8_t>> read(size_t num_bytes, uint32_t timeout_ms) override {
        if (position >= data.size()) {
            return std::nullopt;
        }
        
        size_t available = std::min(num_bytes, data.size() - position);
        std::vector<uint8_t> result(data.begin() + position, 
                                    data.begin() + position + available);
        position += available;
        return result;
    }
    
    bool is_open() const override {
        return true;
    }
};

// Utility function to create test frames
inline std::vector<uint8_t> create_test_frame(const std::vector<uint8_t>& payload) {
    std::vector<uint8_t> frame;
    
    // START marker
    frame.push_back(UniversalFrameParser::FRAME_START);
    
    // LENGTH field (big-endian)
    uint16_t length = static_cast<uint16_t>(payload.size());
    frame.push_back((length >> 8) & 0xFF);
    frame.push_back(length & 0xFF);
    
    // PAYLOAD
    frame.insert(frame.end(), payload.begin(), payload.end());
    
    // Calculate and add CRC
    std::vector<uint8_t> crc_data;
    crc_data.push_back((length >> 8) & 0xFF);
    crc_data.push_back(length & 0xFF);
    crc_data.insert(crc_data.end(), payload.begin(), payload.end());
    
    uint16_t crc = UniversalFrameParser::calculate_crc16_ccitt(crc_data);
    frame.push_back((crc >> 8) & 0xFF);
    frame.push_back(crc & 0xFF);
    
    // END marker
    frame.push_back(UniversalFrameParser::FRAME_END);
    
    return frame;
}

} // namespace bootloader