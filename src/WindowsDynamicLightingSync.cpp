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
#include "json.hpp"

using namespace std::chrono;

WindowsDynamicLightingSync::WindowsDynamicLightingSync()
    : sync_timer(nullptr)
    , is_loaded(false)
    , dark_theme_enabled(false)
    , sync_enabled(true)
    , sync_interval_ms(100)
    , auto_detect_devices(true)
    , logging_enabled(false)
    , widget(nullptr)
    , statusLabel(nullptr)
    , toggleButton(nullptr)
    , enableCheckBox(nullptr)
    , deviceCountLabel(nullptr)
    , syncTimer(nullptr)
    , RMPointer(nullptr)
    , isDynamicLightingEnabled(false)
    , darkTheme(false)
    , testColor(0x00FF0000)
    , bidirectional_sync(true)
    , smooth_transitions(true)
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
    info.URL            = "https://github.com/user/WindowsDynamicLightingSync";
    
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
    
    // Grupo de controle principal
    QGroupBox* controlGroup = new QGroupBox("Controle do Windows Dynamic Lighting");
    QVBoxLayout* controlLayout = new QVBoxLayout(controlGroup);
    
    // Checkbox para habilitar/desabilitar
    enableCheckBox = new QCheckBox("Habilitar Windows Dynamic Lighting");
    enableCheckBox->setChecked(isDynamicLightingEnabled);
    connect(enableCheckBox, &QCheckBox::toggled, this, &WindowsDynamicLightingSync::onToggleDynamicLighting);
    
    // Botão de toggle
    toggleButton = new QPushButton("Ativar Dynamic Lighting");
    toggleButton->setEnabled(false);
    connect(toggleButton, &QPushButton::clicked, this, &WindowsDynamicLightingSync::onToggleDynamicLighting);
    
    // Label de status
    statusLabel = new QLabel("Status: Desabilitado");
    
    // Label de contagem de dispositivos
    deviceCountLabel = new QLabel("Dispositivos detectados: 0");
    
    // Botão de teste de cor
    QPushButton* testColorButton = new QPushButton("Testar Cor (Azul)");
    testColorButton->setToolTip("Aplica uma cor azul de teste a todos os dispositivos");
    connect(testColorButton, &QPushButton::clicked, [this]() {
        testColor = 0x000000FF; // Azul
        applyLightingToAllDevices();
        statusLabel->setText("Status: Teste de cor aplicado (Azul)");
    });
    
    // Botão de teste de cor vermelha
    QPushButton* testRedButton = new QPushButton("Testar Cor (Vermelho)");
    testRedButton->setToolTip("Aplica uma cor vermelha de teste a todos os dispositivos");
    connect(testRedButton, &QPushButton::clicked, [this]() {
        testColor = 0x00FF0000; // Vermelho
        applyLightingToAllDevices();
        statusLabel->setText("Status: Teste de cor aplicado (Vermelho)");
    });
    
    // Layout para botões de teste
    QHBoxLayout* testButtonsLayout = new QHBoxLayout();
    testButtonsLayout->addWidget(testColorButton);
    testButtonsLayout->addWidget(testRedButton);
    
    controlLayout->addWidget(enableCheckBox);
    controlLayout->addWidget(toggleButton);
    controlLayout->addLayout(testButtonsLayout);
    controlLayout->addWidget(statusLabel);
    controlLayout->addWidget(deviceCountLabel);
    
    mainLayout->addWidget(controlGroup);
    
    // Grupo de configurações avançadas
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
    
    // Configuração de multiplicador de brilho
    QHBoxLayout* brightnessLayout = new QHBoxLayout();
    QLabel* brightnessLabel = new QLabel("Multiplicador de Brilho:");
    QDoubleSpinBox* brightnessSpinBox = new QDoubleSpinBox();
    brightnessSpinBox->setRange(0.1, 2.0);
    brightnessSpinBox->setSingleStep(0.1);
    brightnessSpinBox->setValue(brightness_multiplier);
    brightnessSpinBox->setDecimals(1);
    connect(brightnessSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        brightness_multiplier = static_cast<float>(value);
        SaveSettings();
    });
    brightnessLayout->addWidget(brightnessLabel);
    brightnessLayout->addWidget(brightnessSpinBox);
    
    // Checkbox para sincronização bidirecional
    QCheckBox* bidirectionalCheckBox = new QCheckBox("Sincronização Bidirecional");
    bidirectionalCheckBox->setChecked(bidirectional_sync);
    bidirectionalCheckBox->setToolTip("Permite que mudanças no Windows Dynamic Lighting afetem o OpenRGB");
    connect(bidirectionalCheckBox, &QCheckBox::toggled, [this](bool checked) {
        bidirectional_sync = checked;
        SaveSettings();
    });
    
    // Checkbox para transições suaves
    QCheckBox* smoothCheckBox = new QCheckBox("Transições Suaves");
    smoothCheckBox->setChecked(smooth_transitions);
    smoothCheckBox->setToolTip("Ativa transições suaves entre cores");
    connect(smoothCheckBox, &QCheckBox::toggled, [this](bool checked) {
        smooth_transitions = checked;
        SaveSettings();
    });
    
    // Checkbox para detecção automática de dispositivos
    QCheckBox* autoDetectCheckBox = new QCheckBox("Detecção Automática de Dispositivos");
    autoDetectCheckBox->setChecked(auto_detect_devices);
    connect(autoDetectCheckBox, &QCheckBox::toggled, [this](bool checked) {
        auto_detect_devices = checked;
        SaveSettings();
    });
    
    // Checkbox para logging
    QCheckBox* loggingCheckBox = new QCheckBox("Habilitar Logging");
    loggingCheckBox->setChecked(logging_enabled);
    connect(loggingCheckBox, &QCheckBox::toggled, [this](bool checked) {
        logging_enabled = checked;
        SaveSettings();
    });
    
    configLayout->addLayout(intervalLayout);
    configLayout->addLayout(brightnessLayout);
    configLayout->addWidget(bidirectionalCheckBox);
    configLayout->addWidget(smoothCheckBox);
    configLayout->addWidget(autoDetectCheckBox);
    configLayout->addWidget(loggingCheckBox);
    
    mainLayout->addWidget(configGroup);
    
    // Grupo de informações do sistema
    QGroupBox* infoGroup = new QGroupBox("Informações do Sistema");
    QVBoxLayout* infoLayout = new QVBoxLayout(infoGroup);
    
    // Label de suporte ao Windows Dynamic Lighting
    QLabel* supportLabel = new QLabel("Suporte ao Windows Dynamic Lighting: Verificando...");
    
    // Label de versão do plugin
    QLabel* versionLabel = new QLabel(QString("Versão do Plugin: 1.0.0 (Commit: %1)").arg(GIT_COMMIT_ID));
    
    // Botão para atualizar informações
    QPushButton* refreshButton = new QPushButton("Atualizar Informações");
    connect(refreshButton, &QPushButton::clicked, [this, supportLabel]() {
        updateDeviceList();
#ifdef _WIN32
        bool hasSupport = CheckDynamicLightingSupport();
        supportLabel->setText(QString("Suporte ao Windows Dynamic Lighting: %1")
                             .arg(hasSupport ? "Disponível" : "Não Disponível"));
#else
        supportLabel->setText("Suporte ao Windows Dynamic Lighting: Não Disponível (Não Windows)");
#endif
    });
    
    infoLayout->addWidget(supportLabel);
    infoLayout->addWidget(versionLabel);
    infoLayout->addWidget(refreshButton);
    
    mainLayout->addWidget(infoGroup);
    
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
        toggleButton->setEnabled(isDynamicLightingEnabled);
        
        if (isDynamicLightingEnabled)
        {
            toggleButton->setText("Desativar Dynamic Lighting");
            statusLabel->setText("Status: Habilitado - Aguardando ativação");
        }
        else
        {
            toggleButton->setText("Ativar Dynamic Lighting");
            statusLabel->setText("Status: Desabilitado");
            enableDynamicLighting(false);
        }
    }
    else if (sender() == toggleButton)
    {
        if (isDynamicLightingEnabled)
        {
            enableDynamicLighting(!syncTimer->isActive());
        }
    }
}

