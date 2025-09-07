#pragma once

#include <QObject>
#include <QString>
#include <QStringList>
#include <QSerialPortInfo>
#include <vector>
#include <expected>

namespace canopy {

struct DeviceInfo {
    QString port_name;           // e.g., "/dev/ttyUSB0"
    QString description;         // Human-readable description
    QString version_string;      // CockpitVM version from handshake
    bool is_cockpit_device;      // Confirmed CockpitVM device
};

struct DiscoveryError {
    QString message;
    QString recovery_suggestion;
};

/**
 * Hardware discovery service for CockpitVM devices.
 * 
 * Scans available serial ports and identifies CockpitVM devices
 * through handshake protocol. Provides async discovery with
 * progress reporting.
 */
class DeviceDiscovery : public QObject {
    Q_OBJECT

public:
    explicit DeviceDiscovery(QObject* parent = nullptr);
    ~DeviceDiscovery() = default;

    /**
     * Start asynchronous device discovery scan.
     * 
     * Emits deviceDiscovered signals as devices are found,
     * followed by discoveryComplete when scan finishes.
     */
    void startDiscovery();
    
    /**
     * Cancel ongoing discovery operation.
     */
    void cancelDiscovery();
    
    /**
     * Get list of available serial ports (without CockpitVM identification).
     * 
     * @return List of available port names
     */
    static QStringList getAvailableSerialPorts();
    
    /**
     * Test if specific device is CockpitVM device.
     * 
     * @param port_name Serial port to test
     * @return DeviceInfo on success, DiscoveryError on failure
     */
    std::expected<DeviceInfo, DiscoveryError> identifyDevice(const QString& port_name);

signals:
    /**
     * Emitted when discovery scan starts.
     * 
     * @param port_count Number of ports to scan
     */
    void discoveryStarted(int port_count);
    
    /**
     * Emitted for each port scanned during discovery.
     * 
     * @param port_name Current port being scanned
     * @param index Current port index (0-based)
     * @param total Total ports to scan
     */
    void discoveryProgress(const QString& port_name, int index, int total);
    
    /**
     * Emitted when CockpitVM device is discovered.
     * 
     * @param device_info Device information including version
     */
    void deviceDiscovered(const DeviceInfo& device_info);
    
    /**
     * Emitted when discovery scan completes.
     * 
     * @param devices_found Number of CockpitVM devices found
     * @param total_scanned Number of ports scanned
     */
    void discoveryComplete(int devices_found, int total_scanned);
    
    /**
     * Emitted when discovery encounters an error.
     * 
     * @param error Error details
     */
    void discoveryError(const DiscoveryError& error);

private:
    void performDiscovery();
    std::expected<QString, DiscoveryError> performHandshake(const QString& port_name);
    
    std::atomic<bool> cancel_requested_{false};
    QStringList ports_to_scan_;
};

} // namespace canopy