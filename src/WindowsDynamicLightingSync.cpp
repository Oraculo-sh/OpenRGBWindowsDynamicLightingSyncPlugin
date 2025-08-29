#include "WindowsDynamicLightingSync.h"
#include <QHBoxLayout>

ResourceManagerInterface* WindowsDynamicLightingSync::RMPointer = nullptr;

OpenRGBPluginInfo WindowsDynamicLightingSync::GetPluginInfo()
{
    printf("[WindowsDynamicLightingSync] Loading plugin info.\n");

    OpenRGBPluginInfo info;
    info.Name         = "Windows Dynamic Lighting Sync";
    info.Description  = "Synchronizes RGB devices with Windows Dynamic Lighting";
    info.Version  = VERSION_STRING;
    info.Commit  = GIT_COMMIT_ID;
    info.URL  = "https://github.com/Oraculo-sh/OpenRGBWindowsDynamicLightingSyncPlugin";
    info.Icon.load(":/Assets/WindowsDynamicLightingSync128x128.png");

    info.Location     =  OPENRGB_PLUGIN_LOCATION_TOP;
    info.Label        =  "Sample plugin";
    info.TabIconString        =  "Sample plugin";
    info.TabIcon.load(":/Assets/WindowsDynamicLightingSync128x128.png");

    return info;
}

unsigned int WindowsDynamicLightingSync::GetPluginAPIVersion()
{
    printf("[WindowsDynamicLightingSync] Loading plugin API version.\n");

    return OPENRGB_PLUGIN_API_VERSION;
}

void WindowsDynamicLightingSync::Load(ResourceManagerInterface* resource_manager_ptr)
{
    printf("[WindowsDynamicLightingSync] Loading plugin.\n");

    RMPointer = resource_manager_ptr;
}


QWidget* WindowsDynamicLightingSync::GetWidget()
{
    printf("[WindowsDynamicLightingSync] Creating widget.\n");

    QWidget* widget =  new QWidget(nullptr);
    QHBoxLayout* layout = new QHBoxLayout();

    widget->setLayout(layout);
    layout->addWidget(new QLabel("Allo, allo?"));

    return widget;
}

QMenu* WindowsDynamicLightingSync::GetTrayMenu()
{
    printf("[WindowsDynamicLightingSync] Creating tray menu.\n");

    QMenu* menu = new QMenu("Sample plugin");

    return menu;
}

void WindowsDynamicLightingSync::Unload()
{
    printf("[WindowsDynamicLightingSync] Time to call some cleaning stuff.\n");
}

WindowsDynamicLightingSync::WindowsDynamicLightingSync()
{
    printf("[WindowsDynamicLightingSync] Constructor.\n");
}

WindowsDynamicLightingSync::~WindowsDynamicLightingSync()
{
     printf("[WindowsDynamicLightingSync] Time to free some memory.\n");
}

