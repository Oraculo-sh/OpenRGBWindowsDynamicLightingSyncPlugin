#include "WindowsDynamicLightingSync.h"
#include <QApplication>
#include <QStyleFactory>
#include <QMessageBox>
#include <QJsonObject>
#include <QJsonDocument>
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QMenu>
#include <QAction>
#include <QSlider>
#include <QTime>
#include "json.hpp"

#ifdef _WIN32
#include <windows.h>
#include <winrt/base.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Devices.h>
#include <winrt/Windows.Devices.Enumeration.h>
#include <winrt/Windows.Devices.Lights.h>
#include <winrt/Windows.UI.h>
using namespace winrt;
using namespace Windows::Foundation;
using namespace Windows::Devices;
using namespace Windows::Devices::Enumeration;
using namespace Windows::Devices::Lights;
using namespace Windows::UI;
#endif



using namespace std::chrono;

WindowsDynamicLightingSync::WindowsDynamicLightingSync()
    : sync_timer(nullptr)
    , is_loaded(false)
    , dark_theme_enabled(false)
    , sync_enabled(true)
    , sync_interval_ms(100)
    , logging_enabled(false)
    , widget(nullptr)
    , enableCheckBox(nullptr)
    , deviceCountLabel(nullptr)
    , syncTimer(nullptr)
    , isDynamicLightingEnabled(false)
    , darkTheme(false)

    , brightness_multiplier(1.0f)
    , log_file_path("WindowsDynamicLightingSync.log")
    , show_error_dialogs(false)
    , max_log_file_size(10 * 1024 * 1024)
#ifdef _WIN32
    , lampArrayWatcher(nullptr)
    , lampArrays(nullptr)
    , windowsLightingInitialized(false)
#endif
{
    // Plugin initialized without logging to prevent crashes
}

WindowsDynamicLightingSync::~WindowsDynamicLightingSync()
{
    if (is_loaded)
    {
        Unload();
    }
}

OpenRGBPluginInfo WindowsDynamicLightingSync::GetPluginInfo()
{
    OpenRGBPluginInfo info;
    
    info.Name           = "Windows Dynamic Lighting Sync";
    info.Description    = "Synchronizes RGB devices with Windows Dynamic Lighting API";
    info.Version        = "1.0.0";
    info.Commit         = GIT_COMMIT_ID;
    info.URL            = "https://github.com/Oraculo-sh/OpenRGBWindowsDynamicLightingSyncPlugin";
    
    info.Icon.load(":/WindowsDynamicLightingSync.png");
    
    info.Location       = OPENRGB_PLUGIN_LOCATION_TOP;
    info.Label          = "Dynamic Lighting";
    info.TabIconString  = "Dynamic Lighting";
    
    return info;
}

unsigned int WindowsDynamicLightingSync::GetPluginAPIVersion()
{
    return OPENRGB_PLUGIN_API_VERSION;
}

// Static member definition
ResourceManagerInterface* WindowsDynamicLightingSync::RMPointer = nullptr;

void WindowsDynamicLightingSync::Load(ResourceManagerInterface* resource_manager_ptr)
{
    RMPointer = resource_manager_ptr;
    
    // Initialize sync timer
    sync_timer = new QTimer(this);
    connect(sync_timer, &QTimer::timeout, this, &WindowsDynamicLightingSync::OnSyncTimer);
    
    // Initialize Windows Dynamic Lighting API
#ifdef _WIN32
    try {
        InitializeDynamicLighting();
        // Register plugin as device after successful initialization
        RegisterPluginAsDevice();
    } catch (...) {
        // Ignore initialization errors to prevent crashes
    }
#endif
    
    is_loaded = true;
}

QWidget* WindowsDynamicLightingSync::GetWidget()
{
    if (!widget)
    {
        setupUI();
    }
    return widget;
}

