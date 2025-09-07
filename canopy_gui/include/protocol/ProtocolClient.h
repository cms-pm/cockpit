#pragma once

#include <QObject>
#include <QString>
#include <QByteArray>
#include <expected>
#include <vector>
#include <memory>

namespace canopy {

class UniversalFrameParser;

struct HandshakeResponse {
    QString version;
    QString capabilities;
    uint32_t flash_page_size;
    uint32_t target_flash_address;
};

struct UploadResult {
    bool success;
    QString message;
    size_t bytes_uploaded;
    std::chrono::milliseconds duration;
};

struct ProtocolError {
    enum Type {
        CONNECTION_FAILED,
        HANDSHAKE_FAILED,
        TRANSFER_FAILED,
        VERIFICATION_FAILED,
        DEVICE_ERROR
    };
    
    Type type;
    QString message;
    QString recovery_suggestion;
};

/**
 * Protocol client for CockpitVM bytecode upload operations.
 * 
 * Provides high-level interface for device communication using Oracle
 * protocol components with Qt integration and RAII resource management.
 */
class ProtocolClient : public QObject {
    Q_OBJECT

public:
    explicit ProtocolClient(QObject* parent = nullptr);
    ~ProtocolClient();

    /**
     * Perform device handshake to identify CockpitVM device.
     * 
     * @param device_path Serial device path
     * @return HandshakeResponse on success, ProtocolError on failure
     */
    std::expected<HandshakeResponse, ProtocolError> handshake(const QString& device_path);
    
    /**
     * Upload bytecode to connected device.
     * 
     * @param device_path Target device serial path
     * @param bytecode_data Raw bytecode to upload
     * @return UploadResult on completion
     */
    std::expected<UploadResult, ProtocolError> uploadBytecode(
        const QString& device_path,
        const std::vector<uint8_t>& bytecode_data
    );
    
    /**
     * Cancel ongoing upload operation.
     */
    void cancelUpload();

signals:
    /**
     * Emitted during upload for progress indication.
     * 
     * @param percentage Upload completion percentage (0-100)
     * @param stage Current operation stage description
     */
    void uploadProgress(int percentage, const QString& stage);
    
    /**
     * Emitted when upload completes (success or failure).
     * 
     * @param result Upload operation result
     */
    void uploadComplete(const UploadResult& result);
    
    /**
     * Emitted when protocol error occurs.
     * 
     * @param error Error details with recovery suggestions
     */
    void protocolError(const ProtocolError& error);

private:
    std::expected<void, ProtocolError> prepareFlash(size_t bytecode_size);
    std::expected<void, ProtocolError> sendDataPackets(const std::vector<uint8_t>& data);
    std::expected<void, ProtocolError> verifyFlash();
    
    std::unique_ptr<UniversalFrameParser> frame_parser_;
    std::atomic<bool> cancel_requested_{false};
    QString current_device_;
};

} // namespace canopy