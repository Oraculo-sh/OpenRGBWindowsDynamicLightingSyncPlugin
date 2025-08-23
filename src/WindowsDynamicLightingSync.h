#ifndef WINDOWSDYNAMICLIGHTINGSYNC_H
#define WINDOWSDYNAMICLIGHTINGSYNC_H

#include "OpenRGBPluginInterface.h"
#include "ResourceManagerInterface.h"

// Define export macro for plugin functions
#ifdef _WIN32
    #define OPENRGBPLUGININTERFACE_EXPORT __declspec(dllexport)
#else
    #define OPENRGBPLUGININTERFACE_EXPORT
#endif

#include <QObject>
#include <QString>
#include <QtPlugin>
#include <QWidget>
#include <QTimer>
#include <QThread>
#include <QMutex>
#include <QSettings>

class WindowsDynamicLightingSyncWidget;
class DynamicLightingWorker;

class WindowsDynamicLightingSync : public QObject, public OpenRGBPluginInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID OpenRGBPluginInterface_IID)
    Q_INTERFACES(OpenRGBPluginInterface)

public:
    WindowsDynamicLightingSync();
    ~WindowsDynamicLightingSync();

    OpenRGBPluginInfo   GetPluginInfo() override;
    unsigned int        GetPluginAPIVersion() override;

    void                Load(ResourceManagerInterface* resource_manager_ptr) override;
    QWidget*            GetWidget() override;
    QMenu*              GetTrayMenu() override;
    void                Unload() override;

    static ResourceManagerInterface* RMPointer;

private slots:
    void                OnColorChanged(unsigned int color);
    void                OnDeviceListChanged();

private:
    WindowsDynamicLightingSyncWidget*   widget;
    DynamicLightingWorker*              worker;
    QThread*                            worker_thread;
    QSettings*                          settings;
    
    void                InitializeSettings();
    void                StartDynamicLightingMonitoring();
    void                StopDynamicLightingMonitoring();
};

// Worker class for monitoring Windows Dynamic Lighting
class DynamicLightingWorker : public QObject
{
    Q_OBJECT

public:
    explicit DynamicLightingWorker(QObject *parent = nullptr);
    ~DynamicLightingWorker();

public slots:
    void                StartMonitoring();
    void                StopMonitoring();

signals:
    void                ColorChanged(unsigned int color);
    void                ErrorOccurred(const QString& error);

private slots:
    void                CheckDynamicLighting();

private:
    QTimer*             monitoring_timer;
    QMutex              mutex;
    bool                is_monitoring;
    unsigned int        last_color;
    
    unsigned int        GetWindowsDynamicLightingColor();
    bool                InitializeWindowsAPI();
    void                CleanupWindowsAPI();
};

#endif // WINDOWSDYNAMICLIGHTINGSYNC_H