void WindowsDynamicLightingSync::setupUI()
{
    widget = new QWidget();
    
    // Layout principal
    QVBoxLayout* mainLayout = new QVBoxLayout(widget);
    
    // === 1. Submenu: Controle Dynamic Lighting Sync Plugin ===
    QGroupBox* controlGroup = new QGroupBox("Controle Dynamic Lighting Sync Plugin");
    QVBoxLayout* controlLayout = new QVBoxLayout(controlGroup);
    
    // Checkbox principal para habilitar/desabilitar
    enableCheckBox = new QCheckBox("Habilitar Dynamic Lighting Sync");
    enableCheckBox->setChecked(isDynamicLightingEnabled);
    connect(enableCheckBox, &QCheckBox::toggled, this, &WindowsDynamicLightingSync::onToggleDynamicLighting);
    
    // Labels de status da API e sistema
    apiStatusLabel = new QLabel("API Windows Dynamic Lighting: Conectando...");
    systemStatusLabel = new QLabel("Status Windows Dynamic Lighting: (Verificando)");
    currentEffectLabel = new QLabel("Efeito atual: (Não detectado)");
    directionLabel = new QLabel("Direção: (Não aplicável)");
    primaryColorLabel = new QLabel("Cor Principal: (Não definida)");
    secondaryColorLabel = new QLabel("Cor Secundaria: (Não definida)");
    
    controlLayout->addWidget(enableCheckBox);
    controlLayout->addWidget(apiStatusLabel);
    controlLayout->addWidget(systemStatusLabel);
    controlLayout->addWidget(currentEffectLabel);
    controlLayout->addWidget(directionLabel);
    controlLayout->addWidget(primaryColorLabel);
    controlLayout->addWidget(secondaryColorLabel);
    
    mainLayout->addWidget(controlGroup);
    
    // === 2. Submenu: Dispositivos ===
    QGroupBox* devicesGroup = new QGroupBox("Dispositivos");
    QVBoxLayout* devicesLayout = new QVBoxLayout(devicesGroup);
    
    // Contador de dispositivos
    deviceCountLabel = new QLabel("Dispositivos detectados: 0");
    devicesLayout->addWidget(deviceCountLabel);
    
    // Container para lista de dispositivos
    deviceListWidget = new QWidget();
    deviceListLayout = new QVBoxLayout(deviceListWidget);
    
    QScrollArea* deviceScrollArea = new QScrollArea();
    deviceScrollArea->setWidget(deviceListWidget);
    deviceScrollArea->setWidgetResizable(true);
    deviceScrollArea->setMaximumHeight(200);
    
    devicesLayout->addWidget(deviceScrollArea);
    mainLayout->addWidget(devicesGroup);
    
    // === 3. Submenu: Configurações ===
    QGroupBox* configGroup = new QGroupBox("Configurações");
    QVBoxLayout* configLayout = new QVBoxLayout(configGroup);
    
    // Configuração de intervalo de sincronização
    QHBoxLayout* intervalLayout = new QHBoxLayout();
    QLabel* intervalLabel = new QLabel("Intervalo de Sincronização (ms):");
    QSpinBox* intervalSpinBox = new QSpinBox();
    intervalSpinBox->setRange(50, 5000);
    intervalSpinBox->setValue(sync_interval_ms);
    intervalSpinBox->setSuffix(" ms");
    connect(intervalSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), [this](int value) {
        sync_interval_ms = value;
        if (syncTimer && syncTimer->isActive()) {
            syncTimer->setInterval(value);
        }
        SaveSettings();
    });
    intervalLayout->addWidget(intervalLabel);
    intervalLayout->addWidget(intervalSpinBox);
    
    // Controle de brilho
    brightnessControlCheckBox = new QCheckBox("Habilitar controle de brilho");
    brightnessControlCheckBox->setChecked(false);
    
    QHBoxLayout* brightnessLayout = new QHBoxLayout();
    QLabel* brightnessLabel = new QLabel("Substituir Brilho:");
    brightnessSlider = new QSlider(Qt::Horizontal);
    brightnessSlider->setRange(0, 20); // 0.0 to 2.0 with 0.1 steps
    brightnessSlider->setValue(10); // 1.0 default
    brightnessSlider->setEnabled(false);
    
    brightnessValueLabel = new QLabel("1.0");
    
    connect(brightnessControlCheckBox, &QCheckBox::toggled, [this](bool checked) {
        brightnessSlider->setEnabled(checked);
        SaveSettings();
    });
    
    connect(brightnessSlider, &QSlider::valueChanged, [this](int value) {
        float brightness = value / 10.0f;
        brightness_multiplier = brightness;
        brightnessValueLabel->setText(QString::number(brightness, 'f', 1));
        SaveSettings();
    });
    
    brightnessLayout->addWidget(brightnessLabel);
    brightnessLayout->addWidget(brightnessSlider);
    brightnessLayout->addWidget(brightnessValueLabel);
    
    configLayout->addLayout(intervalLayout);
    configLayout->addWidget(brightnessControlCheckBox);
    configLayout->addLayout(brightnessLayout);
    
    mainLayout->addWidget(configGroup);
    
    // === 4. Submenu: Informações do Sistema ===
    QGroupBox* infoGroup = new QGroupBox("Informações do Sistema");
    QVBoxLayout* infoLayout = new QVBoxLayout(infoGroup);
    
    // Informações do sistema
    osInfoLabel = new QLabel("OS: Detectando...");
    compatibilityLabel = new QLabel("Compatibilidade com Windows Dynamic Lighting: Verificando...");
    versionLabel = new QLabel(QString("Versão do Plugin: 1.0.0 (Commit: %1)").arg(GIT_COMMIT_ID));
    urlLabel = new QLabel("<a href='https://github.com/Oraculo-sh/OpenRGBWindowsDynamicLightingSyncPlugin'>https://github.com/Oraculo-sh/OpenRGBWindowsDynamicLightingSyncPlugin</a>");
    urlLabel->setOpenExternalLinks(true);
    
    // Botão para atualizar informações
    QPushButton* refreshButton = new QPushButton("Atualizar Informações");
    connect(refreshButton, &QPushButton::clicked, [this]() {
        updateSystemInfo();
        updateDeviceList();
    });
    
    infoLayout->addWidget(osInfoLabel);
    infoLayout->addWidget(compatibilityLabel);
    infoLayout->addWidget(versionLabel);
    infoLayout->addWidget(urlLabel);
    infoLayout->addWidget(refreshButton);
    
    mainLayout->addWidget(infoGroup);
    
    // Inicializar informações do sistema
    updateSystemInfo();
    
    // Initialize sync timer
    syncTimer = new QTimer(this);
    connect(syncTimer, &QTimer::timeout, this, [this]() {
        // Perform device list updates and sync operations
        updateDeviceList();
        if (isDynamicLightingEnabled) {
            SyncWithDynamicLighting();
        }
    });
    syncTimer->start(sync_interval_ms); // Use configurable interval
    
    // Atualizar lista de dispositivos
    updateDeviceList();
}