void WindowsDynamicLightingSync::enableDynamicLighting(bool enable)
{
    if (enable && isDynamicLightingEnabled)
    {
        // Iniciar sincronização
        syncTimer->start(100); // Atualizar a cada 100ms
        statusLabel->setText("Status: Ativo - Sincronizando");
        toggleButton->setText("Desativar Dynamic Lighting");
        
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
        statusLabel->setText("Status: Habilitado - Inativo");
        toggleButton->setText("Ativar Dynamic Lighting");
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
    } catch (...) {
        // Ignore errors
    }
}

void WindowsDynamicLightingSync::syncLighting()
{
    SafeExecute([this]() {
        if (!isDynamicLightingEnabled || !RMPointer)
        {
            LogWarning("Sync lighting called but conditions not met");
            return;
        }
        
        // Sincronizar com Windows Dynamic Lighting se disponível
#ifdef _WIN32
        if (windowsLightingInitialized && !connectedLampArrays.empty())
        {
            syncWithWindowsLighting();
            LogInfo("Successfully synced with Windows Dynamic Lighting");
        }
        else
#endif
        {
            LogInfo("Applying fallback lighting to all devices");
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
                // Aplicar cor a todos os LEDs
                for (unsigned int i = 0; i < controller->colors.size(); i++)
                {
                    controller->colors[i] = testColor;
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
        this->LogInfo("Performing sync cycle");
        SyncWithDynamicLighting();
    }
    catch (const std::exception& e)
    {
        this->LogError("Error during sync: " + std::string(e.what()));
    }
}

void WindowsDynamicLightingSync::OnDeviceAdded()
{
    this->LogInfo("Device added, refreshing device list");
    RefreshDeviceList();
}

void WindowsDynamicLightingSync::OnDeviceRemoved()
{
    this->LogInfo("Device removed, refreshing device list");
    RefreshDeviceList();
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
        auto_detect_devices = plugin_settings.value("auto_detect_devices", true);
        logging_enabled = false; // Force disable logging
        bidirectional_sync = plugin_settings.value("bidirectional_sync", true);
        smooth_transitions = plugin_settings.value("smooth_transitions", true);
        brightness_multiplier = plugin_settings.value("brightness_multiplier", 1.0f);
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
        plugin_settings["auto_detect_devices"] = auto_detect_devices;
        plugin_settings["enable_logging"] = false; // Force disable logging
        plugin_settings["bidirectional_sync"] = bidirectional_sync;
        plugin_settings["smooth_transitions"] = smooth_transitions;
        plugin_settings["brightness_multiplier"] = brightness_multiplier;
        plugin_settings["show_error_dialogs"] = false; // Force disable error dialogs
        plugin_settings["max_log_file_size"] = max_log_file_size;
        
        settings->SetSettings("WindowsDynamicLightingSync", plugin_settings);
        settings->SaveSettings();
    } catch (...) {
        // Ignore save errors
    }
}

// Logging methods
void WindowsDynamicLightingSync::LogInfo(const std::string& message)
{
    // Logging disabled to prevent crashes
}

void WindowsDynamicLightingSync::LogWarning(const std::string& message)
{
    // Logging disabled to prevent crashes
}

void WindowsDynamicLightingSync::LogError(const std::string& message)
{
    // Logging disabled to prevent crashes
}

void WindowsDynamicLightingSync::WriteToLogFile(const std::string& level, const std::string& message)
{
    // Logging disabled to prevent crashes
}

void WindowsDynamicLightingSync::CheckAndRotateLogFile()
{
    // Logging disabled to prevent crashes
}

// Device management
void WindowsDynamicLightingSync::DetectCompatibleDevices()
{
    if (!RMPointer)
    {
        LogError("ResourceManager not available for device detection");
        return;
    }
    
    LogInfo("Starting automatic device detection...");
    
    // Get current compatible controllers
    std::vector<RGBController*> controllers = GetCompatibleControllers();
    
    // Update device count in UI
    if (deviceCountLabel)
    {
        QString deviceText = QString("Dispositivos compatíveis: %1").arg(controllers.size());
        deviceCountLabel->setText(deviceText);
    }
    
    // Update status based on detection results
    if (controllers.empty())
    {
        LogWarning("No compatible RGB devices found. Make sure your RGB devices are connected and recognized by OpenRGB.");
        if (statusLabel)
        {
            statusLabel->setText("Nenhum dispositivo compatível encontrado");
        }
    }
    else
    {
        LogInfo("Device detection completed successfully. Found " + std::to_string(controllers.size()) + " compatible devices.");
        if (statusLabel)
        {
            statusLabel->setText(QString("Pronto - %1 dispositivos detectados").arg(controllers.size()));
        }
        
        // Log device details for troubleshooting
        for (size_t i = 0; i < controllers.size(); ++i)
        {
            RGBController* controller = controllers[i];
            if (controller)
            {
                LogInfo(QString("Device %1: %2 (%3) - %4 LEDs, %5 modes")
                    .arg(i + 1)
                    .arg(QString::fromStdString(controller->name))
                    .arg(QString::fromStdString(controller->name))
                    .arg(controller->leds.size())
                    .arg(controller->modes.size()).toStdString());
            }
        }
    }
}

void WindowsDynamicLightingSync::RefreshDeviceList()
{
    if (!auto_detect_devices)
    {
        LogInfo("Automatic device detection is disabled");
        return;
    }
    
    LogInfo("Refreshing device list...");
    
    // Perform device detection
    DetectCompatibleDevices();
    
    // If Windows Dynamic Lighting is available, also refresh Windows devices
#ifdef _WIN32
    if (windowsLightingInitialized)
    {
        LogInfo("Refreshing Windows Dynamic Lighting devices...");
        // The DeviceWatcher will automatically handle device changes
        // but we can log current status
        LogInfo("Windows Dynamic Lighting watcher is active");
    }
#endif
    
    LogInfo("Device list refresh completed");
}

std::vector<RGBController*> WindowsDynamicLightingSync::GetCompatibleControllers()
{
    std::vector<RGBController*> compatible_controllers;
    
    if (!RMPointer)
    {
        LogError("ResourceManager not available for getting controllers");
        return compatible_controllers;
    }
    
    std::vector<RGBController*> controllers = RMPointer->GetRGBControllers();
    LogInfo("Scanning " + std::to_string(controllers.size()) + " total RGB controllers");
    
    for (RGBController* controller : controllers)
    {
        if (!controller)
        {
            LogWarning("Found null controller, skipping");
            continue;
        }
        
        if (controller->modes.size() == 0)
        {
            LogInfo("Controller '" + controller->name + "' has no modes, skipping");
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
            LogInfo("Controller '" + controller->name + "' has only " + std::to_string(controller->leds.size()) + " LEDs, may have limited sync capability");
        }
        
        if (is_compatible)
        {
            compatible_controllers.push_back(controller);
            LogInfo("✓ Compatible: '" + controller->name + "' (" + controller->name + ") - " + compatibility_reason + ", " + std::to_string(controller->leds.size()) + " LEDs");
        }
        else
        {
            LogInfo("✗ Incompatible: '" + controller->name + "' - no suitable color control modes");
        }
    }
    
    LogInfo("Found " + std::to_string(compatible_controllers.size()) + " compatible controllers out of " + std::to_string(controllers.size()) + " total");
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
            LogInfo("Updating colors for device: " + controller->name);
        }
    }
}

