#include "WindowsDynamicLightingSync.h"
#include <QHBoxLayout>
#include <QOperatingSystemVersion>
#include <QSysInfo>
#include <vector>
#include <QAction>
#include <QSettings>
#include <QDesktopServices>
#include <QUrl>
#include <QFile>
#include <QDateTime>
#include <QMutexLocker>
#include <QStandardPaths>
#include <QDir>
#include "RGBController.h"
#include <algorithm>
#include "../driver/common/DriverProtocol.h"
#ifdef Q_OS_WIN
#include <windows.h>
#include <roapi.h>
#include <winstring.h>
#endif

// ---------------- Logger implementation (Etapa 1.2.2) ----------------------
QString WDLLogger::logFilePath = QString();
QMutex  WDLLogger::logMutex;

static const char* ToString(WDLLogger::LogLevel lvl)
{
    switch (lvl)
    {
        case WDLLogger::Debug:   return "DEBUG";
        case WDLLogger::Info:    return "INFO";
        case WDLLogger::Warning: return "WARN";
        case WDLLogger::Error:   return "ERROR";
        default: return "INFO";
    }
}

void WDLLogger::SetLogFile(const QString& filepath)
{
    QMutexLocker locker(&logMutex);
    logFilePath = filepath;
}

void WDLLogger::Log(WDLLogger::LogLevel level, const QString& message)
{
    QMutexLocker locker(&logMutex);
    const QString ts = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss.zzz");
    const QString line = QString("[%1] [%2] %3\n").arg(ts, ToString(level), message);

    // Always also print to stdout to preserve current behavior
    fprintf(stdout, "%s", line.toUtf8().constData());

    if (logFilePath.isEmpty())
        return;

    QFile f(logFilePath);
    if (f.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text))
    {
        f.write(line.toUtf8());
        f.close();
    }
}

ResourceManagerInterface* WindowsDynamicLightingSync::RMPointer = nullptr;

OpenRGBPluginInfo WindowsDynamicLightingSync::GetPluginInfo()
{
    WDLLogger::Log(WDLLogger::Debug, "Loading plugin info.");

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
    WDLLogger::Log(WDLLogger::Debug, "Loading plugin API version.");

    return OPENRGB_PLUGIN_API_VERSION;
}