void WindowsDynamicLightingSync::onToggleDynamicLighting()
{
    if (sender() == enableCheckBox)
    {
        isDynamicLightingEnabled = enableCheckBox->isChecked();
        
        if (isDynamicLightingEnabled)
        {
            if (apiStatusLabel) {
                apiStatusLabel->setText("API Windows Dynamic Lighting: ✓ Habilitado");
            }
        }
        else
        {
            if (apiStatusLabel) {
                apiStatusLabel->setText("API Windows Dynamic Lighting: ○ Desabilitado");
            }
            enableDynamicLighting(false);
        }
    }
}

void WindowsDynamicLightingSync::enableDynamicLighting(bool enable)
{
    if (enable && isDynamicLightingEnabled)
    {
        // Iniciar sincronização
        syncTimer->start(100); // Atualizar a cada 100ms
        if (systemStatusLabel) {
            systemStatusLabel->setText("Status Windows Dynamic Lighting: ✓ Ativo - Sincronizando");
        }
        
        // Aplicar iluminação inicial
        applyLightingToAllDevices();
    }
    else
    {
        // Parar sincronização
        if (syncTimer->isActive())
        {
            syncTimer->stop();
        }
        if (systemStatusLabel) {
            systemStatusLabel->setText("Status Windows Dynamic Lighting: ○ Inativo");
        }
    }
}

void WindowsDynamicLightingSync::updateDeviceList()
{
    try {
        if (!RMPointer)
        {
            if (deviceCountLabel) {
                deviceCountLabel->setText("Dispositivos detectados: 0 (Erro: RM não inicializado)");
            }
            return;
        }
        
        std::vector<RGBController*> controllers = RMPointer->GetRGBControllers();
        int deviceCount = static_cast<int>(controllers.size());
        
        if (deviceCountLabel) {
            deviceCountLabel->setText(QString("Dispositivos detectados: %1").arg(deviceCount));
        }
        
        // Atualizar lista visual de dispositivos
        updateDeviceListUI();
    } catch (...) {
        // Ignore errors
    }
}

void WindowsDynamicLightingSync::updateDeviceListUI()
{
    if (!deviceListWidget || !RMPointer) {
        return;
    }
    
    // Limpar lista atual
    if (deviceListLayout) {
        QLayoutItem* item;
        while ((item = deviceListLayout->takeAt(0)) != nullptr) {
            delete item->widget();
            delete item;
        }
    }
    
    // Usar o layout existente
    QVBoxLayout* listLayout = deviceListLayout;
    
    try {
        std::vector<RGBController*> controllers = RMPointer->GetRGBControllers();
        
        if (controllers.empty()) {
            QLabel* noDevicesLabel = new QLabel("Nenhum dispositivo RGB detectado");
            noDevicesLabel->setAlignment(Qt::AlignCenter);
            noDevicesLabel->setStyleSheet("color: #666; font-style: italic; padding: 20px;");
            listLayout->addWidget(noDevicesLabel);
        } else {
            for (size_t i = 0; i < controllers.size(); i++) {
                RGBController* controller = controllers[i];
                if (!controller) continue;
                
                // Criar frame para cada dispositivo
                QFrame* deviceFrame = new QFrame();
                deviceFrame->setFrameStyle(QFrame::Box);
                deviceFrame->setStyleSheet("QFrame { border: 1px solid #ccc; border-radius: 5px; margin: 2px; padding: 5px; }");
                
                QVBoxLayout* deviceLayout = new QVBoxLayout(deviceFrame);
                
                // Nome do dispositivo
                QLabel* nameLabel = new QLabel(QString::fromStdString(controller->name));
                nameLabel->setStyleSheet("font-weight: bold; color: #333;");
                
                // Informações do dispositivo
                QLabel* infoLabel = new QLabel(QString("Tipo: %1 | LEDs: %2 | Zonas: %3")
                    .arg(QString::fromStdString(controller->name))
                    .arg(controller->leds.size())
                    .arg(controller->zones.size()));
                infoLabel->setStyleSheet("color: #666; font-size: 11px;");
                
                // Status de compatibilidade
                QLabel* statusLabel = new QLabel();
                bool isCompatible = controller->colors.size() > 0;
                if (isCompatible) {
                    statusLabel->setText("✓ Compatível");
                    statusLabel->setStyleSheet("color: #4CAF50; font-weight: bold;");
                } else {
                    statusLabel->setText("⚠ Limitado");
                    statusLabel->setStyleSheet("color: #FF9800; font-weight: bold;");
                }
                
                deviceLayout->addWidget(nameLabel);
                deviceLayout->addWidget(infoLabel);
                deviceLayout->addWidget(statusLabel);
                
                listLayout->addWidget(deviceFrame);
            }
        }
    } catch (...) {
        QLabel* errorLabel = new QLabel("Erro ao carregar dispositivos");
        errorLabel->setAlignment(Qt::AlignCenter);
        errorLabel->setStyleSheet("color: #f44336; font-style: italic; padding: 20px;");
        listLayout->addWidget(errorLabel);
    }
    
    listLayout->addStretch();
}

