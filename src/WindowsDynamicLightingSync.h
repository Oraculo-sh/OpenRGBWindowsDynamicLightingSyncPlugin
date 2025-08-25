#ifndef WINDOWSDYNAMICLIGHTINGSYNC_H
#define WINDOWSDYNAMICLIGHTINGSYNC_H

#include "OpenRGBPluginInterface.h"
#include "ResourceManagerInterface.h"

#include <QObject>
#include <QString>
#include <QtPlugin>
#include <QWidget>

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
};

#endif // WINDOWSDYNAMICLIGHTINGSYNC_H