void WindowsDynamicLightingSync::Load(ResourceManagerInterface* resource_manager_ptr)
{
    WDLLogger::Log(WDLLogger::Info, "Loading plugin.");

    // Define arquivo de log em diretório de dados da aplicação
    const QString appDataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (!appDataDir.isEmpty())
    {
        QDir().mkpath(appDataDir);
        const QString logPath = appDataDir + QDir::separator() + "WindowsDynamicLightingSync.log";
        WDLLogger::SetLogFile(logPath);
        WDLLogger::Log(WDLLogger::Info, QString("Log file: %1").arg(logPath));
    }

    // Carregar preferências persistidas
    {
        QSettings s("Oraculo", "OpenRGBWindowsDynamicLightingSyncPlugin");
        syncEnabled               = s.value("enableSync", false).toBool();
        isPluginEnabled           = syncEnabled;
        syncIntervalMs            = s.value("syncIntervalMs", 100).toInt();
        brightnessOverrideEnabled = s.value("brightnessEnabled", false).toBool();
        brightnessOverride        = s.value("brightness", 1.0).toDouble();
        brightnessMultiplier      = brightnessOverride;
        WDLLogger::Log(WDLLogger::Debug, QString("Settings loaded: enable=%1, interval=%2, bright_en=%3, bright=%4")
                        .arg(syncEnabled)
                        .arg(syncIntervalMs)
                        .arg(brightnessOverrideEnabled)
                        .arg(brightnessOverride));
    }

    RMPointer = resource_manager_ptr;

    // Registrar callback para mudanças na lista de dispositivos apenas uma vez
    if (RMPointer && !deviceCallbackRegistered)
    {
        RMPointer->RegisterDeviceListChangeCallback(&WindowsDynamicLightingSync::DeviceListChangedCallback, this);
        deviceCallbackRegistered = true;
        WDLLogger::Log(WDLLogger::Debug, "Registered device list change callback.");
    }

    // Etapa 4 — conectar ao driver via Named Pipe (QLocalSocket)
    if (ConnectToVirtualDriver())
    {
        WDLLogger::Log(WDLLogger::Info, "Connected to driver server.");
    }
    else
    {
        WDLLogger::Log(WDLLogger::Warning, "Failed to connect to driver server.");
    }
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

    // Aplicar valores persistidos nas preferências antes de conectar sinais
    enableSyncCheckbox->setChecked(syncEnabled);
    syncIntervalSpinbox->setValue(syncIntervalMs);
    enableBrightnessCheckbox->setChecked(brightnessOverrideEnabled);
    brightnessSlider->setValue(static_cast<int>(qRound(brightnessOverride * 10.0)));
    brightnessValueLabel->setText(QString::number(brightnessOverride, 'f', 1));
    brightnessContainer->setEnabled(enableBrightnessCheckbox->isChecked());

    // Conectar sinais aos slots (após configurar valores iniciais)
    connect(enableSyncCheckbox, &QCheckBox::toggled, this, &WindowsDynamicLightingSync::onEnableSyncCheckboxToggled);
    connect(syncIntervalSpinbox, QOverload<int>::of(&QSpinBox::valueChanged), this, &WindowsDynamicLightingSync::onSyncIntervalSpinboxValueChanged);
    connect(enableBrightnessCheckbox, &QCheckBox::toggled, this, &WindowsDynamicLightingSync::onEnableBrightnessCheckboxToggled);
    connect(brightnessSlider, &QSlider::valueChanged, this, &WindowsDynamicLightingSync::onBrightnessSliderValueChanged);
    connect(updateButton, &QPushButton::clicked, this, &WindowsDynamicLightingSync::onUpdateButtonClicked);
    connect(reloadButton, &QPushButton::clicked, this, &WindowsDynamicLightingSync::onReloadButtonClicked);

    // Inicializações de sistema e API (via wrappers Etapa 1.2.1)
    initializeSystemInfo();
    InitializeDynamicLighting();

    // Renderizar lista de dispositivos na primeira abertura (wrappers)
    UpdateDeviceList();

    return mainWidget;
}

QMenu* WindowsDynamicLightingSync::GetTrayMenu()
{
    WDLLogger::Log(WDLLogger::Debug, "Creating tray menu.");

    QMenu* menu = new QMenu("Dynamic Lighting Sync");

    // Actions to control sync state from tray
    menu->addAction("Habilitar Sync", [this]() {
        if (enableSyncCheckbox)
        {
            enableSyncCheckbox->setChecked(true);
        }
    });

    menu->addAction("Desabilitar Sync", [this]() {
        if (enableSyncCheckbox)
        {
            enableSyncCheckbox->setChecked(false);
        }
    });

    menu->addSeparator();

    // Refresh device list action
    menu->addAction("Atualizar Dispositivos", [this]() {
        this->refreshDeviceList();
    });

    return menu;
}

void WindowsDynamicLightingSync::Unload()
{
    WDLLogger::Log(WDLLogger::Info, "Unloading plugin (cleanup).");

    // Cancelar registro de callback para evitar dangling pointers
    if (RMPointer && deviceCallbackRegistered)
    {
        RMPointer->UnregisterDeviceListChangeCallback(&WindowsDynamicLightingSync::DeviceListChangedCallback, this);
        deviceCallbackRegistered = false;
        WDLLogger::Log(WDLLogger::Debug, "Unregistered device list change callback.");
    }

    // Etapa 1.2.1 — encerrar conexão com driver virtual (stub)
    DisconnectFromVirtualDriver();
}

WindowsDynamicLightingSync::WindowsDynamicLightingSync() : mainWidget(nullptr)
{
    WDLLogger::Log(WDLLogger::Debug, "Constructor.");
    // Inicializar flags de estado
    isWindowsCompatible = false;
    isLampArrayApiAvailable = false;
    syncTimer = new QTimer(this);
    connect(syncTimer, &QTimer::timeout, this, &WindowsDynamicLightingSync::onSyncTick);
}

