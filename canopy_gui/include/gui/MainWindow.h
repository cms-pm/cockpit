#pragma once

#include <QMainWindow>
#include <QTabWidget>
#include <QPushButton>
#include <QProgressBar>
#include <QTextEdit>
#include <QComboBox>
#include <QLabel>
#include <QThread>
#include <memory>

QT_BEGIN_NAMESPACE
class QFileDialog;
QT_END_NAMESPACE

namespace canopy {

class ProtocolClient;

/**
 * Main application window for Canopy bytecode uploader.
 * 
 * Provides simple interface for file selection, device choice, and upload progress.
 * Implements progressive disclosure with Simple/Advanced tabs.
 */
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    void onSelectFileClicked();
    void onRefreshDevicesClicked();
    void onUploadClicked();
    void onCancelUploadClicked();
    
    // Protocol worker signals
    void onUploadProgress(int percentage, const QString& stage);
    void onUploadComplete(bool success, const QString& message);
    void onDeviceDiscovered(const QString& device_path, const QString& version);

private:
    void setupUI();
    void setupSimpleTab();
    void setupAdvancedTab();
    void updateUIState();
    void startDeviceDiscovery();
    
    // UI Components - Simple Tab
    QPushButton* select_file_button_;
    QLabel* selected_file_label_;
    QComboBox* device_combo_;
    QPushButton* refresh_devices_button_;
    QPushButton* upload_button_;
    QPushButton* cancel_button_;
    QProgressBar* progress_bar_;
    QLabel* status_label_;
    
    // UI Components - Advanced Tab
    QTextEdit* bytecode_info_text_;
    QTextEdit* protocol_log_text_;
    
    // Core Components
    QTabWidget* tab_widget_;
    std::unique_ptr<ProtocolClient> protocol_client_;
    std::unique_ptr<QThread> worker_thread_;
    
    // State
    QString selected_file_path_;
    bool upload_in_progress_;
};

} // namespace canopy