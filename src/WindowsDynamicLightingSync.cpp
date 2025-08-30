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
    info.Label        =  "Dynamic Lighting Sync";
    info.TabIconString        =  "Dynamic Lighting Sync";
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
    if (mainWidget)
    {
        return mainWidget;
    }

    mainWidget = new QWidget(nullptr);
    QVBoxLayout* mainLayout = new QVBoxLayout(mainWidget);

    // 1. Controle Dynamic Lighting Sync Plugin
    QGroupBox* controlGroupBox = new QGroupBox("Controle Dynamic Lighting Sync Plugin");
    QVBoxLayout* controlLayout = new QVBoxLayout(controlGroupBox);

    enableSyncCheckbox = new QCheckBox("Habilitar Dynamic Lighting Sync");
    controlLayout->addWidget(enableSyncCheckbox);

    apiStatusLabel = new QLabel("API Windows Dynamic Lighting: Conectando...");
    controlLayout->addWidget(apiStatusLabel);

    systemStatusLabel = new QLabel("Status Windows Dynamic Lighting: Verificando...");
    controlLayout->addWidget(systemStatusLabel);

    currentEffectLabel = new QLabel("Efeito atual: Verificando...");
    controlLayout->addWidget(currentEffectLabel);

    directionEffectLabel = new QLabel("Direção: Verificando...");
    controlLayout->addWidget(directionEffectLabel);

    primaryColorLabel = new QLabel("Cor Principal: Verificando...");
    controlLayout->addWidget(primaryColorLabel);

    secondaryColorLabel = new QLabel("Cor Secundaria: Verificando...");
    controlLayout->addWidget(secondaryColorLabel);

    mainLayout->addWidget(controlGroupBox);

    // 2. Dispositivos
    QGroupBox* devicesGroupBox = new QGroupBox("Dispositivos");
    QVBoxLayout* devicesLayout = new QVBoxLayout(devicesGroupBox);

    deviceCountLabel = new QLabel("Dispositivos detectados: 0");
    devicesLayout->addWidget(deviceCountLabel);

    // Placeholder for device list
    deviceListLayout = new QVBoxLayout();
    devicesLayout->addLayout(deviceListLayout);

    mainLayout->addWidget(devicesGroupBox);

    // 3. Configurações
    QGroupBox* settingsGroupBox = new QGroupBox("Configurações");
    QGridLayout* settingsLayout = new QGridLayout(settingsGroupBox);

    settingsLayout->addWidget(new QLabel("Intervalo de Sincronização (ms):"), 0, 0);
    syncIntervalSpinbox = new QSpinBox();
    syncIntervalSpinbox->setRange(50, 5000);
    syncIntervalSpinbox->setValue(100);
    settingsLayout->addWidget(syncIntervalSpinbox, 0, 1);

    enableBrightnessCheckbox = new QCheckBox("Habilitar controle de brilho");
    settingsLayout->addWidget(enableBrightnessCheckbox, 1, 0, 1, 2);

    brightnessContainer = new QWidget();
    QHBoxLayout* brightnessLayout = new QHBoxLayout(brightnessContainer);
    brightnessSlider = new QSlider(Qt::Horizontal);
    brightnessSlider->setRange(0, 10);
    brightnessSlider->setValue(10);
    brightnessLayout->addWidget(new QLabel("Substituir Brilho:"));
    brightnessLayout->addWidget(brightnessSlider);
    brightnessValueLabel = new QLabel("1.0");
    brightnessLayout->addWidget(brightnessValueLabel);
    settingsLayout->addWidget(brightnessContainer, 2, 0, 1, 2);

    mainLayout->addWidget(settingsGroupBox);

    // 4. Informações do Sistema
    QGroupBox* infoGroupBox = new QGroupBox("Informações do Sistema");
    QVBoxLayout* infoLayout = new QVBoxLayout(infoGroupBox);

    osInfoLabel = new QLabel("OS: Verificando...");
    infoLayout->addWidget(osInfoLabel);

    compatibilityLabel = new QLabel("Compatibilidade: Verificando...");
    infoLayout->addWidget(compatibilityLabel);

    versionLabel = new QLabel("Versão do Plugin: " VERSION_STRING " (Commit: " GIT_COMMIT_ID ")");
    infoLayout->addWidget(versionLabel);

    urlLabel = new QLabel("URL: <a href=\"https://github.com/Oraculo-sh/OpenRGBWindowsDynamicLightingSyncPlugin\">https://github.com/Oraculo-sh/OpenRGBWindowsDynamicLightingSyncPlugin</a>");
    urlLabel->setOpenExternalLinks(true);
    infoLayout->addWidget(urlLabel);

    QHBoxLayout* buttonsLayout = new QHBoxLayout();
    updateButton = new QPushButton("Verificar Atualizações");
    reloadButton = new QPushButton("Recarrega Plugin");
    buttonsLayout->addWidget(updateButton);
    buttonsLayout->addWidget(reloadButton);
    infoLayout->addLayout(buttonsLayout);

    mainLayout->addWidget(infoGroupBox);

    mainLayout->addStretch();

    // Conectar sinais aos slots
    connect(enableSyncCheckbox, &QCheckBox::toggled, this, &WindowsDynamicLightingSync::onEnableSyncCheckboxToggled);
    connect(syncIntervalSpinbox, QOverload<int>::of(&QSpinBox::valueChanged), this, &WindowsDynamicLightingSync::onSyncIntervalSpinboxValueChanged);
    connect(enableBrightnessCheckbox, &QCheckBox::toggled, this, &WindowsDynamicLightingSync::onEnableBrightnessCheckboxToggled);
    connect(brightnessSlider, &QSlider::valueChanged, this, &WindowsDynamicLightingSync::onBrightnessSliderValueChanged);
    connect(updateButton, &QPushButton::clicked, this, &WindowsDynamicLightingSync::onUpdateButtonClicked);
    connect(reloadButton, &QPushButton::clicked, this, &WindowsDynamicLightingSync::onReloadButtonClicked);

    // Inicializar o estado do brightnessContainer
    brightnessContainer->setEnabled(enableBrightnessCheckbox->isChecked());

    return mainWidget;
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