WindowsDynamicLightingSync::~WindowsDynamicLightingSync()
{
     WDLLogger::Log(WDLLogger::Debug, "Destructor.");
}

void WindowsDynamicLightingSync::onEnableSyncCheckboxToggled(bool checked)
{
    // Atualiza estado e timer
    syncEnabled = checked;
    isPluginEnabled = checked; // manter espelhado com Etapa 1.2.1
    if (syncEnabled) {
        syncTimer->start(syncIntervalMs);
    } else {
        syncTimer->stop();
    }
    WDLLogger::Log(WDLLogger::Info, QString("Enable Sync: %1").arg(checked));

    // Persistir
    QSettings s("Oraculo", "OpenRGBWindowsDynamicLightingSyncPlugin");
    s.setValue("enableSync", syncEnabled);
}

void WindowsDynamicLightingSync::onSyncIntervalSpinboxValueChanged(int value)
{
    // Ajusta intervalo
    syncIntervalMs = value;
    if (syncEnabled && syncTimer->isActive()) {
        syncTimer->start(syncIntervalMs);
    }
    WDLLogger::Log(WDLLogger::Debug, QString("Sync interval changed: %1 ms").arg(value));

    // Persistir
    QSettings s("Oraculo", "OpenRGBWindowsDynamicLightingSyncPlugin");
    s.setValue("syncIntervalMs", syncIntervalMs);
}

void WindowsDynamicLightingSync::onEnableBrightnessCheckboxToggled(bool checked)
{
    // Atualiza estado e UI
    brightnessOverrideEnabled = checked;
    WDLLogger::Log(WDLLogger::Debug, QString("Brightness override toggled: %1").arg(checked));
    brightnessContainer->setEnabled(checked);

    // Enviar brightness atual ao driver
    {
        float value = checked ? static_cast<float>(brightnessOverride) : 1.0f;
        QByteArray payload;
        QDataStream ds(&payload, QIODevice::WriteOnly);
        ds.setByteOrder(QDataStream::LittleEndian);
        ds << value;
        sendMessage(static_cast<quint16>(DriverProtocol::MessageType::SetBrightness), payload);
    }

    // Persistir
    QSettings s("Oraculo", "OpenRGBWindowsDynamicLightingSyncPlugin");
    s.setValue("brightnessEnabled", brightnessOverrideEnabled);
}

void WindowsDynamicLightingSync::onBrightnessSliderValueChanged(int value)
{
    // Atualiza valor e label
    brightnessOverride = static_cast<double>(value) / 10.0;
    brightnessMultiplier = brightnessOverride; // manter espelhado com Etapa 1.2.1
    brightnessValueLabel->setText(QString::number(brightnessOverride, 'f', 1));
    WDLLogger::Log(WDLLogger::Debug, QString("Brightness override value: %1").arg(brightnessOverride));

    // Enviar novo brilho ao driver (se controle estiver habilitado)
    if (brightnessOverrideEnabled) {
        float fval = static_cast<float>(brightnessOverride);
        QByteArray payload;
        QDataStream ds(&payload, QIODevice::WriteOnly);
        ds.setByteOrder(QDataStream::LittleEndian);
        ds << fval;
        sendMessage(static_cast<quint16>(DriverProtocol::MessageType::SetBrightness), payload);
    }

    // Persistir
    QSettings s("Oraculo", "OpenRGBWindowsDynamicLightingSyncPlugin");
    s.setValue("brightness", brightnessOverride);
}

