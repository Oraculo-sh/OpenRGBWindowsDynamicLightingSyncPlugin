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
#include <QMenu>
#include <QTimer>
#include <QMutex>
#include <QLocalSocket>
#include <QScopedPointer>

// Logger simples conforme Etapa 1.2.2 do roadmap
class WDLLogger {
public:
    enum LogLevel { Debug, Info, Warning, Error };
    static void Log(LogLevel level, const QString& message);
    static void SetLogFile(const QString& filepath);
private:
    static QString logFilePath;
    static QMutex  logMutex;
};

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

    // Estado interno
    bool isWindowsCompatible = false;     // Compatível com Dynamic Lighting (Windows 11 22H2+)
    bool isLampArrayApiAvailable = false; // Presença de API WinRT LampArray
    bool deviceCallbackRegistered = false; // Callback de mudança na lista de dispositivos registrado
    bool syncEnabled = false;             // Estado do toggle de sincronização

    // Etapa 1.2.1 — novos membros para gerenciamento WDL
    bool   isDynamicLightingAvailable = false; // espelha isLampArrayApiAvailable
    bool   isPluginEnabled = false;            // espelha syncEnabled
    int    syncIntervalMs = 100;               // já existente
    double brightnessMultiplier = 1.0;         // espelha brightnessOverride

    // Temporização e ajustes de brilho
    QTimer* syncTimer = nullptr;
    bool    brightnessOverrideEnabled = false;
    double  brightnessOverride = 1.0; // 0.0 - 1.0

    // Métodos auxiliares
    void initializeSystemInfo();
    void detectDynamicLightingAPI();
    void refreshUiStatus();
    void refreshDeviceList(); // Atualiza contagem e lista de dispositivos
    static void DeviceListChangedCallback(void* arg); // Callback estático para mudanças de lista

    // Etapa 1.2.1 — novos métodos (invólucros internos)
    bool InitializeDynamicLighting();
    void CheckDynamicLightingAvailability();
    void UpdateDeviceList();
    bool ConnectToVirtualDriver();
    void DisconnectFromVirtualDriver();

    // IPC Client (Named Pipe via QLocalSocket)
    QScopedPointer<QLocalSocket> m_driverSocket;
    QByteArray m_rxBuffer;
    QString m_driverServerName = QStringLiteral("OpenRGB_WDL_Driver");

    bool sendMessage(quint16 type, const QByteArray& payload);

private slots:
    void onEnableSyncCheckboxToggled(bool checked);
    void onSyncIntervalSpinboxValueChanged(int value);
    void onEnableBrightnessCheckboxToggled(bool checked);
    void onBrightnessSliderValueChanged(int value);
    void onUpdateButtonClicked();
    void onReloadButtonClicked();

    void onSyncTick();
};

#endif // WINDOWSDYNAMICLIGHTINGSYNC_H
