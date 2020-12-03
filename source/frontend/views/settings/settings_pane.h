//=============================================================================
/// Copyright (c) 2018-2020 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Header for the Settings pane.
//=============================================================================

#ifndef RMV_VIEWS_SETTINGS_SETTINGS_PANE_H_
#define RMV_VIEWS_SETTINGS_SETTINGS_PANE_H_

#include "ui_settings_pane.h"

#include "views/base_pane.h"

/// Class declaration.
class SettingsPane : public BasePane
{
    Q_OBJECT

public:
    /// Constructor.
    /// \param parent The widget's parent.
    explicit SettingsPane(QWidget* parent = nullptr);

    /// Destructor.
    virtual ~SettingsPane();

    /// Overridden show event. Fired when this pane is opened.
    /// \param event the show event object.
    virtual void showEvent(QShowEvent* event) Q_DECL_OVERRIDE;

    /// Cycle through the available time units.
    void CycleTimeUnits();

public slots:
    /// Slot to handle what happens when the auto updates box changes.
    /// Update and save the settings.
    void CheckForUpdatesOnStartupStateChanged();

    /// Slot to handle what happens when the time units combo box has changed.
    void TimeUnitsChanged();

    /// Slot to handle what happens when the heap alloc uniqueness box changes.
    /// Update and save the settings.
    void HeapUniquenessSelectionStateChanged();

    /// Slot to handle what happens when the block alloc uniqueness box changes.
    /// Update and save the settings.
    void AllocationUniquenessSelectionStateChanged();

    /// Slot to handle what happens when the offset alloc uniqueness box changes.
    /// Update and save the settings.
    void OffsetUniquenessSelectionStateChanged();

private:
    /// Save the current time units setting.
    /// \param current_index The new index of the combo box corresponding to the new
    /// time units.
    void SaveTimeUnits(int current_index);

    /// Update the time unit combo box. Take into account that the mapping of the
    /// time value enums and the indices in the combo box is not linear.
    /// \param units The time units
    void UpdateTimeComboBox(int units);

    Ui::SettingsPane* ui_;  ///< Pointer to the Qt UI design.
};

#endif  // RMV_VIEWS_SETTINGS_SETTINGS_PANE_H_
