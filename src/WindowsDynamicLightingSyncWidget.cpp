#include "WindowsDynamicLightingSyncWidget.h"
#include "WindowsDynamicLightingSync.h"
#include "ResourceManagerInterface.h"
#include "RGBController.h"
#include <QApplication>
#include <QMessageBox>
#include <QDateTime>
#include <QDebug>
#include <cmath>

WindowsDynamicLightingSyncWidget::WindowsDynamicLightingSyncWidget(QWidget *parent, WindowsDynamicLightingSync* plugin)
    : QWidget(parent)
    , plugin_instance(plugin)
    , settings(nullptr)
    , main_layout(nullptr)
    , current_primary_color(0x0080FF)
    , current_secondary_color(0xFF8000)
    , sync_active(false)
    , update_timer(nullptr)
{
    setWindowTitle("Windows Dynamic Lighting Sync");
    setMinimumSize(600, 500);
    
    SetupUI();
    ConnectSignals();
    LoadSettings();
    
    // Setup update timer
    update_timer = new QTimer(this);
    connect(update_timer, &QTimer::timeout, this, &WindowsDynamicLightingSyncWidget::OnUpdateTimer);
    update_timer->start(1000); // Update every second
    
    RefreshDeviceList();
    UpdateColorDisplays();
    UpdateStatusInfo();
}

WindowsDynamicLightingSyncWidget::~WindowsDynamicLightingSyncWidget()
{
    SaveSettings();
}

void WindowsDynamicLightingSyncWidget::SetupUI()
{
    main_layout = new QVBoxLayout(this);
    main_layout->setSpacing(10);
    main_layout->setContentsMargins(10, 10, 10, 10);
    
    SetupSyncGroup();
    SetupModeGroup();
    SetupDevicesGroup();
    SetupColorGroup();
    SetupStatusGroup();
    
    main_layout->addStretch();
}

void WindowsDynamicLightingSyncWidget::SetupSyncGroup()
{
    sync_group = new QGroupBox("Controle de Sincronização", this);
    QVBoxLayout* layout = new QVBoxLayout(sync_group);
    
    sync_enabled_checkbox = new QCheckBox("Ativar sincronização com Iluminação Dinâmica do Windows", this);
    sync_enabled_checkbox->setChecked(false);
    
    sync_status_label = new QLabel("Status: Desativado", this);
    sync_status_label->setStyleSheet("color: red; font-weight: bold;");
    
    layout->addWidget(sync_enabled_checkbox);
    layout->addWidget(sync_status_label);
    
    main_layout->addWidget(sync_group);
}

void WindowsDynamicLightingSyncWidget::SetupModeGroup()
{
    mode_group = new QGroupBox("Modo de Operação", this);
    QVBoxLayout* layout = new QVBoxLayout(mode_group);
    
    mode_combo = new QComboBox(this);
    mode_combo->addItem("Modo Padrão - Cor única (Static)", 0);
    mode_combo->addItem("Modo Gradiente - Duas cores (Direct)", 1);
    
    mode_description_label = new QLabel(
        "Modo Padrão: Aplica a cor principal da Iluminação Dinâmica usando o modo Static do OpenRGB.", 
        this
    );
    mode_description_label->setWordWrap(true);
    mode_description_label->setStyleSheet("color: gray; font-style: italic;");
    
    layout->addWidget(new QLabel("Selecione o modo de operação:", this));
    layout->addWidget(mode_combo);
    layout->addWidget(mode_description_label);
    
    main_layout->addWidget(mode_group);
}

