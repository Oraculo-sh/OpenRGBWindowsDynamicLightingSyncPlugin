#include "WindowsDynamicLightingSync.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QMenu>
#include <QAction>
#include <QMessageBox>

static const char* copyright_string = "Copyright © 2024 WindowsDynamicLightingSync developers";

WindowsDynamicLightingSync::WindowsDynamicLightingSync()
    : widget(nullptr)
    , statusLabel(nullptr)
    , toggleButton(nullptr)
    , enableCheckBox(nullptr)
    , deviceCountLabel(nullptr)
    , syncTimer(nullptr)
    , RMPointer(nullptr)
    , isDynamicLightingEnabled(false)
    , darkTheme(false)
    , testColor(0x00FF6600)
#ifdef _WIN32
    , lampArrayWatcher(nullptr)
    , lampArrays(nullptr)
    , windowsLightingInitialized(false)
#endif
{
}

WindowsDynamicLightingSync::~WindowsDynamicLightingSync()
{
    if (syncTimer)
    {
        syncTimer->stop();
        delete syncTimer;
    }
    
#ifdef _WIN32
    cleanupWindowsLighting();
#endif
}

OpenRGBPluginInfo WindowsDynamicLightingSync::GetPluginInfo()
{
    OpenRGBPluginInfo info;
    
    info.Name           = "Windows Dynamic Lighting Sync";
    info.Description    = "Sincroniza iluminação RGB com Windows Dynamic Lighting";
    info.Version        = "1.0.0";
    info.Commit         = "";
    info.URL            = "https://github.com/user/WindowsDynamicLightingSync";
    info.Icon.load(":/WindowsDynamicLightingSync.png");
    info.Location       = OPENRGB_PLUGIN_LOCATION_TOP;
    info.Label          = "Windows Dynamic Lighting";
    
    return info;
}

unsigned int WindowsDynamicLightingSync::GetPluginAPIVersion()
{
    return OPENRGB_PLUGIN_API_VERSION;
}

void WindowsDynamicLightingSync::Load(ResourceManagerInterface* resource_manager_ptr)
{
    RMPointer = resource_manager_ptr;
    darkTheme = false; // Valor padrão
    
    // Inicializar timer de sincronização
    syncTimer = new QTimer(this);
    connect(syncTimer, &QTimer::timeout, this, &WindowsDynamicLightingSync::syncLighting);
    
    // Inicializar Windows Dynamic Lighting API
#ifdef _WIN32
    initializeWindowsLighting();
#endif
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
    
    controlLayout->addWidget(enableCheckBox);
    controlLayout->addWidget(toggleButton);
    controlLayout->addWidget(statusLabel);
    controlLayout->addWidget(deviceCountLabel);
    
    mainLayout->addWidget(controlGroup);
    
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
    if (!RMPointer)
    {
        deviceCountLabel->setText("Dispositivos detectados: 0 (ResourceManager não disponível)");
        return;
    }
    
    std::vector<RGBController*> controllers = RMPointer->GetRGBControllers();
    deviceCountLabel->setText(QString("Dispositivos detectados: %1").arg(controllers.size()));
}

void WindowsDynamicLightingSync::syncLighting()
{
    if (!isDynamicLightingEnabled || !RMPointer)
    {
        return;
    }
    
    // Sincronizar com Windows Dynamic Lighting se disponível
#ifdef _WIN32
    if (windowsLightingInitialized && !connectedLampArrays.empty())
    {
        syncWithWindowsLighting();
    }
    else
#endif
    {
        // Fallback: aplicar uma cor simples para teste
        applyLightingToAllDevices();
    }
}

void WindowsDynamicLightingSync::applyLightingToAllDevices()
{
    if (!RMPointer)
    {
        return;
    }
    
    std::vector<RGBController*> controllers = RMPointer->GetRGBControllers();
    
    for (RGBController* controller : controllers)
    {
        if (controller)
        {
            // Definir modo customizado
            controller->SetCustomMode();
            
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
    if (syncTimer && syncTimer->isActive())
    {
        syncTimer->stop();
    }
    
    cleanupWindowsLighting();
    
    if (widget)
    {
        delete widget;
        widget = nullptr;
    }
}

#ifdef _WIN32
void WindowsDynamicLightingSync::initializeWindowsLighting()
{
    try
    {
        // Inicializar Windows Runtime
        init_apartment();
        
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
    }
    catch (...)
    {
        // Falha na inicialização - continuar sem Windows Dynamic Lighting
        windowsLightingInitialized = false;
    }
}

void WindowsDynamicLightingSync::cleanupWindowsLighting()
{
    try
    {
        if (lampArrayWatcher != nullptr)
        {
            lampArrayWatcher.Stop();
            lampArrayWatcher = nullptr;
        }
        
        connectedLampArrays.clear();
        windowsLightingInitialized = false;
    }
    catch (...)
    {
        // Ignorar erros durante limpeza
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
                    
                    // Aplicar cores ao LampArray
                    auto colorArray = winrt::single_threaded_vector<Windows::UI::Color>(std::move(lampColors));
                    lampArray.SetColorsForIndices(colorArray.GetView(), nullptr);
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
#endif