#include "WindowsDynamicLightingSync.h"
#include "WindowsDynamicLightingSyncWidget.h"
#include <QApplication>
#include <QDebug>
#include <QStandardPaths>
#include <QDir>

#ifdef _WIN32
#include <windows.h>
#include <winrt/base.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.UI.ViewManagement.h>
using namespace winrt;
using namespace Windows::Foundation;
using namespace Windows::UI::ViewManagement;
#endif

ResourceManagerInterface* WindowsDynamicLightingSync::RMPointer = nullptr;

WindowsDynamicLightingSync::WindowsDynamicLightingSync()
    : widget(nullptr)
    , worker(nullptr)
    , worker_thread(nullptr)
    , settings(nullptr)
{
    qDebug() << "[WindowsDynamicLightingSync] Constructor";
}

WindowsDynamicLightingSync::~WindowsDynamicLightingSync()
{
    qDebug() << "[WindowsDynamicLightingSync] Destructor";
    
    if (settings)
    {
        delete settings;
        settings = nullptr;
    }
}

OpenRGBPluginInfo WindowsDynamicLightingSync::GetPluginInfo()
{
    qDebug() << "[WindowsDynamicLightingSync] Loading plugin info";

    OpenRGBPluginInfo info;
    info.Name           = "Windows Dynamic Lighting Sync";
    info.Description    = "Sincroniza dispositivos RGB com a Iluminação Dinâmica do Windows";
    info.Version        = "1.0.0";
    info.Commit         = "initial";
    info.URL            = "https://github.com/user/WindowsDynamicLightingSync";
    
    // Load icon from resources (will be created later)
    info.Icon.load(":/WindowsDynamicLightingSync.png");

    info.Location       = OPENRGB_PLUGIN_LOCATION_TOP;
    info.Label          = "Dynamic Lighting";
    info.TabIconString  = "Dynamic Lighting";
    info.TabIcon.load(":/WindowsDynamicLightingSync.png");

    return info;
}

unsigned int WindowsDynamicLightingSync::GetPluginAPIVersion()
{
    qDebug() << "[WindowsDynamicLightingSync] Loading plugin API version";
    return OPENRGB_PLUGIN_API_VERSION;
}

void WindowsDynamicLightingSync::Load(ResourceManagerInterface* resource_manager_ptr)
{
    qDebug() << "[WindowsDynamicLightingSync] Loading plugin";

    RMPointer = resource_manager_ptr;
    
    InitializeSettings();
    
    // Create worker thread for monitoring Windows Dynamic Lighting
    worker = new DynamicLightingWorker();
    worker_thread = new QThread();
    worker->moveToThread(worker_thread);
    
    // Connect signals
    connect(worker, &DynamicLightingWorker::ColorChanged, this, &WindowsDynamicLightingSync::OnColorChanged);
    connect(worker_thread, &QThread::started, worker, &DynamicLightingWorker::StartMonitoring);
    connect(worker_thread, &QThread::finished, worker, &DynamicLightingWorker::deleteLater);
    
    worker_thread->start();
}

QWidget* WindowsDynamicLightingSync::GetWidget()
{
    qDebug() << "[WindowsDynamicLightingSync] Creating widget";

    if (!widget)
    {
        widget = new WindowsDynamicLightingSyncWidget(nullptr, this);
    }
    
    return widget;
}

QMenu* WindowsDynamicLightingSync::GetTrayMenu()
{
    qDebug() << "[WindowsDynamicLightingSync] Creating tray menu";

    QMenu* menu = new QMenu("Dynamic Lighting Sync");
    
    QAction* toggle_action = new QAction("Ativar/Desativar Sincronização", this);
    connect(toggle_action, &QAction::triggered, [this]() {
        if (widget)
        {
            // Toggle sync state through widget
            widget->ToggleSync();
        }
    });
    
    menu->addAction(toggle_action);
    
    return menu;
}

void WindowsDynamicLightingSync::Unload()
{
    qDebug() << "[WindowsDynamicLightingSync] Unloading plugin";
    
    StopDynamicLightingMonitoring();
    
    if (worker_thread)
    {
        worker_thread->quit();
        worker_thread->wait(3000); // Wait up to 3 seconds
        delete worker_thread;
        worker_thread = nullptr;
    }
    
    if (worker)
    {
        delete worker;
        worker = nullptr;
    }
}