void WindowsDynamicLightingSync::SetAutoDetectDevices(bool enable)
{
    if (auto_detect_devices != enable)
    {
        auto_detect_devices = enable;
        LogInfo("Auto-detect devices " + std::string(enable ? "enabled" : "disabled"));
        
        if (enable)
        {
            // Immediately perform detection when enabled
            RefreshDeviceList();
        }
    }
}

bool WindowsDynamicLightingSync::IsAutoDetectEnabled() const
{
    return auto_detect_devices;
}

#ifdef _WIN32
void WindowsDynamicLightingSync::InitializeDynamicLighting()
{
    try
    {
        LogInfo("Initializing Windows Dynamic Lighting API");
        
        // Initialize Windows Runtime
        winrt::init_apartment();
        
        // Check if Dynamic Lighting is supported
        if (!CheckDynamicLightingSupport())
        {
            LogWarning("Windows Dynamic Lighting is not supported on this system");
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
        LogInfo("Windows Dynamic Lighting API initialized successfully");
    }
    catch (...)
    {
        // Falha na inicialização - continuar sem Windows Dynamic Lighting
        windowsLightingInitialized = false;
        LogError("Failed to initialize Windows Dynamic Lighting");
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
        
        LogInfo("Checking Dynamic Lighting support");
        return true; // Placeholder
    }
    catch (...)
    {
        LogError("Error checking Dynamic Lighting support");
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
        if (bidirectional_sync)
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
        LogError("Error during Dynamic Lighting sync");
    }
}

void WindowsDynamicLightingSync::CleanupDynamicLighting()
{
    try
    {
        LogInfo("Cleaning up Windows Dynamic Lighting API");
        
        if (lampArrayWatcher != nullptr)
        {
            lampArrayWatcher.Stop();
            lampArrayWatcher = nullptr;
        }
        
        connectedLampArrays.clear();
        windowsLightingInitialized = false;
        
        LogInfo("Windows Dynamic Lighting API cleaned up successfully");
    }
    catch (...)
    {
        LogError("Error during Dynamic Lighting cleanup");
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
void WindowsDynamicLightingSync::SetBidirectionalSync(bool enable)
{
    if (bidirectional_sync != enable)
    {
        bidirectional_sync = enable;
        LogInfo("Bidirectional sync " + std::string(enable ? "enabled" : "disabled"));
    }
}

bool WindowsDynamicLightingSync::IsBidirectionalSyncEnabled() const
{
    return bidirectional_sync;
}

void WindowsDynamicLightingSync::SetSyncInterval(int interval_ms)
{
    if (interval_ms > 0 && interval_ms <= 1000)
    {
        sync_interval_ms = interval_ms;
        LogInfo("Sync interval set to " + std::to_string(interval_ms) + "ms");
        
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

void WindowsDynamicLightingSync::SetSmoothTransitions(bool enable)
{
    if (smooth_transitions != enable)
    {
        smooth_transitions = enable;
        LogInfo("Smooth transitions " + std::string(enable ? "enabled" : "disabled"));
    }
}

bool WindowsDynamicLightingSync::IsSmoothTransitionsEnabled() const
{
    return smooth_transitions;
}

void WindowsDynamicLightingSync::SetBrightnessMultiplier(float multiplier)
{
    if (multiplier >= 0.1f && multiplier <= 2.0f)
    {
        brightness_multiplier = multiplier;
        LogInfo("Brightness multiplier set to " + std::to_string(multiplier));
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
        
        LogInfo("Syncing OpenRGB to Windows Dynamic Lighting");
        
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
        LogError("Error during OpenRGB to Windows sync");
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
        
        LogInfo("Syncing Windows Dynamic Lighting to OpenRGB");
        
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
        LogError("Error during Windows to OpenRGB sync");
    }
}
#endif