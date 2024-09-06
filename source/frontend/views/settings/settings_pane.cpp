//=============================================================================
// Copyright (c) 2018-2024 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Implementation of Settings pane.
//=============================================================================

#include "views/settings/settings_pane.h"

#include "managers/message_manager.h"
#include "settings/rmv_settings.h"
#include "util/widget_util.h"
#include "views/custom_widgets/rmv_colored_checkbox.h"

#include "qt_common/custom_widgets/driver_overrides_model.h"

using namespace driver_overrides;

SettingsPane::SettingsPane(QWidget* parent)
    : BasePane(parent)
    , ui_(new Ui::SettingsPane)
{
    ui_->setupUi(this);

    // Set up the Driver Overrides notification configuration widget.
    ui_->driver_overrides_notification_config_widget_->Init(rmv::RMVSettings::Get().GetDriverOverridesAllowNotifications(), false);

    connect(ui_->driver_overrides_notification_config_widget_,
            &DriverOverridesNotificationConfigWidget::StateChanged,
            this,
            &SettingsPane::DriverOverridesAllowNotificationsChanged);
    
    rmv::widget_util::ApplyStandardPaneStyle(ui_->main_scroll_area_);

    // Set up checkboxes.
    ui_->check_for_updates_on_startup_checkbox_->SetOnText(rmv::text::kCheckForUpdates);
    ui_->check_for_updates_on_startup_checkbox_->SetOffText(rmv::text::kCheckForUpdates);
    ui_->check_for_updates_on_startup_checkbox_->setChecked(rmv::RMVSettings::Get().GetCheckForUpdatesOnStartup());

    ui_->memory_leak_title_->hide();
    ui_->heap_uniqueness_checkbox_->hide();
    ui_->allocation_uniqueness_checkbox_->hide();
    ui_->offset_uniqueness_checkbox_->hide();

    // Populate the time combo box.
    rmv::widget_util::InitSingleSelectComboBox(parent, ui_->units_combo_push_button_, rmv::text::kSettingsUnitsClocks, false);
    ui_->units_combo_push_button_->ClearItems();
    ui_->units_combo_push_button_->AddItem(rmv::text::kSettingsUnitsClocks);
    ui_->units_combo_push_button_->AddItem(rmv::text::kSettingsUnitsMilliseconds);
    ui_->units_combo_push_button_->AddItem(rmv::text::kSettingsUnitsSeconds);
    ui_->units_combo_push_button_->AddItem(rmv::text::kSettingsUnitsMinutes);
    ui_->units_combo_push_button_->AddItem(rmv::text::kSettingsUnitsHours);
    ui_->units_combo_push_button_->SetSelectedRow(0);
    connect(ui_->units_combo_push_button_, &ArrowIconComboBox::SelectionChanged, this, &SettingsPane::TimeUnitsChanged);

    connect(ui_->check_for_updates_on_startup_checkbox_, &CheckBoxWidget::stateChanged, this, &SettingsPane::CheckForUpdatesOnStartupStateChanged);

}

SettingsPane::~SettingsPane()
{
}

void SettingsPane::showEvent(QShowEvent* event)
{
    Q_UNUSED(event);

    // Update the combo box push button text.
    int units = rmv::RMVSettings::Get().GetUnits();
    UpdateTimeComboBox(units);

    QWidget::showEvent(event);
}

void SettingsPane::UpdateTimeComboBox(int units)
{
    // Map back to UI row (skip the time units between TIME_UNIT_TYPE_MS and TIME_UNIT_TYPE_CLK).
    if (units > 0)
    {
        units -= (kTimeUnitTypeMillisecond - kTimeUnitTypeClk - 1);
    }
    ui_->units_combo_push_button_->SetSelectedRow(units);
}

void SettingsPane::TimeUnitsChanged()
{
    int index = ui_->units_combo_push_button_->CurrentRow();

    // Map back from UI row in combo box (skip the time units between TIME_UNIT_TYPE_MS and TIME_UNIT_TYPE_CLK).
    if (index > 0)
    {
        index += (kTimeUnitTypeMillisecond - kTimeUnitTypeClk - 1);
    }
    rmv::RMVSettings::Get().SetUnits(static_cast<TimeUnitType>(index));
    rmv::RMVSettings::Get().SaveSettings();
}

void SettingsPane::SwitchTimeUnits()
{
    int units = rmv::RMVSettings::Get().GetUnits();
    UpdateTimeComboBox(units);
}

void SettingsPane::CheckForUpdatesOnStartupStateChanged()
{
    rmv::RMVSettings::Get().SetCheckForUpdatesOnStartup(ui_->check_for_updates_on_startup_checkbox_->isChecked());
    rmv::RMVSettings::Get().SaveSettings();
}

void SettingsPane::DriverOverridesAllowNotificationsChanged(bool checked)
{
    rmv::RMVSettings::Get().SetDriverOverridesAllowNotifications(checked);
    rmv::RMVSettings::Get().SaveSettings();
}