void WindowsDynamicLightingSync::InitializeSettings()
{
    QString config_dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(config_dir);
    
    QString settings_file = config_dir + "/WindowsDynamicLightingSync.ini";
    settings = new QSettings(settings_file, QSettings::IniFormat);
    
    qDebug() << "[WindowsDynamicLightingSync] Settings file:" << settings_file;
}

void WindowsDynamicLightingSync::StartDynamicLightingMonitoring()
{
    if (worker && worker_thread && worker_thread->isRunning())
    {
        QMetaObject::invokeMethod(worker, "StartMonitoring", Qt::QueuedConnection);
    }
}

void WindowsDynamicLightingSync::StopDynamicLightingMonitoring()
{
    if (worker)
    {
        QMetaObject::invokeMethod(worker, "StopMonitoring", Qt::QueuedConnection);
    }
}

void WindowsDynamicLightingSync::OnColorChanged(unsigned int color)
{
    qDebug() << "[WindowsDynamicLightingSync] Color changed:" << QString::number(color, 16);
    
    if (widget)
    {
        widget->OnDynamicLightingColorChanged(color);
    }
}

void WindowsDynamicLightingSync::OnDeviceListChanged()
{
    qDebug() << "[WindowsDynamicLightingSync] Device list changed";
    
    if (widget)
    {
        widget->RefreshDeviceList();
    }
}

// DynamicLightingWorker implementation
DynamicLightingWorker::DynamicLightingWorker(QObject *parent)
    : QObject(parent)
    , monitoring_timer(nullptr)
    , is_monitoring(false)
    , last_color(0)
{
    monitoring_timer = new QTimer(this);
    connect(monitoring_timer, &QTimer::timeout, this, &DynamicLightingWorker::CheckDynamicLighting);
}

DynamicLightingWorker::~DynamicLightingWorker()
{
    StopMonitoring();
    CleanupWindowsAPI();
}

void DynamicLightingWorker::StartMonitoring()
{
    QMutexLocker locker(&mutex);
    
    if (is_monitoring)
        return;
        
    qDebug() << "[DynamicLightingWorker] Starting monitoring";
    
    if (InitializeWindowsAPI())
    {
        is_monitoring = true;
        monitoring_timer->start(100); // Check every 100ms
    }
    else
    {
        emit ErrorOccurred("Failed to initialize Windows Dynamic Lighting API");
    }
}

void DynamicLightingWorker::StopMonitoring()
{
    QMutexLocker locker(&mutex);
    
    if (!is_monitoring)
        return;
        
    qDebug() << "[DynamicLightingWorker] Stopping monitoring";
    
    is_monitoring = false;
    monitoring_timer->stop();
}

void DynamicLightingWorker::CheckDynamicLighting()
{
    if (!is_monitoring)
        return;
        
    unsigned int current_color = GetWindowsDynamicLightingColor();
    
    if (current_color != last_color)
    {
        last_color = current_color;
        emit ColorChanged(current_color);
    }
}

unsigned int DynamicLightingWorker::GetWindowsDynamicLightingColor()
{
#ifdef _WIN32
    try
    {
        // Try to get Windows accent color as a fallback
        // In a real implementation, this would use the actual Dynamic Lighting API
        UISettings ui_settings;
        auto accent_color = ui_settings.GetColorValue(UIColorType::Accent);
        
        unsigned int color = (accent_color.R << 16) | (accent_color.G << 8) | accent_color.B;
        return color;
    }
    catch (...)
    {
        // Fallback to a default color if API fails
        return 0x0080FF; // Blue
    }
#else
    // Non-Windows platforms return a default color
    return 0x0080FF; // Blue
#endif
}

bool DynamicLightingWorker::InitializeWindowsAPI()
{
#ifdef _WIN32
    try
    {
        // Initialize WinRT
        init_apartment();
        return true;
    }
    catch (...)
    {
        return false;
    }
#else
    return false;
#endif
}

void DynamicLightingWorker::CleanupWindowsAPI()
{
#ifdef _WIN32
    try
    {
        // Cleanup WinRT if needed
        uninit_apartment();
    }
    catch (...)
    {
        // Ignore cleanup errors
    }
#endif
}

#include "WindowsDynamicLightingSync.moc"