void WindowsDynamicLightingSync::onSyncTick()
{
    if (!syncEnabled) return;
    if (!RMPointer) return;

    // Só sincroniza se o SO for compatível e a API estiver disponível
    if (!isWindowsCompatible || !isLampArrayApiAvailable) {
        WDLLogger::Log(WDLLogger::Warning, "Sync skipped: OS or API not compatible/available.");
        return;
    }

    // 1) Capturar cor de acentuação do Windows (DWM ColorizationColor)
    QColor accentColorQt(255, 255, 255); // fallback branco
#ifdef Q_OS_WIN
    {
        QSettings settings("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\DWM", QSettings::NativeFormat);
        QVariant v = settings.value("ColorizationColor");
        if (v.isValid()) {
            bool ok = false;
            quint32 argb = v.toUInt(&ok);
            if (ok) {
                // Interpreta como 0xAARRGGBB
                int a = (argb >> 24) & 0xFF;
                int r = (argb >> 16) & 0xFF;
                int g = (argb >> 8)  & 0xFF;
                int b = (argb)       & 0xFF;
                Q_UNUSED(a);
                accentColorQt = QColor(r, g, b);
            }
        }
    }
#endif

    // 2) Aplica brilho (se habilitado)
    double factor = brightnessOverrideEnabled ? std::clamp(brightnessOverride, 0.0, 1.0) : 1.0;
    int r = static_cast<int>(accentColorQt.red()   * factor);
    int g = static_cast<int>(accentColorQt.green() * factor);
    int b = static_cast<int>(accentColorQt.blue()  * factor);

    // 3) Atualiza rótulos de status do efeito
    if (currentEffectLabel)      currentEffectLabel->setText("Efeito atual: Estático (Accent)");
    if (directionEffectLabel)    directionEffectLabel->setText("Direção: N/A");
    if (primaryColorLabel)       primaryColorLabel->setText(QString("Cor Principal: R%1 G%2 B%3").arg(r).arg(g).arg(b));
    if (secondaryColorLabel)     secondaryColorLabel->setText("Cor Secundaria: N/A");

    // 4) Empacota para RGBColor do OpenRGB e aplica em todos controladores
    RGBColor orColor = ToRGBColor(static_cast<unsigned char>(r), static_cast<unsigned char>(g), static_cast<unsigned char>(b));

    std::vector<RGBController*>& controllers = RMPointer->GetRGBControllers();
    for (RGBController* ctrl : controllers)
    {
        if (!ctrl) continue;
        // Define modo custom para garantir controle direto
        ctrl->SetCustomMode();
        // Define todas as LEDs com a cor
        ctrl->SetAllLEDs(orColor);
        // Envia atualização ao dispositivo
        ctrl->UpdateLEDs();
    }

    // 5) Enviar cor ao driver virtual
    {
        QByteArray payload;
        payload.append(static_cast<char>(r & 0xFF));
        payload.append(static_cast<char>(g & 0xFF));
        payload.append(static_cast<char>(b & 0xFF));
        if (!sendMessage(static_cast<quint16>(DriverProtocol::MessageType::SetLedColors), payload)) {
            WDLLogger::Log(WDLLogger::Warning, "Failed to send SetLedColors to driver.");
        }
    }
}

void WindowsDynamicLightingSync::onUpdateButtonClicked()
{
    // Abrir página de releases para atualização
    WDLLogger::Log(WDLLogger::Info, "Update button clicked; opening releases page.");
    QDesktopServices::openUrl(QUrl("https://github.com/Oraculo-sh/OpenRGBWindowsDynamicLightingSyncPlugin/releases"));
}

void WindowsDynamicLightingSync::onReloadButtonClicked()
{
    // Reprocessa detecções e força atualização de UI/dispositivos
    WDLLogger::Log(WDLLogger::Info, "Reload button clicked; re-detecting API and refreshing UI/devices.");

    detectDynamicLightingAPI();
    refreshUiStatus();
    refreshDeviceList();

    // Reinicia timer se necessário
    if (syncEnabled) {
        syncTimer->start(syncIntervalMs);
    }
}

// --- Auxiliares internos ----------------------------------------------------

void WindowsDynamicLightingSync::initializeSystemInfo()
{
    const QOperatingSystemVersion v = QOperatingSystemVersion::current();
    const QString osStr = QString("%1 %2.%3 Build %4")
                              .arg(QSysInfo::prettyProductName())
                              .arg(v.majorVersion())
                              .arg(v.minorVersion())
                              .arg(v.microVersion());
    osInfoLabel->setText("OS: " + osStr);

    // Compatibilidade: Windows 11 22H2+ (build >= 22621) recomendado
#ifdef Q_OS_WIN
    const QString pretty = QSysInfo::prettyProductName();
    // Tentar obter o build a partir do kernelVersion (ex.: "10.0.22631")
    int build = 0;
    const QString kernel = QSysInfo::kernelVersion();
    const QStringList parts = kernel.split('.');
    if (parts.size() >= 3) {
        bool ok = false;
        int b = parts.at(2).toInt(&ok);
        if (ok) build = b;
    }
    isWindowsCompatible = (pretty.contains("Windows 11", Qt::CaseInsensitive) && build >= 22621);
#else
    isWindowsCompatible = false;
#endif
    WDLLogger::Log(WDLLogger::Debug, QString("OS compatible: %1").arg(isWindowsCompatible));
}

