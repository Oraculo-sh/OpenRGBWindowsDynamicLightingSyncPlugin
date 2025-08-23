#ifndef WINDOWSDYNAMICLIGHTINGSYNCWIDGET_H
#define WINDOWSDYNAMICLIGHTINGSYNCWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QCheckBox>
#include <QComboBox>
#include <QLabel>
#include <QPushButton>
#include <QSlider>
#include <QSpinBox>
#include <QColorDialog>
#include <QListWidget>
#include <QListWidgetItem>
#include <QProgressBar>
#include <QTimer>
#include <QSettings>
#include <QMap>
#include "RGBController.h"

class WindowsDynamicLightingSync;
class RGBController;

struct DeviceSettings
{
    bool enabled;
    bool gradient_mode;
    bool anti_bug_mode;
    unsigned int secondary_color;
    bool auto_secondary;
};

class WindowsDynamicLightingSyncWidget : public QWidget
{
    Q_OBJECT

public:
    explicit WindowsDynamicLightingSyncWidget(QWidget *parent = nullptr, WindowsDynamicLightingSync* plugin = nullptr);
    ~WindowsDynamicLightingSyncWidget();
    
    void ToggleSync();
    void OnDynamicLightingColorChanged(unsigned int color);
    void RefreshDeviceList();

public slots:
    void OnSyncToggled(bool enabled);
    void OnModeChanged();
    void OnDeviceSelectionChanged();
    void OnSecondaryColorChanged();
    void OnAutoSecondaryToggled(bool enabled);
    void OnAntiBugToggled(bool enabled);
    void OnUpdateTimer();

private slots:
    void ShowColorPicker();
    void OnDeviceItemChanged(QListWidgetItem* item);

private:
    WindowsDynamicLightingSync* plugin_instance;
    QSettings* settings;
    
    // Main layout
    QVBoxLayout* main_layout;
    
    // Control groups
    QGroupBox* sync_group;
    QGroupBox* mode_group;
    QGroupBox* devices_group;
    QGroupBox* color_group;
    QGroupBox* status_group;
    
    // Sync controls
    QCheckBox* sync_enabled_checkbox;
    QLabel* sync_status_label;
    
    // Mode controls
    QComboBox* mode_combo;
    QLabel* mode_description_label;
    
    // Device controls
    QListWidget* device_list;
    QPushButton* refresh_devices_button;
    QPushButton* select_all_button;
    QPushButton* select_none_button;
    
    // Color controls
    QLabel* primary_color_label;
    QLabel* primary_color_display;
    QLabel* secondary_color_label;
    QLabel* secondary_color_display;
    QPushButton* secondary_color_button;
    QCheckBox* auto_secondary_checkbox;
    
    // Status controls
    QLabel* connected_devices_label;
    QLabel* active_devices_label;
    QProgressBar* sync_quality_bar;
    QLabel* last_update_label;
    
    // Internal state
    unsigned int current_primary_color;
    unsigned int current_secondary_color;
    bool sync_active;
    QTimer* update_timer;
    QMap<int, DeviceSettings> device_settings;
    
    // Helper methods
    void SetupUI();
    void SetupSyncGroup();
    void SetupModeGroup();
    void SetupDevicesGroup();
    void SetupColorGroup();
    void SetupStatusGroup();
    void ConnectSignals();
    void LoadSettings();
    void SaveSettings();
    void UpdateDeviceList();
    void UpdateColorDisplays();
    void UpdateStatusInfo();
    void ApplyColorsToDevices();
    void ApplyStaticMode(RGBController* controller, unsigned int color);
    void ApplyGradientMode(RGBController* controller, unsigned int primary_color, unsigned int secondary_color);
    unsigned int GenerateHarmoniousSecondaryColor(unsigned int primary_color);
    unsigned int InterpolateColor(unsigned int color1, unsigned int color2, float ratio);
    QString ColorToHex(unsigned int color);
    void SetColorDisplay(QLabel* label, unsigned int color);
    
    // Color generation helpers
    void RGBtoHSV(unsigned int rgb, float& h, float& s, float& v);
    unsigned int HSVtoRGB(float h, float s, float v);
    
    // Helper function to convert unsigned int to RGBColor
    RGBColor ConvertToRGBColor(unsigned int color);
};

#endif // WINDOWSDYNAMICLIGHTINGSYNCWIDGET_H