void WindowsDynamicLightingSyncWidget::SetupDevicesGroup()
{
    devices_group = new QGroupBox("Seleção de Dispositivos", this);
    QVBoxLayout* layout = new QVBoxLayout(devices_group);
    
    // Control buttons
    QHBoxLayout* button_layout = new QHBoxLayout();
    refresh_devices_button = new QPushButton("Atualizar Lista", this);
    select_all_button = new QPushButton("Selecionar Todos", this);
    select_none_button = new QPushButton("Desmarcar Todos", this);
    
    button_layout->addWidget(refresh_devices_button);
    button_layout->addWidget(select_all_button);
    button_layout->addWidget(select_none_button);
    button_layout->addStretch();
    
    // Device list
    device_list = new QListWidget(this);
    device_list->setMaximumHeight(150);
    
    layout->addLayout(button_layout);
    layout->addWidget(new QLabel("Dispositivos disponíveis:", this));
    layout->addWidget(device_list);
    
    main_layout->addWidget(devices_group);
}

void WindowsDynamicLightingSyncWidget::SetupColorGroup()
{
    color_group = new QGroupBox("Controle de Cores", this);
    QGridLayout* layout = new QGridLayout(color_group);
    
    // Primary color
    primary_color_label = new QLabel("Cor Principal (Windows):", this);
    primary_color_display = new QLabel(this);
    primary_color_display->setMinimumSize(50, 30);
    primary_color_display->setStyleSheet("border: 1px solid black;");
    
    // Secondary color
    secondary_color_label = new QLabel("Cor Secundária (Gradiente):", this);
    secondary_color_display = new QLabel(this);
    secondary_color_display->setMinimumSize(50, 30);
    secondary_color_display->setStyleSheet("border: 1px solid black;");
    
    secondary_color_button = new QPushButton("Escolher Cor", this);
    auto_secondary_checkbox = new QCheckBox("Gerar automaticamente", this);
    auto_secondary_checkbox->setChecked(true);
    
    layout->addWidget(primary_color_label, 0, 0);
    layout->addWidget(primary_color_display, 0, 1);
    layout->addWidget(secondary_color_label, 1, 0);
    layout->addWidget(secondary_color_display, 1, 1);
    layout->addWidget(secondary_color_button, 1, 2);
    layout->addWidget(auto_secondary_checkbox, 2, 0, 1, 3);
    
    main_layout->addWidget(color_group);
}

void WindowsDynamicLightingSyncWidget::SetupStatusGroup()
{
    status_group = new QGroupBox("Status do Sistema", this);
    QGridLayout* layout = new QGridLayout(status_group);
    
    connected_devices_label = new QLabel("Dispositivos conectados: 0", this);
    active_devices_label = new QLabel("Dispositivos ativos: 0", this);
    
    sync_quality_bar = new QProgressBar(this);
    sync_quality_bar->setRange(0, 100);
    sync_quality_bar->setValue(0);
    
    last_update_label = new QLabel("Última atualização: Nunca", this);
    
    layout->addWidget(connected_devices_label, 0, 0);
    layout->addWidget(active_devices_label, 0, 1);
    layout->addWidget(new QLabel("Qualidade da sincronização:", this), 1, 0);
    layout->addWidget(sync_quality_bar, 1, 1);
    layout->addWidget(last_update_label, 2, 0, 1, 2);
    
    main_layout->addWidget(status_group);
}

void WindowsDynamicLightingSyncWidget::ConnectSignals()
{
    connect(sync_enabled_checkbox, &QCheckBox::toggled, this, &WindowsDynamicLightingSyncWidget::OnSyncToggled);
    connect(mode_combo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &WindowsDynamicLightingSyncWidget::OnModeChanged);
    connect(refresh_devices_button, &QPushButton::clicked, this, &WindowsDynamicLightingSyncWidget::RefreshDeviceList);
    connect(select_all_button, &QPushButton::clicked, [this]() {
        for (int i = 0; i < device_list->count(); ++i) {
            QListWidgetItem* item = device_list->item(i);
            item->setCheckState(Qt::Checked);
        }
    });
    connect(select_none_button, &QPushButton::clicked, [this]() {
        for (int i = 0; i < device_list->count(); ++i) {
            QListWidgetItem* item = device_list->item(i);
            item->setCheckState(Qt::Unchecked);
        }
    });
    connect(secondary_color_button, &QPushButton::clicked, this, &WindowsDynamicLightingSyncWidget::ShowColorPicker);
    connect(auto_secondary_checkbox, &QCheckBox::toggled, this, &WindowsDynamicLightingSyncWidget::OnAutoSecondaryToggled);
    connect(device_list, &QListWidget::itemChanged, this, &WindowsDynamicLightingSyncWidget::OnDeviceItemChanged);
}

