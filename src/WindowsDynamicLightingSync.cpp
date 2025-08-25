#include "WindowsDynamicLightingSync.h"
#include <QHBoxLayout>
#include <QLabel>

ResourceManagerInterface* WindowsDynamicLightingSync::RMPointer = nullptr;

OpenRGBPluginInfo WindowsDynamicLightingSync::GetPluginInfo()
{
    printf("WindowsDynamicLightingSync: GetPluginInfo called\n");

    OpenRGBPluginInfo info;
    info.Name         = "Windows Dynamic Lighting Sync";
    info.Description  = "Synchronizes OpenRGB with Windows Dynamic Lighting";
    info.Version      = VERSION_STRING;
    info.Commit       = GIT_COMMIT_ID;
    info.URL          = "https://github.com/user/WindowsDynamicLightingSync";
    info.Icon.load(":icon.png");

    info.Location     = OPENRGB_PLUGIN_LOCATION_TOP;
    info.Label        = "Windows Dynamic Lighting";

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

    QWidget* widget = new QWidget(nullptr);
    QHBoxLayout* layout = new QHBoxLayout();

    widget->setLayout(layout);
    layout->addWidget(new QLabel("Windows Dynamic Lighting Sync Plugin"));

    return widget;
}

QMenu* WindowsDynamicLightingSync::GetTrayMenu()
{
    printf("[WindowsDynamicLightingSync] Creating tray menu.\n");

    return nullptr;
}

void WindowsDynamicLightingSync::Unload()
{
    printf("[WindowsDynamicLightingSync] Unloading plugin.\n");
}

WindowsDynamicLightingSync::WindowsDynamicLightingSync()
{
    printf("[WindowsDynamicLightingSync] Constructor called.\n");
}

WindowsDynamicLightingSync::~WindowsDynamicLightingSync()
{
    printf("[WindowsDynamicLightingSync] Destructor called.\n");
}