void WindowsDynamicLightingSync::onBrightnessChanged(int value)
{
    brightness_multiplier = value / 100.0f; // Converter de 0-100 para 0.0-1.0
    
    // Atualizar label do valor
    if (brightnessValueLabel) {
        brightnessValueLabel->setText(QString("%1%").arg(value));
    }
    
    // Salvar configurações
    SaveSettings();
    
    // Aplicar nova configuração de brilho se estiver ativo
    if (isDynamicLightingEnabled) {
        applyLightingToAllDevices();
    }
}

void WindowsDynamicLightingSync::syncLighting()
{
    SafeExecute([this]() {
        if (!isDynamicLightingEnabled || !RMPointer)
        {
            // Sync lighting called but conditions not met
            return;
        }
        
        // Sincronizar com Windows Dynamic Lighting se disponível
#ifdef _WIN32
        if (windowsLightingInitialized && !connectedLampArrays.empty())
        {
            syncWithWindowsLighting();
            // Successfully synced with Windows Dynamic Lighting
        }
        else
#endif
        {
            // Applying fallback lighting to all devices
            // Fallback: aplicar uma cor simples para teste
            applyLightingToAllDevices();
        }
    }, "syncLighting");
}

void WindowsDynamicLightingSync::applyLightingToAllDevices()
{
    if (!RMPointer)
    {
        return;
    }
    
    try
    {
        std::vector<RGBController*> controllers = RMPointer->GetRGBControllers();
        
        for (RGBController* controller : controllers)
        {
            if (controller && controller->colors.size() > 0)
            {
                // Aplicar cor a todos os LEDs com multiplicador de brilho
                for (unsigned int i = 0; i < controller->colors.size(); i++)
                {
                    // Usar cor vermelha como padrão para teste
                    unsigned char r = 255;
                    unsigned char g = 0;
                    unsigned char b = 0;
                    
                    // Aplicar multiplicador de brilho
                    r = static_cast<unsigned char>(r * brightness_multiplier);
                    g = static_cast<unsigned char>(g * brightness_multiplier);
                    b = static_cast<unsigned char>(b * brightness_multiplier);
                    
                    // Recompor a cor
                    RGBColor adjustedColor = (r << 16) | (g << 8) | b;
                    controller->colors[i] = adjustedColor;
                }
                
                // Atualizar dispositivo
                controller->UpdateLEDs();
            }
        }
    }
    catch (...)
    {
        // Ignore errors
    }
}

QMenu* WindowsDynamicLightingSync::GetTrayMenu()
{
    QMenu* menu = new QMenu("Windows Dynamic Lighting");
    
    QAction* toggleAction = new QAction(isDynamicLightingEnabled ? "Desabilitar" : "Habilitar", menu);
    connect(toggleAction, &QAction::triggered, [this]() {
        enableCheckBox->setChecked(!isDynamicLightingEnabled);
        onToggleDynamicLighting();
    });
    
    QAction* configAction = new QAction("Configurar", menu);
    connect(configAction, &QAction::triggered, [this]() {
        QMessageBox::information(nullptr, "Configuração", "Interface de configuração em desenvolvimento.");
    });
    
    menu->addAction(toggleAction);
    menu->addSeparator();
    menu->addAction(configAction);
    
    return menu;
}

void WindowsDynamicLightingSync::Unload()
{
    if (sync_timer && sync_timer->isActive())
    {
        sync_timer->stop();
    }
    
#ifdef _WIN32
    try {
        CleanupDynamicLighting();
    } catch (...) {
        // Ignore cleanup errors
    }
#endif
    
    is_loaded = false;
}

// Slot methods
void WindowsDynamicLightingSync::OnSyncTimer()
{
    QMutexLocker locker(&sync_mutex);
    
    if (!is_loaded || !sync_enabled)
    {
        return;
    }
    
    try
    {
        // Performing sync cycle
        SyncWithDynamicLighting();
    }
    catch (const std::exception& e)
    {
        // Error during sync: " + std::string(e.what())
    }
}

void WindowsDynamicLightingSync::OnDeviceAdded()
{
    // Device added, updating device list
    updateDeviceList();
}

void WindowsDynamicLightingSync::OnDeviceRemoved()
{
    // Device removed, updating device list
    updateDeviceList();
}

// Settings management
void WindowsDynamicLightingSync::LoadSettings()
{
    if (!RMPointer)
    {
        return;
    }
    
    try {
        SettingsManager* settings = RMPointer->GetSettingsManager();
        if (!settings)
        {
            return;
        }
        
        json plugin_settings = settings->GetSettings("WindowsDynamicLightingSync");
        
        sync_enabled = plugin_settings.value("sync_enabled", true);
        sync_interval_ms = plugin_settings.value("sync_interval_ms", 100);
        brightness_multiplier = plugin_settings.value("brightness_multiplier", 1.0f);
        // Funcionalidades removidas: bidirectional_sync, smooth_transitions, auto_detect_devices, logging_enabled
        logging_enabled = false; // Force disable logging
        show_error_dialogs = false; // Force disable error dialogs
        max_log_file_size = plugin_settings.value("max_log_file_size", 10 * 1024 * 1024);
    } catch (...) {
        // Use defaults if loading fails
    }
}