void WindowsDynamicLightingSyncWidget::ToggleSync()
{
    sync_enabled_checkbox->setChecked(!sync_enabled_checkbox->isChecked());
}

void WindowsDynamicLightingSyncWidget::OnSyncToggled(bool enabled)
{
    sync_active = enabled;
    
    if (enabled) {
        sync_status_label->setText("Status: Ativo");
        sync_status_label->setStyleSheet("color: green; font-weight: bold;");
        
        // Start monitoring
        if (plugin_instance) {
            // Plugin will handle starting the monitoring
        }
    } else {
        sync_status_label->setText("Status: Desativado");
        sync_status_label->setStyleSheet("color: red; font-weight: bold;");
        
        // Stop monitoring
        if (plugin_instance) {
            // Plugin will handle stopping the monitoring
        }
    }
    
    UpdateStatusInfo();
    SaveSettings();
}

void WindowsDynamicLightingSyncWidget::OnModeChanged()
{
    int mode = mode_combo->currentData().toInt();
    
    if (mode == 0) { // Standard mode
        mode_description_label->setText(
            "Modo Padrão: Aplica a cor principal da Iluminação Dinâmica usando o modo Static do OpenRGB."
        );
        color_group->setEnabled(true);
        secondary_color_label->setEnabled(false);
        secondary_color_display->setEnabled(false);
        secondary_color_button->setEnabled(false);
        auto_secondary_checkbox->setEnabled(false);
    } else { // Gradient mode
        mode_description_label->setText(
            "Modo Gradiente: Combina a cor principal com uma cor secundária usando o modo Direct do OpenRGB para criar gradientes."
        );
        color_group->setEnabled(true);
        secondary_color_label->setEnabled(true);
        secondary_color_display->setEnabled(true);
        secondary_color_button->setEnabled(!auto_secondary_checkbox->isChecked());
        auto_secondary_checkbox->setEnabled(true);
    }
    
    SaveSettings();
}

void WindowsDynamicLightingSyncWidget::OnDynamicLightingColorChanged(unsigned int color)
{
    current_primary_color = color;
    
    if (auto_secondary_checkbox->isChecked()) {
        current_secondary_color = GenerateHarmoniousSecondaryColor(color);
    }
    
    UpdateColorDisplays();
    
    if (sync_active) {
        ApplyColorsToDevices();
    }
    
    last_update_label->setText("Última atualização: " + QDateTime::currentDateTime().toString("hh:mm:ss"));
}

void WindowsDynamicLightingSyncWidget::RefreshDeviceList()
{
    UpdateDeviceList();
    UpdateStatusInfo();
}

void WindowsDynamicLightingSyncWidget::UpdateDeviceList()
{
    device_list->clear();
    
    if (!WindowsDynamicLightingSync::RMPointer) {
        return;
    }
    
    auto controllers = WindowsDynamicLightingSync::RMPointer->GetRGBControllers();
    
    for (size_t i = 0; i < controllers.size(); ++i) {
        RGBController* controller = controllers[i];
        if (!controller) continue;
        
        QString device_name = QString::fromStdString(controller->name);
        QString device_info = QString("%1 (%2 LEDs)").arg(device_name).arg(controller->leds.size());
        
        QListWidgetItem* item = new QListWidgetItem(device_info, device_list);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(Qt::Unchecked);
        item->setData(Qt::UserRole, static_cast<int>(i));
        
        // Add anti-bug mode checkbox
        QWidget* widget = new QWidget();
        QHBoxLayout* layout = new QHBoxLayout(widget);
        layout->setContentsMargins(0, 0, 0, 0);
        
        QCheckBox* anti_bug_checkbox = new QCheckBox("Modo Anti-Bug", widget);
        anti_bug_checkbox->setToolTip("Força o uso do modo Static mesmo no modo Gradiente");
        
        layout->addStretch();
        layout->addWidget(anti_bug_checkbox);
        
        device_list->addItem(item);
        device_list->setItemWidget(item, widget);
        
        // Connect anti-bug checkbox
        connect(anti_bug_checkbox, &QCheckBox::toggled, [this, i](bool enabled) {
            device_settings[i].anti_bug_mode = enabled;
            SaveSettings();
        });
    }
}