void WindowsDynamicLightingSync::detectDynamicLightingAPI()
{
    // Detecção real da API WinRT LampArray sem criar novas classes
#ifdef Q_OS_WIN
    bool available = false;
    HRESULT hrInit = RoInitialize(RO_INIT_MULTITHREADED);
    // Inicialização pode retornar RO_E_WRONG_THREAD ou S_FALSE; seguimos adiante se não for falha crítica
    const wchar_t* lampArrayClass = L"Windows.Devices.Lights.LampArray";
    HSTRING hstr = nullptr;
    if (SUCCEEDED(WindowsCreateString(lampArrayClass, static_cast<UINT32>(wcslen(lampArrayClass)), &hstr))) {
        IInspectable* pFactory = nullptr;
        HRESULT hr = RoGetActivationFactory(hstr, IID_IInspectable, reinterpret_cast<void**>(&pFactory));
        if (SUCCEEDED(hr) && pFactory) {
            available = true;
            pFactory->Release();
        }
        WindowsDeleteString(hstr);
    }
    if (SUCCEEDED(hrInit)) {
        RoUninitialize();
    }
    isLampArrayApiAvailable = available;
#else
    isLampArrayApiAvailable = false;
#endif
    WDLLogger::Log(WDLLogger::Info, QString("LampArray API available: %1").arg(isLampArrayApiAvailable));
}

void WindowsDynamicLightingSync::refreshUiStatus()
{
    apiStatusLabel->setText(QString("API Windows Dynamic Lighting: ") + (isLampArrayApiAvailable ? "Disponível" : "Indisponível"));
    systemStatusLabel->setText(QString("Status Windows Dynamic Lighting: ") + (isWindowsCompatible ? "Compatível" : "Não compatível"));
    if (compatibilityLabel)
    {
        compatibilityLabel->setText(QString("Compatibilidade: ") + (isWindowsCompatible ? "Compatível" : "Não compatível"));
    }
    WDLLogger::Log(WDLLogger::Debug, "UI status refreshed.");
}

void WindowsDynamicLightingSync::refreshDeviceList()
{
    if (!deviceCountLabel || !deviceListLayout)
    {
        return;
    }

    // Limpar lista anterior
    QLayoutItem* child;
    while ((child = deviceListLayout->takeAt(0)) != nullptr)
    {
        if (child->widget())
        {
            child->widget()->deleteLater();
        }
        delete child;
    }

    int count = 0;
    if (RMPointer)
    {
        // Obter referência ao vetor de controladores
        std::vector<RGBController*>& controllers = RMPointer->GetRGBControllers();
        count = static_cast<int>(controllers.size());

        // Renderizar até um máximo razoável para não poluir UI
        const int maxRender = 50;
        int rendered = 0;
        for (RGBController* ctrl : controllers)
        {
            if (!ctrl) continue;
            // Calcular quantidade total de LEDs somando zonas
            unsigned int total_leds = 0;
            for (std::size_t zi = 0; zi < ctrl->zones.size(); ++zi)
            {
                total_leds += ctrl->GetLEDsInZone(static_cast<unsigned int>(zi));
            }

            QString labelText = QString("%1 — LEDs: %2")
                                    .arg(QString::fromStdString(ctrl->name))
                                    .arg(total_leds);
            QLabel* lbl = new QLabel(labelText);
            deviceListLayout->addWidget(lbl);
            if (++rendered >= maxRender) break;
        }
    }

    deviceCountLabel->setText(QString("Dispositivos detectados: %1").arg(count));
    WDLLogger::Log(WDLLogger::Debug, QString("Device list refreshed. Count=%1").arg(count));
}

