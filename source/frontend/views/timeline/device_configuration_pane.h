//=============================================================================
// Copyright (c) 2020-2024 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Header for the device configuration pane.
//=============================================================================

#ifndef RMV_VIEWS_TIMELINE_DEVICE_CONFIGURATION_PANE_H_
#define RMV_VIEWS_TIMELINE_DEVICE_CONFIGURATION_PANE_H_

#include "ui_device_configuration_pane.h"

#include <QWidget>

#include "models/timeline/device_configuration_model.h"
#include "views/base_pane.h"

/// @brief Class declaration.
class DeviceConfigurationPane : public BasePane
{
    Q_OBJECT

public:
    /// @brief Constructor.
    ///
    /// @param [in] parent The parent widget.
    explicit DeviceConfigurationPane(QWidget* parent = nullptr);

    /// @brief Destructor.
    ~DeviceConfigurationPane();

    /// @brief Overridden Qt show event. Fired when this pane is opened.
    ///
    /// @param [in] event The show event object.
    virtual void showEvent(QShowEvent* event) Q_DECL_OVERRIDE;

    /// @brief Reset UI state.
    virtual void Reset() Q_DECL_OVERRIDE;

private:
    /// @brief Refresh the UI.
    void Refresh();

    /// @brief Update the pane based on the color theme.
    void OnColorThemeUpdated();

    Ui::DeviceConfigurationPane*   ui_;     ///< Pointer to the Qt UI design.
    rmv::DeviceConfigurationModel* model_;  ///< The model for this pane.
};

#endif  // RMV_VIEWS_TIMELINE_DEVICE_CONFIGURATION_PANE_H_