void WindowsDynamicLightingSyncWidget::UpdateColorDisplays()
{
    SetColorDisplay(primary_color_display, current_primary_color);
    SetColorDisplay(secondary_color_display, current_secondary_color);
}

void WindowsDynamicLightingSyncWidget::UpdateStatusInfo()
{
    if (!WindowsDynamicLightingSync::RMPointer) {
        connected_devices_label->setText("Dispositivos conectados: 0");
        active_devices_label->setText("Dispositivos ativos: 0");
        sync_quality_bar->setValue(0);
        return;
    }
    
    auto controllers = WindowsDynamicLightingSync::RMPointer->GetRGBControllers();
    int total_devices = controllers.size();
    int active_devices = 0;
    
    for (int i = 0; i < device_list->count(); ++i) {
        QListWidgetItem* item = device_list->item(i);
        if (item && item->checkState() == Qt::Checked) {
            active_devices++;
        }
    }
    
    connected_devices_label->setText(QString("Dispositivos conectados: %1").arg(total_devices));
    active_devices_label->setText(QString("Dispositivos ativos: %1").arg(active_devices));
    
    int quality = sync_active ? (total_devices > 0 ? (active_devices * 100 / total_devices) : 0) : 0;
    sync_quality_bar->setValue(quality);
}

void WindowsDynamicLightingSyncWidget::ApplyColorsToDevices()
{
    if (!WindowsDynamicLightingSync::RMPointer || !sync_active) {
        return;
    }
    
    auto controllers = WindowsDynamicLightingSync::RMPointer->GetRGBControllers();
    int mode = mode_combo->currentData().toInt();
    
    for (int i = 0; i < device_list->count(); ++i) {
        QListWidgetItem* item = device_list->item(i);
        if (!item || item->checkState() != Qt::Checked) {
            continue;
        }
        
        int controller_index = item->data(Qt::UserRole).toInt();
        if (controller_index >= 0 && controller_index < static_cast<int>(controllers.size())) {
            RGBController* controller = controllers[controller_index];
            
            bool anti_bug_mode = device_settings.contains(controller_index) ? 
                                device_settings[controller_index].anti_bug_mode : false;
            
            if (mode == 0 || anti_bug_mode) { // Standard mode or anti-bug mode
                ApplyStaticMode(controller, current_primary_color);
            } else { // Gradient mode
                ApplyGradientMode(controller, current_primary_color, current_secondary_color);
            }
        }
    }
}

void WindowsDynamicLightingSyncWidget::ApplyStaticMode(RGBController* controller, unsigned int color)
{
    if (!controller) return;
    
    // Set to static mode
    for (size_t i = 0; i < controller->modes.size(); ++i) {
        if (controller->modes[i].name == "Static" || controller->modes[i].name == "Direct") {
            controller->SetMode(i);
            break;
        }
    }
    
    // Apply color to all LEDs
    RGBColor rgb_color = ConvertToRGBColor(color);
    for (size_t i = 0; i < controller->leds.size(); ++i) {
        controller->colors[i] = rgb_color;
    }
    
    controller->UpdateLEDs();
}

