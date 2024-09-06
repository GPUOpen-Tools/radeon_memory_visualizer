//=============================================================================
// Copyright (c) 2020-2024 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Implementation of the device configuration pane.
//=============================================================================

#include "views/timeline/device_configuration_pane.h"

#include "qt_common/utils/qt_util.h"

#include "managers/message_manager.h"
#include "util/widget_util.h"

DeviceConfigurationPane::DeviceConfigurationPane(QWidget* parent)
    : BasePane(parent)
    , ui_(new Ui::DeviceConfigurationPane)
{
    ui_->setupUi(this);

    // Set mouse cursor to pointing hand cursor for various widgets.
    ui_->button_copy_to_clipboard_->setCursor(Qt::PointingHandCursor);

    // Hide the copy to clipboard button until it's implemented.
    ui_->button_copy_to_clipboard_->hide();
    ui_->horizontal_spacer_->changeSize(0, 0, QSizePolicy::Fixed, QSizePolicy::Fixed);

    if (QtCommon::QtUtils::ColorTheme::Get().GetColorTheme() == ColorThemeType::kColorThemeTypeDark)
    {
        ui_->label_amd_logo_->setStyleSheet("image: url(:/Resources/assets/amd_logo_white.svg)");
    }

    connect(&QtCommon::QtUtils::ColorTheme::Get(), &QtCommon::QtUtils::ColorTheme::ColorThemeUpdated, this, &DeviceConfigurationPane::OnColorThemeUpdated);

    model_ = new rmv::DeviceConfigurationModel();

    model_->InitializeModel(ui_->content_processor_brand_, rmv::kDeviceConfigurationCPUName, "text");
    model_->InitializeModel(ui_->content_processor_speed_, rmv::kDeviceConfigurationCPUSpeed, "text");
    model_->InitializeModel(ui_->content_physical_cores_, rmv::kDeviceConfigurationCPUPhysicalCores, "text");
    model_->InitializeModel(ui_->content_logical_cores_, rmv::kDeviceConfigurationCPULogicalCores, "text");
    model_->InitializeModel(ui_->content_system_memory_, rmv::kDeviceConfigurationSystemMemorySize, "text");
    model_->InitializeModel(ui_->content_device_name_, rmv::kDeviceConfigurationDeviceName, "text");
    model_->InitializeModel(ui_->content_device_id_, rmv::kDeviceConfigurationDeviceID, "text");
    model_->InitializeModel(ui_->content_memory_size_, rmv::kDeviceConfigurationMemorySize, "text");
    model_->InitializeModel(ui_->content_shader_core_clock_frequency_, rmv::kDeviceConfigurationShaderCoreClockFrequency, "text");
    model_->InitializeModel(ui_->content_memory_clock_frequency_, rmv::kDeviceConfigurationMemoryClockFrequency, "text");
    model_->InitializeModel(ui_->content_local_memory_bandwidth_, rmv::kDeviceConfigurationLocalMemoryBandwidth, "text");
    model_->InitializeModel(ui_->content_local_memory_type_, rmv::kDeviceConfigurationLocalMemoryType, "text");
    model_->InitializeModel(ui_->content_local_memory_bus_width_, rmv::kDeviceConfigurationLocalMemoryBusWidth, "text");
    model_->InitializeModel(ui_->content_driver_packaging_version_, rmv::kDeviceConfigurationDriverPackagingVersion, "text");
    model_->InitializeModel(ui_->content_driver_software_version_, rmv::kDeviceConfigurationDriverSoftwareVersion, "text");
}

DeviceConfigurationPane::~DeviceConfigurationPane()
{
    delete ui_;
    delete model_;
}

void DeviceConfigurationPane::showEvent(QShowEvent* event)
{
    Refresh();
    QWidget::showEvent(event);
    bool visible = model_->ExtendedInfoAvailable();

    ui_->label_title_system_->setVisible(visible);
    ui_->label_processor_brand_->setVisible(visible);
    ui_->content_processor_brand_->setVisible(visible);
    ui_->label_processor_speed_->setVisible(visible);
    ui_->content_processor_speed_->setVisible(visible);
    ui_->label_physical_cores_->setVisible(visible);
    ui_->content_physical_cores_->setVisible(visible);
    ui_->label_logical_cores_->setVisible(visible);
    ui_->content_logical_cores_->setVisible(visible);
    ui_->label_system_memory_->setVisible(visible);
    ui_->content_system_memory_->setVisible(visible);

    ui_->label_driver_information_->setVisible(visible);
    ui_->label_driver_packaging_version_->setVisible(visible);
    ui_->content_driver_packaging_version_->setVisible(visible);
#ifdef WIN32
    // Driver software version is Windows only.
    ui_->label_driver_software_version_->setVisible(visible);
    ui_->content_driver_software_version_->setVisible(visible);
#else
    ui_->label_driver_software_version_->setVisible(false);
    ui_->content_driver_software_version_->setVisible(false);
#endif
}

void DeviceConfigurationPane::OnColorThemeUpdated()
{
    if (QtCommon::QtUtils::ColorTheme::Get().GetColorTheme() == ColorThemeType::kColorThemeTypeDark)
    {
        ui_->label_amd_logo_->setStyleSheet("QLabel#label_amd_logo_ { image: url(:/Resources/assets/amd_logo_white.svg); }");
    }
    else
    {
        ui_->label_amd_logo_->setStyleSheet("QLabel#label_amd_logo_ { image: url(:/Resources/assets/amd_logo.svg); }");
    }
}

void DeviceConfigurationPane::Refresh()
{
    model_->Update();
}

void DeviceConfigurationPane::Reset()
{
    model_->ResetModelValues();
}