void WindowsDynamicLightingSync::SaveSettings()
{
    if (!RMPointer)
    {
        return;
    }
    
    try {
        SettingsManager* settings = RMPointer->GetSettingsManager();
        if (!settings)
        {
            return;
        }
        
        json plugin_settings;
        plugin_settings["sync_enabled"] = sync_enabled;
        plugin_settings["sync_interval_ms"] = sync_interval_ms;
        plugin_settings["brightness_multiplier"] = brightness_multiplier;
        // Funcionalidades removidas: bidirectional_sync, smooth_transitions, auto_detect_devices
        plugin_settings["enable_logging"] = false; // Force disable logging
        plugin_settings["show_error_dialogs"] = false; // Force disable error dialogs
        plugin_settings["max_log_file_size"] = max_log_file_size;
        
        settings->SetSettings("WindowsDynamicLightingSync", plugin_settings);
        settings->SaveSettings();
    } catch (...) {
        // Ignore save errors
    }
}

// Logging system removed - OpenRGB handles all logging

// System information update
void WindowsDynamicLightingSync::updateSystemInfo()
{
    // Update OS information
    if (osInfoLabel)
    {
#ifdef _WIN32
        OSVERSIONINFOEX osvi;
        ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
        osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
        
        QString osInfo = "OS: Windows ";
        
        // Get Windows version using GetVersionEx (deprecated but still works)
        if (GetVersionEx((OSVERSIONINFO*)&osvi))
        {
            osInfo += QString("%1.%2 Build %3")
                .arg(osvi.dwMajorVersion)
                .arg(osvi.dwMinorVersion)
                .arg(osvi.dwBuildNumber);
        }
        else
        {
            osInfo += "(Versão não detectada)";
        }
        
        osInfoLabel->setText(osInfo);
#else
        osInfoLabel->setText("OS: Não Windows (Plugin não compatível)");
#endif
    }
    
    // Update compatibility information
    if (compatibilityLabel)
    {
#ifdef _WIN32
        bool isSupported = CheckDynamicLightingSupport();
        QString compatText = "Compatibilidade com Windows Dynamic Lighting: ";
        if (isSupported)
        {
            compatText += "✓ Suportado";
        }
        else
        {
            compatText += "✗ Não suportado (Windows 11 22H2+ necessário)";
        }
        compatibilityLabel->setText(compatText);
#else
        compatibilityLabel->setText("Compatibilidade com Windows Dynamic Lighting: ✗ Não Windows");
#endif
    }
    
    // Update API status
    if (apiStatusLabel)
    {
#ifdef _WIN32
        if (windowsLightingInitialized)
        {
            apiStatusLabel->setText("API Windows Dynamic Lighting: ✓ Conectado");
        }
        else
        {
            apiStatusLabel->setText("API Windows Dynamic Lighting: ✗ Não conectado");
        }
#else
        apiStatusLabel->setText("API Windows Dynamic Lighting: ✗ Não disponível");
#endif
    }
    
    // Update system status
    if (systemStatusLabel)
    {
        if (isDynamicLightingEnabled)
        {
            systemStatusLabel->setText("Status Windows Dynamic Lighting: ✓ Ativo");
        }
        else
        {
            systemStatusLabel->setText("Status Windows Dynamic Lighting: ○ Inativo");
        }
    }
}

// Device management - Simplified to only list OpenRGB devices
// Removed: DetectCompatibleDevices() - OpenRGB handles device detection
// Removed: RefreshDeviceList() - Unnecessary duplicate detection logic

std::vector<RGBController*> WindowsDynamicLightingSync::GetCompatibleControllers()
{
    std::vector<RGBController*> compatible_controllers;
    
    if (!RMPointer)
    {
        // ResourceManager not available for getting controllers
        return compatible_controllers;
    }
    
    std::vector<RGBController*> controllers = RMPointer->GetRGBControllers();
    // Scanning " + std::to_string(controllers.size()) + " total RGB controllers
    
    for (RGBController* controller : controllers)
    {
        if (!controller)
        {
            // Found null controller, skipping
            continue;
        }
        
        if (controller->modes.size() == 0)
        {
            // Controller '" + controller->name + "' has no modes, skipping
            continue;
        }
        
        bool is_compatible = false;
        std::string compatibility_reason = "";
        
        // Check for per-LED color support (highest priority)
        for (const mode& mode : controller->modes)
        {
            if (mode.flags & MODE_FLAG_HAS_PER_LED_COLOR)
            {
                is_compatible = true;
                compatibility_reason = "supports per-LED color control";
                break;
            }
        }
        
        // If no per-LED support, check for basic color support
        if (!is_compatible)
        {
            for (const mode& mode : controller->modes)
            {
                if (mode.flags & MODE_FLAG_HAS_MODE_SPECIFIC_COLOR)
                {
                    is_compatible = true;
                    compatibility_reason = "supports mode-specific color control";
                    break;
                }
            }
        }
        
        // Check for minimum LED count (avoid single LED devices for better sync)
        if (is_compatible && controller->leds.size() < 3)
        {
            // Controller '" + controller->name + "' has only " + std::to_string(controller->leds.size()) + " LEDs, may have limited sync capability
        }
        
        if (is_compatible)
        {
            compatible_controllers.push_back(controller);
            // ✓ Compatible: '" + controller->name + "' (" + controller->name + ") - " + compatibility_reason + ", " + std::to_string(controller->leds.size()) + " LEDs
        }
        else
        {
            // ✗ Incompatible: '" + controller->name + "' - no suitable color control modes
        }
    }
    
    // Found " + std::to_string(compatible_controllers.size()) + " compatible controllers out of " + std::to_string(controllers.size()) + " total
    return compatible_controllers;
}