void WindowsDynamicLightingSyncWidget::ApplyGradientMode(RGBController* controller, unsigned int primary_color, unsigned int secondary_color)
{
    if (!controller) return;
    
    // Set to direct mode
    for (size_t i = 0; i < controller->modes.size(); ++i) {
        if (controller->modes[i].name == "Direct") {
            controller->SetMode(i);
            break;
        }
    }
    
    // Apply gradient
    size_t led_count = controller->leds.size();
    if (led_count == 0) return;
    
    for (size_t i = 0; i < led_count; ++i) {
        float ratio = static_cast<float>(i) / static_cast<float>(led_count - 1);
        unsigned int interpolated_color = InterpolateColor(primary_color, secondary_color, ratio);
        controller->colors[i] = ConvertToRGBColor(interpolated_color);
    }
    
    controller->UpdateLEDs();
}

unsigned int WindowsDynamicLightingSyncWidget::GenerateHarmoniousSecondaryColor(unsigned int primary_color)
{
    float h, s, v;
    RGBtoHSV(primary_color, h, s, v);
    
    // Generate complementary color with slight adjustments for harmony
    float secondary_h = fmod(h + 180.0f + (rand() % 60 - 30), 360.0f); // Complementary with variation
    float secondary_s = s * 0.8f; // Slightly less saturated
    float secondary_v = v * 0.9f; // Slightly darker
    
    return HSVtoRGB(secondary_h, secondary_s, secondary_v);
}

unsigned int WindowsDynamicLightingSyncWidget::InterpolateColor(unsigned int color1, unsigned int color2, float ratio)
{
    if (ratio <= 0.0f) return color1;
    if (ratio >= 1.0f) return color2;
    
    unsigned int r1 = (color1 >> 16) & 0xFF;
    unsigned int g1 = (color1 >> 8) & 0xFF;
    unsigned int b1 = color1 & 0xFF;
    
    unsigned int r2 = (color2 >> 16) & 0xFF;
    unsigned int g2 = (color2 >> 8) & 0xFF;
    unsigned int b2 = color2 & 0xFF;
    
    unsigned int r = static_cast<unsigned int>(r1 + (r2 - r1) * ratio);
    unsigned int g = static_cast<unsigned int>(g1 + (g2 - g1) * ratio);
    unsigned int b = static_cast<unsigned int>(b1 + (b2 - b1) * ratio);
    
    return (r << 16) | (g << 8) | b;
}

void WindowsDynamicLightingSyncWidget::RGBtoHSV(unsigned int rgb, float& h, float& s, float& v)
{
    float r = ((rgb >> 16) & 0xFF) / 255.0f;
    float g = ((rgb >> 8) & 0xFF) / 255.0f;
    float b = (rgb & 0xFF) / 255.0f;
    
    float max_val = std::max({r, g, b});
    float min_val = std::min({r, g, b});
    float delta = max_val - min_val;
    
    v = max_val;
    s = (max_val == 0.0f) ? 0.0f : (delta / max_val);
    
    if (delta == 0.0f) {
        h = 0.0f;
    } else if (max_val == r) {
        h = 60.0f * fmod((g - b) / delta, 6.0f);
    } else if (max_val == g) {
        h = 60.0f * ((b - r) / delta + 2.0f);
    } else {
        h = 60.0f * ((r - g) / delta + 4.0f);
    }
    
    if (h < 0.0f) h += 360.0f;
}