WindowsDynamicLightingSync::WindowsDynamicLightingSync() : mainWidget(nullptr)
{
    printf("[WindowsDynamicLightingSync] Constructor.\n");
}

WindowsDynamicLightingSync::~WindowsDynamicLightingSync()
{
     printf("[WindowsDynamicLightingSync] Time to free some memory.\n");
}

void WindowsDynamicLightingSync::onEnableSyncCheckboxToggled(bool checked)
{
    // Implementar lógica para habilitar/desabilitar sincronização
    printf("[WindowsDynamicLightingSync] Enable Sync Checkbox Toggled: %d\n", checked);
}

void WindowsDynamicLightingSync::onSyncIntervalSpinboxValueChanged(int value)
{
    // Implementar lógica para ajustar o intervalo de sincronização
    printf("[WindowsDynamicLightingSync] Sync Interval Spinbox Value Changed: %d\n", value);
}

void WindowsDynamicLightingSync::onEnableBrightnessCheckboxToggled(bool checked)
{
    // Implementar lógica para habilitar/desabilitar controle de brilho
    printf("[WindowsDynamicLightingSync] Enable Brightness Checkbox Toggled: %d\n", checked);
    brightnessContainer->setEnabled(checked);
}

void WindowsDynamicLightingSync::onBrightnessSliderValueChanged(int value)
{
    // Implementar lógica para ajustar o brilho
    double brightness = (double)value / 10.0;
    brightnessValueLabel->setText(QString::number(brightness, 'f', 1));
    printf("[WindowsDynamicLightingSync] Brightness Slider Value Changed: %f\n", brightness);
}

void WindowsDynamicLightingSync::onUpdateButtonClicked()
{
    // Implementar lógica para verificar atualizações
    printf("[WindowsDynamicLightingSync] Update Button Clicked.\n");
}

void WindowsDynamicLightingSync::onReloadButtonClicked()
{
    // Implementar lógica para recarregar o plugin
    printf("[WindowsDynamicLightingSync] Reload Button Clicked.\n");
}

