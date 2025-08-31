#ifndef WINDOWSDYNAMICLIGHTINGSYNC_H
#define WINDOWSDYNAMICLIGHTINGSYNC_H

#include "OpenRGBPluginInterface.h"
#include "ResourceManagerInterface.h"

#include <QObject>
#include <QString>
#include <QtPlugin>
#include <QWidget>
#include <QGroupBox>
#include <QCheckBox>
#include <QLabel>
#include <QSpinBox>
#include <QSlider>
#include <QComboBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>

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

private:
    // Main Widget
    QWidget* mainWidget;

    // Controle Dynamic Lighting Sync Plugin
    QCheckBox* enableSyncCheckbox;
    QLabel* apiStatusLabel;
    QLabel* systemStatusLabel;
    QLabel* currentEffectLabel;
    QLabel* directionEffectLabel;
    QLabel* primaryColorLabel;
    QLabel* secondaryColorLabel;

    // Dispositivos
    QLabel* deviceCountLabel;
    QVBoxLayout* deviceListLayout;

    // Configurações
    QSpinBox* syncIntervalSpinbox;
    QCheckBox* enableBrightnessCheckbox;
    QWidget* brightnessContainer;
    QSlider* brightnessSlider;
    QLabel* brightnessValueLabel;

    // Informações do Sistema
    QLabel* osInfoLabel;
    QLabel* compatibilityLabel;
    QLabel* versionLabel;
    QLabel* urlLabel;
    QPushButton* updateButton;
    QPushButton* reloadButton;

private slots:
    void onEnableSyncCheckboxToggled(bool checked);
    void onSyncIntervalSpinboxValueChanged(int value);
    void onEnableBrightnessCheckboxToggled(bool checked);
    void onBrightnessSliderValueChanged(int value);
    void onUpdateButtonClicked();
    void onReloadButtonClicked();
};

#endif // WINDOWSDYNAMICLIGHTINGSYNC_H