unsigned int WindowsDynamicLightingSyncWidget::HSVtoRGB(float h, float s, float v)
{
    float c = v * s;
    float x = c * (1.0f - std::abs(fmod(h / 60.0f, 2.0f) - 1.0f));
    float m = v - c;
    
    float r, g, b;
    
    if (h >= 0.0f && h < 60.0f) {
        r = c; g = x; b = 0.0f;
    } else if (h >= 60.0f && h < 120.0f) {
        r = x; g = c; b = 0.0f;
    } else if (h >= 120.0f && h < 180.0f) {
        r = 0.0f; g = c; b = x;
    } else if (h >= 180.0f && h < 240.0f) {
        r = 0.0f; g = x; b = c;
    } else if (h >= 240.0f && h < 300.0f) {
        r = x; g = 0.0f; b = c;
    } else {
        r = c; g = 0.0f; b = x;
    }
    
    unsigned int red = static_cast<unsigned int>((r + m) * 255.0f);
    unsigned int green = static_cast<unsigned int>((g + m) * 255.0f);
    unsigned int blue = static_cast<unsigned int>((b + m) * 255.0f);
    
    return (red << 16) | (green << 8) | blue;
}

QString WindowsDynamicLightingSyncWidget::ColorToHex(unsigned int color)
{
    return QString("#%1").arg(color, 6, 16, QChar('0')).toUpper();
}

void WindowsDynamicLightingSyncWidget::SetColorDisplay(QLabel* label, unsigned int color)
{
    QString hex = ColorToHex(color);
    label->setText(hex);
    label->setStyleSheet(QString("background-color: %1; border: 1px solid black; color: %2;")
                        .arg(hex)
                        .arg((color & 0x808080) ? "white" : "black"));
}

void WindowsDynamicLightingSyncWidget::ShowColorPicker()
{
    QColor current_color = QColor(
        (current_secondary_color >> 16) & 0xFF,
        (current_secondary_color >> 8) & 0xFF,
        current_secondary_color & 0xFF
    );
    
    QColor new_color = QColorDialog::getColor(current_color, this, "Escolher Cor Secundária");
    
    if (new_color.isValid()) {
        current_secondary_color = (new_color.red() << 16) | (new_color.green() << 8) | new_color.blue();
        UpdateColorDisplays();
        SaveSettings();
        
        if (sync_active) {
            ApplyColorsToDevices();
        }
    }
}

void WindowsDynamicLightingSyncWidget::OnAutoSecondaryToggled(bool enabled)
{
    secondary_color_button->setEnabled(!enabled && mode_combo->currentData().toInt() == 1);
    
    if (enabled) {
        current_secondary_color = GenerateHarmoniousSecondaryColor(current_primary_color);
        UpdateColorDisplays();
        
        if (sync_active) {
            ApplyColorsToDevices();
        }
    }
    
    SaveSettings();
}

void WindowsDynamicLightingSyncWidget::OnDeviceItemChanged(QListWidgetItem* item)
{
    UpdateStatusInfo();
    SaveSettings();
}

void WindowsDynamicLightingSyncWidget::OnUpdateTimer()
{
    UpdateStatusInfo();
}

void WindowsDynamicLightingSyncWidget::LoadSettings()
{
    // Settings will be loaded from the plugin's settings instance
    // This is a placeholder for now
}

void WindowsDynamicLightingSyncWidget::SaveSettings()
{
    // Settings will be saved to the plugin's settings instance
    // This is a placeholder for now
}

void WindowsDynamicLightingSyncWidget::OnDeviceSelectionChanged()
{
    // Handle device selection changes
    // This is a placeholder for now
}

void WindowsDynamicLightingSyncWidget::OnSecondaryColorChanged()
{
    // Handle secondary color changes
    // This is a placeholder for now
}

void WindowsDynamicLightingSyncWidget::OnAntiBugToggled(bool enabled)
{
    // Handle anti-bug mode toggle
    // This is a placeholder for now
}

// Helper function to convert unsigned int to RGBColor
RGBColor WindowsDynamicLightingSyncWidget::ConvertToRGBColor(unsigned int color)
{
    unsigned char r = (color >> 16) & 0xFF;  // Red
    unsigned char g = (color >> 8) & 0xFF;   // Green
    unsigned char b = color & 0xFF;          // Blue
    return ToRGBColor(r, g, b);
}

#include "WindowsDynamicLightingSyncWidget.moc"