#ifndef WINDOWSDYNAMICLIGHTINGSYNC_H
#define WINDOWSDYNAMICLIGHTINGSYNC_H

#include "OpenRGBPluginInterface.h"
#include "ResourceManagerInterface.h"
#include "RGBController.h"
#include "SettingsManager.h"

#include <QObject>

using json = nlohmann::json;
#include <QString>
#include <QtPlugin>
#include <QWidget>
#include <QTimer>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QCheckBox>
#include <QComboBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QGroupBox>
#include <QSlider>
#include <QScrollArea>
#include <QMutex>
#include <QMenu>
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
#include <iomanip>
#include <cstdio>
#include <cmath>
#include <limits>

// Define GIT_COMMIT_ID if not defined by build system
#ifndef GIT_COMMIT_ID
#define GIT_COMMIT_ID "unknown"
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
#include <winrt/Windows.UI.h>
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
    void                Load(ResourceManagerInterface* resource_manager_ptr) override;
    QWidget*            GetWidget() override;
    QMenu*              GetTrayMenu() override;
    void                Unload() override;

    static ResourceManagerInterface* RMPointer;

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
    QWidget*            createDeviceWidget(RGBController* controller, int index);
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
    QSpinBox*                   syncIntervalSpinBox;
    QCheckBox*                  enableBrightnessCheckBox;
    QWidget*                    brightnessContainer;
    QCheckBox*                  ambientModeCheckBox;
    QWidget*                    ambientControlsContainer;
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
    
    // ResourceManagerInterface is now static
    bool                        isDynamicLightingEnabled;
    bool                        darkTheme;
    
    // Sync configuration
    bool                        sync_enabled;
    int                         sync_interval_ms;
    float                       brightness_multiplier;
    bool                        ambientModeEnabled;
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
    
    // Métodos de iluminação ambiente
    void ApplyAmbientLighting();
    void EnableAmbientMode(bool enable);
    
    // Color conversion methods
    RGBColor ConvertWindowsColorToRGB(const Windows::UI::Color& windowsColor);
    Windows::UI::Color ConvertRGBToWindowsColor(const RGBColor& rgbColor);
    
    // Virtual device management
    void CreateVirtualLampArrayDevice();
    void DestroyVirtualLampArrayDevice();
    bool IsVirtualDeviceCreated() const;
    
    // Virtual device members
    bool virtualDeviceCreated;
    std::wstring virtualDeviceId;
#endif
};

#endif // WINDOWSDYNAMICLIGHTINGSYNC_H