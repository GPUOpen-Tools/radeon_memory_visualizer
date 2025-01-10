//=============================================================================
// Copyright (c) 2018-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Header for the Settings pane.
//=============================================================================

#ifndef RMV_VIEWS_SETTINGS_SETTINGS_PANE_H_
#define RMV_VIEWS_SETTINGS_SETTINGS_PANE_H_

#include "ui_settings_pane.h"

#include "views/base_pane.h"

/// @brief Class declaration.
class SettingsPane : public BasePane
{
    Q_OBJECT

public:
    /// @brief Constructor.
    ///
    /// @param [in] parent The parent widget.
    explicit SettingsPane(QWidget* parent = nullptr);

    /// @brief Destructor.
    virtual ~SettingsPane();

    /// @brief Overridden show event. Fired when this pane is opened.
    ///
    /// @param [in] event the show event object.
    virtual void showEvent(QShowEvent* event) Q_DECL_OVERRIDE;

    /// @brief Update time units.
    virtual void SwitchTimeUnits() Q_DECL_OVERRIDE;

public slots:
    /// @brief Slot to handle what happens when the auto updates box changes.
    ///
    /// Update and save the settings.
    void CheckForUpdatesOnStartupStateChanged();

    /// @brief Slot to handle what happens when the Driver Overrides notification check box changes.
    ///
    /// @param checked The new state of the check box.
    void DriverOverridesAllowNotificationsChanged(bool checked);

    /// @brief Slot to handle what happens when the time units combo box has changed.
    void TimeUnitsChanged();

private:
    /// @brief Update the time unit combo box.
    ///
    /// Take into account that the mapping of the time value enums and the indices
    /// in the combo box is not linear.
    ///
    /// @param [in] units The time units.
    void UpdateTimeComboBox(int units);

    Ui::SettingsPane* ui_;  ///< Pointer to the Qt UI design.
};

#endif  // RMV_VIEWS_SETTINGS_SETTINGS_PANE_H_
