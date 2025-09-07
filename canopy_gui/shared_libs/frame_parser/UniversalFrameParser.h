/**
 * Universal Frame Parser - Qt6 Integration
 * =======================================
 * 
 * Qt6-integrated version of the Oracle frame parser, adapted for RAII
 * resource management and Qt signal/slot communication patterns.
 * 
 * Based on: tests/oracle_bootloader/lib/frame_parser_universal.hpp
 * Adapted for: Canopy GUI with QSerialPort integration
 */

#pragma once

#include <QSerialPort>
#include <QObject>
#include <QTimer>
#include <cstdint>
#include <vector>
#include <optional>
#include <expected>
#include <chrono>
#include <memory>

namespace canopy {

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

struct ProtocolError {
    enum Type {
        TIMEOUT,
        CRC_MISMATCH, 
        INVALID_FRAME,
        SERIAL_ERROR,
        PAYLOAD_TOO_LARGE
    };
    
    Type type;
    QString message;
    
    ProtocolError(Type t, const QString& msg) : type(t), message(msg) {}
};

struct ParseResult {
    std::vector<uint8_t> payload;
    size_t bytes_consumed = 0;
    std::chrono::milliseconds parse_time{0};
    
    ParseResult() = default;
    ParseResult(std::vector<uint8_t> data) : payload(std::move(data)) {}
};

/**
 * Qt6-integrated universal frame parser with RAII resource management.
 * 
 * Provides exception-safe frame parsing with automatic resource cleanup
 * and Qt signal integration for progress reporting.
 */
class UniversalFrameParser : public QObject {
    Q_OBJECT
    
private:
    // Protocol constants (from Oracle implementation)
    static constexpr uint8_t FRAME_START = 0x7E;
    static constexpr uint8_t FRAME_END = 0x7F;
    static constexpr uint16_t MAX_PAYLOAD_SIZE = 1024;
    static constexpr uint16_t CRC16_CCITT_POLY = 0x1021;
    static constexpr uint16_t CRC16_CCITT_INIT = 0xFFFF;
    
    std::unique_ptr<QSerialPort> serial_port_;
    std::unique_ptr<QTimer> timeout_timer_;
    uint32_t read_timeout_ms_;
    FrameParserState state_;
    std::vector<uint8_t> buffer_;
    uint16_t expected_payload_length_;
    uint16_t received_crc_;
    std::vector<uint8_t> payload_buffer_;
    
public:
    /**
     * Construct frame parser with RAII serial port management.
     * 
     * @param device_path Serial device path (e.g., "/dev/ttyUSB0")
     * @param timeout_ms Read timeout in milliseconds
     * @param parent Qt parent object for memory management
     */
    explicit UniversalFrameParser(
        const QString& device_path,
        uint32_t timeout_ms = 1000,
        QObject* parent = nullptr
    );
    
    ~UniversalFrameParser() = default;
    
    // Move-only semantics for RAII
    UniversalFrameParser(const UniversalFrameParser&) = delete;
    UniversalFrameParser& operator=(const UniversalFrameParser&) = delete;
    UniversalFrameParser(UniversalFrameParser&&) = default;
    UniversalFrameParser& operator=(UniversalFrameParser&&) = default;
    
    /**
     * Parse a complete frame from the serial connection.
     * 
     * @return ParseResult on success, ProtocolError on failure
     */
    std::expected<ParseResult, ProtocolError> parseFrame();
    
    /**
     * Parse frame with automatic retry logic.
     * 
     * @param max_attempts Maximum retry attempts
     * @return ParseResult on success, ProtocolError on failure
     */
    std::expected<ParseResult, ProtocolError> parseFrameWithRetry(int max_attempts = 3);
    
    /**
     * Check if serial connection is open and ready.
     */
    bool isReady() const;
    
    /**
     * Reset parser state machine to IDLE.
     */
    void resetParser();
    
    /**
     * Calculate CRC16-CCITT for data validation.
     */
    static uint16_t calculateCrc16Ccitt(const std::vector<uint8_t>& data);

signals:
    /**
     * Emitted during frame parsing for progress indication.
     * 
     * @param state Current parser state
     * @param bytes_read Number of bytes processed
     */
    void parseProgress(FrameParserState state, size_t bytes_read);
    
    /**
     * Emitted when frame parsing encounters an error.
     * 
     * @param error Error details
     */
    void parseError(const ProtocolError& error);

private slots:
    void onReadyRead();
    void onTimeout();

private:
    std::optional<std::vector<uint8_t>> readWithTimeout(size_t num_bytes);
    bool findFrameStart(std::vector<uint8_t>& discarded_bytes);
    std::expected<ParseResult, ProtocolError> parseFrameInternal();
};

/**
 * Factory function for creating frame parsers with error handling.
 * 
 * @param device_path Serial device path
 * @param timeout_ms Read timeout
 * @return Frame parser on success, error on failure
 */
std::expected<std::unique_ptr<UniversalFrameParser>, ProtocolError> 
createFrameParser(const QString& device_path, uint32_t timeout_ms = 1000);

} // namespace canopy