void WindowsDynamicLightingSync::UpdateDeviceColors()
{
    std::vector<RGBController*> controllers = GetCompatibleControllers();
    
    for (RGBController* controller : controllers)
    {
        if (controller)
        {
            // This will be implemented with actual color sync logic
            // For now, just log the update
            // Updating colors for device: " + controller->name
        }
    }
}

// Removed: SetAutoDetectDevices() and IsAutoDetectEnabled()
// Device detection is now handled exclusively by OpenRGB

#ifdef _WIN32
void WindowsDynamicLightingSync::InitializeDynamicLighting()
{
    try
    {
        // Initializing Windows Dynamic Lighting API
        
        // Initialize Windows Runtime
        winrt::init_apartment();
        
        // Check if Dynamic Lighting is supported
        if (!CheckDynamicLightingSupport())
        {
            // Windows Dynamic Lighting is not supported on this system
            return;
        }
        
        // Criar seletor para dispositivos LampArray
        hstring lampArraySelector = LampArray::GetDeviceSelector();
        
        // Criar DeviceWatcher para monitorar dispositivos LampArray
        lampArrayWatcher = DeviceInformation::CreateWatcher(lampArraySelector);
        
        // Configurar eventos do watcher
        lampArrayWatcher.Added({ this, &WindowsDynamicLightingSync::onLampArrayAdded });
        lampArrayWatcher.Removed({ this, &WindowsDynamicLightingSync::onLampArrayRemoved });
        
        // Iniciar monitoramento
        lampArrayWatcher.Start();
        
        windowsLightingInitialized = true;
        // Windows Dynamic Lighting API initialized successfully
    }
    catch (...)
    {
        // Falha na inicialização - continuar sem Windows Dynamic Lighting
        windowsLightingInitialized = false;
        // Failed to initialize Windows Dynamic Lighting
    }
}

bool WindowsDynamicLightingSync::CheckDynamicLightingSupport()
{
    try
    {
        // Check Windows version and Dynamic Lighting support
        // This is a placeholder - actual implementation would check:
        // - Windows 11 version
        // - Dynamic Lighting capability
        // - Available LampArray devices
        
        // Checking Dynamic Lighting support
        return true; // Placeholder
    }
    catch (...)
    {
        // Error checking Dynamic Lighting support
        return false;
    }
}

void WindowsDynamicLightingSync::SyncWithDynamicLighting()
{
    try
    {
        if (!isDynamicLightingEnabled || !windowsLightingInitialized)
        {
            return;
        }
        
        // Perform bidirectional sync based on configuration
        if (true) // TODO: Add proper condition for bidirectional sync
        {
            // Sync OpenRGB colors to Windows Dynamic Lighting
            SyncOpenRGBToWindows();
            
            // Also sync Windows colors back to OpenRGB if needed
            // This creates a two-way synchronization
            SyncWindowsToOpenRGB();
        }
        else
        {
            // Only sync OpenRGB to Windows (one-way)
            SyncOpenRGBToWindows();
        }
        
        // Update device colors for any remaining devices
        UpdateDeviceColors();
    }
    catch (...)
    {
        // Error during Dynamic Lighting sync
    }
}

void WindowsDynamicLightingSync::CleanupDynamicLighting()
{
    try
    {
        // Unregister plugin as device before cleanup
        UnregisterPluginAsDevice();
        
        // Cleaning up Windows Dynamic Lighting API
        
        if (lampArrayWatcher != nullptr)
        {
            lampArrayWatcher.Stop();
            lampArrayWatcher = nullptr;
        }
        
        connectedLampArrays.clear();
        windowsLightingInitialized = false;
        
        // Windows Dynamic Lighting API cleaned up successfully
    }
    catch (...)
    {
        // Error during Dynamic Lighting cleanup
    }
}

bool WindowsDynamicLightingSync::RegisterPluginAsDevice()
{
    try
    {
        // Register this plugin as a device in the Windows Dynamic Lighting API
        // This allows the plugin to be recognized as a lighting device by the system
        
        if (!windowsLightingInitialized)
        {
            return false;
        }
        
        // Create device registration information
        // Note: This is a simplified implementation - actual Windows Dynamic Lighting
        // device registration may require additional WinRT API calls
        
        // Plugin successfully registered as device
        return true;
    }
    catch (...)
    {
        // Failed to register plugin as device
        return false;
    }
}

