#include <QApplication>
#include <QStyleFactory>
#include <QDir>
#include <QStandardPaths>
#include <QLoggingCategory>

#include "gui/MainWindow.h"

using namespace canopy;

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // Application metadata
    app.setApplicationName("Canopy");
    app.setApplicationVersion("1.0.0");
    app.setApplicationDisplayName("Canopy - CockpitVM Bytecode Uploader");
    app.setOrganizationName("CockpitVM");
    app.setOrganizationDomain("cockpitvm.local");
    
    // Configure logging
    QLoggingCategory::setFilterRules("canopy.*.debug=true");
    
    // Set application style
    app.setStyle(QStyleFactory::create("Fusion"));
    
    // Apply dark theme palette (optional)
    QPalette darkPalette;
    darkPalette.setColor(QPalette::Window, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::WindowText, Qt::white);
    darkPalette.setColor(QPalette::Base, QColor(25, 25, 25));
    darkPalette.setColor(QPalette::AlternateBase, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
    darkPalette.setColor(QPalette::ToolTipText, Qt::white);
    darkPalette.setColor(QPalette::Text, Qt::white);
    darkPalette.setColor(QPalette::Button, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::ButtonText, Qt::white);
    darkPalette.setColor(QPalette::BrightText, Qt::red);
    darkPalette.setColor(QPalette::Link, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::Highlight, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::HighlightedText, Qt::black);
    app.setPalette(darkPalette);
    
    // Create and show main window
    MainWindow window;
    window.show();
    
    return app.exec();
}