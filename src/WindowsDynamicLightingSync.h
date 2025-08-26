#ifndef WINDOWSDYNAMICLIGHTINGSYNC_H
#define WINDOWSDYNAMICLIGHTINGSYNC_H

#include "OpenRGBPluginInterface.h"
#include "ResourceManager.h"
#include "RGBController.h"
#include "LogManager.h"
#include "SettingsManager.h"
#include <QObject>
#include <QWidget>
#include <QMenu>
#include <QTimer>
#include <QThread>
#include <QMutex>

// Define GIT_COMMIT_ID if not defined by build system
#ifndef GIT_COMMIT_ID
#define GIT_COMMIT_ID "unknown"
#endif
#include <QLabel>
#include <QPushButton>
#include <QCheckBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSlider>
#include <QScrollArea>
#include <vector>
#include <memory>
#include <algorithm>
#include <chrono>
#include <stdexcept>
#include <exception>
#include <iostream>
#include <fstream>
#include <sstream>
#include <functional>
#include <chrono>
#include <iomanip>
#include <cstdio>

// RGB Color macros (from OpenRGB)
#ifndef RGBGetRValue
#define RGBGetRValue(rgb)      ((unsigned char)(rgb))
#endif
#ifndef RGBGetGValue
#define RGBGetGValue(rgb)      ((unsigned char)(((unsigned short)(rgb)) >> 8))
#endif
#ifndef RGBGetBValue
#define RGBGetBValue(rgb)      ((unsigned char)((rgb) >> 16))
#endif
#ifndef ToRGBColor
#define ToRGBColor(r,g,b)      ((RGBColor)(((unsigned char)(r)|((unsigned short)((unsigned char)(g))<<8))|(((unsigned long)(unsigned char)(b))<<16)))
#endif

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <winrt/base.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Devices.Enumeration.h>
#include <winrt/Windows.Devices.Lights.h>
#include <winrt/Windows.Devices.Lights.Effects.h>
using namespace winrt;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::Devices::Enumeration;
using namespace Windows::Devices::Lights;
using namespace Windows::Devices::Lights::Effects;
#endif

class WindowsDynamicLightingSync : public QObject, public OpenRGBPluginInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID OpenRGBPluginInterface_IID)
    Q_INTERFACES(OpenRGBPluginInterface)

public:
    WindowsDynamicLightingSync();
    ~WindowsDynamicLightingSync();

    // OpenRGBPluginInterface methods
    OpenRGBPluginInfo   GetPluginInfo() override;
    unsigned int        GetPluginAPIVersion() override;
    void                        Load(ResourceManagerInterface* resource_manager_ptr) override;
    QWidget*            GetWidget() override;
    QMenu*              GetTrayMenu() override;
    void                Unload() override;

private slots:
    void OnSyncTimer();
    void OnDeviceAdded();
    void OnDeviceRemoved();
    void onToggleDynamicLighting();
    void updateDeviceList();
    void syncLighting();
    void updateSystemInfo();
    void updateDeviceListUI();
    void onBrightnessChanged(int value);

private:
    void                setupUI();
    void                enableDynamicLighting(bool enable);
    void                applyLightingToAllDevices();
#ifdef _WIN32
    void                initializeWindowsLighting();
    void                cleanupWindowsLighting();
    void                onLampArrayAdded(DeviceWatcher const& sender, DeviceInformation const& deviceInfo);
    void                onLampArrayRemoved(DeviceWatcher const& sender, DeviceInformationUpdate const& deviceInfoUpdate);
    void                syncWithWindowsLighting();
#endif
    
    QWidget*                    widget;
    
    // Controle Dynamic Lighting Sync Plugin
    QCheckBox*                  enableCheckBox;
    QLabel*                     apiStatusLabel;
    QLabel*                     systemStatusLabel;
    QLabel*                     currentEffectLabel;
    QLabel*                     directionLabel;
    QLabel*                     primaryColorLabel;
    QLabel*                     secondaryColorLabel;
    
    // Dispositivos
    QLabel*                     deviceCountLabel;
    QWidget*                    deviceListWidget;
    QVBoxLayout*                deviceListLayout;
    
    // Configurações
    QCheckBox*                  brightnessControlCheckBox;
    QSlider*                    brightnessSlider;
    QLabel*                     brightnessValueLabel;
    
    // Informações do Sistema
    QLabel*                     osInfoLabel;
    QLabel*                     compatibilityLabel;
    QLabel*                     versionLabel;
    QLabel*                     urlLabel;
    
    QTimer*                     syncTimer;
    QTimer*                     sync_timer;
    QMutex                      sync_mutex;
    
    ResourceManagerInterface*   RMPointer;
    bool                        isDynamicLightingEnabled;
    bool                        darkTheme;
    RGBColor                    testColor;
    

    
    // Sync configuration
    bool                        sync_enabled;
    int                         sync_interval_ms;
    float                       brightness_multiplier;
    bool                        logging_enabled;
    bool                        is_loaded;
    bool                        dark_theme_enabled;
    // Funcionalidades removidas: bidirectional_sync, smooth_transitions, auto_detect_devices
    
    // Error handling and logging
    std::string                 log_file_path;
    bool                        show_error_dialogs;
    int                         max_log_file_size;
    std::ofstream               log_file_stream;
    
#ifdef _WIN32
    // Windows Dynamic Lighting members
    DeviceWatcher               lampArrayWatcher;
    IVectorView<LampArray>      lampArrays;
    std::vector<LampArray>      connectedLampArrays;
    bool                        windowsLightingInitialized;
    
    // Windows Dynamic Lighting integration
    std::vector<winrt::Windows::Devices::Lights::LampArray> lamp_arrays;
    std::vector<winrt::Windows::Devices::Lights::Effects::LampArrayBitmapEffect> bitmap_effects;
    
    // Windows Dynamic Lighting methods
    void InitializeDynamicLighting();
    void CleanupDynamicLighting();
    bool CheckDynamicLightingSupport();
    void SyncWithDynamicLighting();
    bool RegisterPluginAsDevice();
    void UnregisterPluginAsDevice();
    
    // Device management methods - Simplified
    // Removed: DetectCompatibleDevices(), RefreshDeviceList() - OpenRGB handles detection
    std::vector<RGBController*> GetCompatibleControllers();
    void UpdateDeviceColors();
    
    // Sync configuration methods
    void SetSyncInterval(int interval_ms);
    int GetSyncInterval() const;
    // Métodos removidos: DetectCompatibleDevices, RefreshDeviceList, SetAutoDetectDevices, IsAutoDetectEnabled, SetBidirectionalSync, IsBidirectionalSyncEnabled, SetSmoothTransitions, IsSmoothTransitionsEnabled
    
    // Settings management methods
    void LoadSettings();
    void SaveSettings();
    
    // Error handling and logging methods
    // Logging system removed - OpenRGB handles all logging
    void                        HandleException(const std::exception& e, const std::string& context);
    bool                        SafeExecute(std::function<void()> func, const std::string& operation);
    void                        ShowErrorDialog(const QString& title, const QString& message);
    void SetBrightnessMultiplier(float multiplier);
    float GetBrightnessMultiplier() const;
    
    // Advanced sync methods
    void SyncOpenRGBToWindows();
    void SyncWindowsToOpenRGB();
    RGBColor ConvertWindowsColorToRGB(const Windows::UI::Color& winColor);
    Windows::UI::Color ConvertRGBToWindowsColor(const RGBColor& rgbColor);
#endif
};

#endif // WINDOWSDYNAMICLIGHTINGSYNC_H