void WindowsDynamicLightingSync::UnregisterPluginAsDevice()
{
    try
    {
        // Unregister this plugin from the Windows Dynamic Lighting API
        // This removes the plugin from being recognized as a lighting device
        
        if (!windowsLightingInitialized)
        {
            return;
        }
        
        // Perform device unregistration
        // Note: This is a simplified implementation - actual Windows Dynamic Lighting
        // device unregistration may require additional WinRT API calls
        
        // Plugin successfully unregistered as device
    }
    catch (...)
    {
        // Failed to unregister plugin as device
    }
}

void WindowsDynamicLightingSync::onLampArrayAdded(DeviceWatcher const& sender, DeviceInformation const& deviceInfo)
{
    try
    {
        // Obter LampArray do dispositivo
        auto lampArrayAsync = LampArray::FromIdAsync(deviceInfo.Id());
        auto lampArray = lampArrayAsync.get();
        
        if (lampArray != nullptr)
        {
            connectedLampArrays.push_back(lampArray);
            
            // Atualizar UI na thread principal
            QMetaObject::invokeMethod(this, [this]() {
                updateDeviceList();
            }, Qt::QueuedConnection);
        }
    }
    catch (...)
    {
        // Ignorar erros ao adicionar dispositivo
    }
}

void WindowsDynamicLightingSync::onLampArrayRemoved(DeviceWatcher const& sender, DeviceInformationUpdate const& deviceInfoUpdate)
{
    try
    {
        // Remover dispositivo da lista
        auto it = std::remove_if(connectedLampArrays.begin(), connectedLampArrays.end(),
            [&deviceInfoUpdate](const LampArray& lampArray) {
                return lampArray.DeviceId() == deviceInfoUpdate.Id();
            });
        
        if (it != connectedLampArrays.end())
        {
            connectedLampArrays.erase(it, connectedLampArrays.end());
            
            // Atualizar UI na thread principal
            QMetaObject::invokeMethod(this, [this]() {
                updateDeviceList();
            }, Qt::QueuedConnection);
        }
    }
    catch (...)
     {
         // Ignorar erros ao remover dispositivo
     }
}

void WindowsDynamicLightingSync::syncWithWindowsLighting()
{
    try
    {
        if (!windowsLightingInitialized || connectedLampArrays.empty())
        {
            return;
        }
        
        // Obter controladores RGB do OpenRGB
        std::vector<RGBController*> controllers = RMPointer->GetRGBControllers();
        
        if (controllers.empty())
        {
            return;
        }
        
        // Para cada LampArray conectado
        for (const auto& lampArray : connectedLampArrays)
        {
            try
            {
                // Obter informações do LampArray
                auto lampCount = lampArray.LampCount();
                
                if (lampCount == 0)
                {
                    continue;
                }
                
                // Criar array de cores baseado no primeiro controlador OpenRGB
                RGBController* primaryController = controllers[0];
                
                if (primaryController && !primaryController->colors.empty())
                {
                    // Preparar cores para o LampArray
                    std::vector<Windows::UI::Color> lampColors;
                    lampColors.reserve(lampCount);
                    
                    for (uint32_t i = 0; i < lampCount; i++)
                    {
                        // Usar cor do LED correspondente ou a primeira cor se não houver LEDs suficientes
                        RGBColor rgbColor = (i < primaryController->colors.size()) ? 
                            primaryController->colors[i] : primaryController->colors[0];
                        
                        // Converter RGBColor para Windows::UI::Color
                        Windows::UI::Color winColor;
                        winColor.A = 255; // Alpha total
                        winColor.R = RGBGetRValue(rgbColor);
                        winColor.G = RGBGetGValue(rgbColor);
                        winColor.B = RGBGetBValue(rgbColor);
                        
                        lampColors.push_back(winColor);
                    }
                    
                    // Create indices array for all lamps
                    std::vector<int32_t> indices;
                    indices.reserve(lampCount);
                    for (uint32_t i = 0; i < lampCount; i++)
                    {
                        indices.push_back(static_cast<int32_t>(i));
                    }
                    
                    // Aplicar cores ao LampArray usando arrays nativos
                    winrt::array_view<Windows::UI::Color const> colorArrayView(lampColors);
                    winrt::array_view<int32_t const> indexArrayView(indices);
                    lampArray.SetColorsForIndices(colorArrayView, indexArrayView);
                }
            }
            catch (...)
            {
                // Ignorar erros em LampArrays individuais
                continue;
            }
        }
    }
    catch (...)
    {
        // Ignorar erros gerais de sincronização
    }
}

// Sync configuration methods




void WindowsDynamicLightingSync::SetSyncInterval(int interval_ms)
{
    if (interval_ms > 0 && interval_ms <= 1000)
    {
        sync_interval_ms = interval_ms;
        // Sync interval set to " + std::to_string(interval_ms) + "ms
        
        // Update timer interval if it's running
        if (syncTimer && syncTimer->isActive())
        {
            syncTimer->setInterval(interval_ms);
        }
    }
}

int WindowsDynamicLightingSync::GetSyncInterval() const
{
    return sync_interval_ms;
}



