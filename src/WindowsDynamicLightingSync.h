#ifndef WINDOWSDYNAMICLIGHTINGSYNC_H
#define WINDOWSDYNAMICLIGHTINGSYNC_H

#include "../dependencies/OpenRGBSamplePlugin/OpenRGB/OpenRGBPluginInterface.h"
#include "ResourceManager.h"
#include "RGBController.h"

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QCheckBox>
#include <QTimer>

// Windows Dynamic Lighting API includes
#ifdef _WIN32
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

    OpenRGBPluginInfo   GetPluginInfo();
    unsigned int        GetPluginAPIVersion();
    void                Load(ResourceManagerInterface* resource_manager_ptr);
    QWidget*            GetWidget();
    QMenu*              GetTrayMenu();
    void                Unload();

private slots:
    void                onToggleDynamicLighting();
    void                updateDeviceList();
    void                syncLighting();

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
    QLabel*                     statusLabel;
    QPushButton*                toggleButton;
    QCheckBox*                  enableCheckBox;
    QLabel*                     deviceCountLabel;
    QTimer*                     syncTimer;
    
    ResourceManagerInterface*   RMPointer;
    bool                        isDynamicLightingEnabled;
    bool                        darkTheme;
    RGBColor                    testColor;
    
#ifdef _WIN32
    // Windows Dynamic Lighting members
    DeviceWatcher               lampArrayWatcher;
    IVectorView<LampArray>      lampArrays;
    std::vector<LampArray>      connectedLampArrays;
    bool                        windowsLightingInitialized;
#endif
};

#endif // WINDOWSDYNAMICLIGHTINGSYNC_H