void WindowsDynamicLightingSync::DeviceListChangedCallback(void* arg)
{
    WindowsDynamicLightingSync* self = reinterpret_cast<WindowsDynamicLightingSync*>(arg);
    if (!self) return;

    // Esta callback pode ser chamada de thread diferente; para segurança, enviar ao thread da UI
    if (self->mainWidget)
    {
        QMetaObject::invokeMethod(self->mainWidget, [self]() {
            self->refreshDeviceList();
        });
    }
    WDLLogger::Log(WDLLogger::Debug, "Device list change callback invoked.");
}

// ---------------- Etapa 1.2.1 — Wrappers/Invólucros -------------------------

bool WindowsDynamicLightingSync::InitializeDynamicLighting()
{
    initializeSystemInfo();
    detectDynamicLightingAPI();
    CheckDynamicLightingAvailability();
    refreshUiStatus();
    return true;
}

void WindowsDynamicLightingSync::CheckDynamicLightingAvailability()
{
    // Espelha estado em membro interno
    isDynamicLightingAvailable = isLampArrayApiAvailable;
}

void WindowsDynamicLightingSync::UpdateDeviceList()
{
    refreshDeviceList();
}

bool WindowsDynamicLightingSync::ConnectToVirtualDriver()
{
    // Conecta ao servidor do driver via QLocalSocket
    if (!m_driverSocket)
    {
        m_driverSocket.reset(new QLocalSocket(this));
        connect(m_driverSocket.data(), &QLocalSocket::readyRead, this, [this]{
            m_rxBuffer += m_driverSocket->readAll();
            // No momento apenas consome/descarta; futura lógica de protocolo pode ser adicionada aqui
            m_rxBuffer.clear();
        });
        connect(m_driverSocket.data(), &QLocalSocket::disconnected, this, [this]{
            WDLLogger::Log(WDLLogger::Warning, "Driver socket disconnected.");
        });
        connect(m_driverSocket.data(), QOverload<QLocalSocket::LocalSocketError>::of(&QLocalSocket::errorOccurred), this, [this](QLocalSocket::LocalSocketError code){
            WDLLogger::Log(WDLLogger::Error, QString("Driver socket error: %1").arg(static_cast<int>(code)));
        });
    }

    if (m_driverSocket->state() == QLocalSocket::ConnectedState)
        return true;

    m_driverSocket->connectToServer(m_driverServerName);
    if (!m_driverSocket->waitForConnected(500))
    {
        WDLLogger::Log(WDLLogger::Warning, QString("Failed to connect to driver server '%1'.").arg(m_driverServerName));
        return false;
    }

    WDLLogger::Log(WDLLogger::Info, QString("Connected to driver server '%1'.").arg(m_driverServerName));
    return true;
}

void WindowsDynamicLightingSync::DisconnectFromVirtualDriver()
{
    if (m_driverSocket)
    {
        if (m_driverSocket->state() == QLocalSocket::ConnectedState)
        {
            m_driverSocket->disconnectFromServer();
            m_driverSocket->waitForDisconnected(200);
        }
        m_driverSocket.reset();
    }
}

bool WindowsDynamicLightingSync::sendMessage(quint16 type, const QByteArray& payload)
{
    // Ensure socket is connected
    if (!m_driverSocket || m_driverSocket->state() != QLocalSocket::ConnectedState)
    {
        if (!ConnectToVirtualDriver())
        {
            WDLLogger::Log(WDLLogger::Error, "sendMessage: driver connection not available.");
            return false;
        }
    }

    const QByteArray packet = DriverProtocol::pack(static_cast<DriverProtocol::MessageType>(type), payload);
    const qint64 written = m_driverSocket->write(packet);
    if (written < 0)
    {
        WDLLogger::Log(WDLLogger::Error, QString("sendMessage: write failed (%1)").arg(m_driverSocket->errorString()));
        return false;
    }
    if (written != packet.size())
    {
        WDLLogger::Log(WDLLogger::Warning, QString("sendMessage: partial write (%1/%2 bytes)").arg(written).arg(packet.size()));
    }

    m_driverSocket->flush();
    m_driverSocket->waitForBytesWritten(20);
    return written > 0;
}