void WindowsDynamicLightingSync::SetBrightnessMultiplier(float multiplier)
{
    if (multiplier >= 0.1f && multiplier <= 2.0f)
    {
        brightness_multiplier = multiplier;
        // Brightness multiplier set to " + std::to_string(multiplier)
    }
}

float WindowsDynamicLightingSync::GetBrightnessMultiplier() const
{
    return brightness_multiplier;
}

bool WindowsDynamicLightingSync::SafeExecute(std::function<void()> func, const std::string& operation)
{
    try
    {
        func();
        return true;
    }
    catch (...)
    {
        return false;
    }
}

// Color conversion methods
RGBColor WindowsDynamicLightingSync::ConvertWindowsColorToRGB(const Windows::UI::Color& winColor)
{
    // Apply brightness multiplier
    uint8_t r = static_cast<uint8_t>(std::min(255.0f, winColor.R * brightness_multiplier));
    uint8_t g = static_cast<uint8_t>(std::min(255.0f, winColor.G * brightness_multiplier));
    uint8_t b = static_cast<uint8_t>(std::min(255.0f, winColor.B * brightness_multiplier));
    
    return ToRGBColor(r, g, b);
}

Windows::UI::Color WindowsDynamicLightingSync::ConvertRGBToWindowsColor(const RGBColor& rgbColor)
{
    Windows::UI::Color winColor;
    winColor.A = 255; // Full alpha
    
    // Apply brightness multiplier
    winColor.R = static_cast<uint8_t>(std::min(255.0f, RGBGetRValue(rgbColor) * brightness_multiplier));
    winColor.G = static_cast<uint8_t>(std::min(255.0f, RGBGetGValue(rgbColor) * brightness_multiplier));
    winColor.B = static_cast<uint8_t>(std::min(255.0f, RGBGetBValue(rgbColor) * brightness_multiplier));
    
    return winColor;
}

// Advanced sync methods
void WindowsDynamicLightingSync::SyncOpenRGBToWindows()
{
    try
    {
        if (!windowsLightingInitialized || connectedLampArrays.empty())
        {
            return;
        }
        
        std::vector<RGBController*> controllers = GetCompatibleControllers();
        if (controllers.empty())
        {
            return;
        }
        
        // Syncing OpenRGB to Windows Dynamic Lighting
        
        for (const auto& lampArray : connectedLampArrays)
        {
            try
            {
                auto lampCount = lampArray.LampCount();
                if (lampCount == 0) continue;
                
                // Use first compatible controller as source
                RGBController* sourceController = controllers[0];
                if (!sourceController || sourceController->colors.empty()) continue;
                
                std::vector<Windows::UI::Color> lampColors;
                lampColors.reserve(lampCount);
                
                for (uint32_t i = 0; i < lampCount; i++)
                {
                    RGBColor rgbColor = (i < sourceController->colors.size()) ? 
                        sourceController->colors[i] : sourceController->colors[0];
                    
                    lampColors.push_back(ConvertRGBToWindowsColor(rgbColor));
                }
                
                // Create indices array for all lamps
                std::vector<int32_t> indices;
                indices.reserve(lampCount);
                for (uint32_t i = 0; i < lampCount; i++)
                {
                    indices.push_back(static_cast<int32_t>(i));
                }
                
                // Aplicar cores ao LampArray usando arrays nativos
                winrt::array_view<Windows::UI::Color const> colorArrayView(lampColors);
                winrt::array_view<int32_t const> indexArrayView(indices);
                lampArray.SetColorsForIndices(colorArrayView, indexArrayView);
            }
            catch (...)
            {
                continue; // Skip failed lamp arrays
            }
        }
    }
    catch (...)
    {
        // Error during OpenRGB to Windows sync
    }
}

void WindowsDynamicLightingSync::SyncWindowsToOpenRGB()
{
    try
    {
        if (!windowsLightingInitialized || connectedLampArrays.empty())
        {
            return;
        }
        
        std::vector<RGBController*> controllers = GetCompatibleControllers();
        if (controllers.empty())
        {
            return;
        }
        
        // Syncing Windows Dynamic Lighting to OpenRGB
        
        // Get colors from first lamp array
        if (!connectedLampArrays.empty())
        {
            const auto& lampArray = connectedLampArrays[0];
            auto lampCount = lampArray.LampCount();
            
            if (lampCount > 0)
            {
                // This would require reading current colors from Windows Dynamic Lighting
                // For now, we'll implement a placeholder that applies a test pattern
                for (RGBController* controller : controllers)
                {
                    if (controller && !controller->colors.empty())
                    {
                        // Apply a simple pattern based on Windows lighting state
                        for (size_t i = 0; i < controller->colors.size(); i++)
                        {
                            // Placeholder: create a breathing effect
                            float intensity = (sin(QTime::currentTime().msec() * 0.01f) + 1.0f) * 0.5f;
                            uint8_t value = static_cast<uint8_t>(255 * intensity * brightness_multiplier);
                            controller->colors[i] = ToRGBColor(value, value / 2, value / 4);
                        }
                        
                        controller->UpdateLEDs();
                    }
                }
            }
        }
    }
    catch (...)
    {
        // Error during Windows to OpenRGB sync
    }
}
#endif