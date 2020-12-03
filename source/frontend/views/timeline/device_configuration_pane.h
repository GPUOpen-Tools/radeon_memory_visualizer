//=============================================================================
/// Copyright (c) 2020 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief Header for RMV's device configuration pane.
//=============================================================================

#ifndef RMV_VIEWS_TIMELINE_DEVICE_CONFIGURATION_PANE_H_
#define RMV_VIEWS_TIMELINE_DEVICE_CONFIGURATION_PANE_H_

#include "ui_device_configuration_pane.h"

#include <QWidget>

#include "models/timeline/device_configuration_model.h"
#include "views/base_pane.h"

class DeviceConfigurationPane : public BasePane
{
    Q_OBJECT

public:
    /// Constructor.
    /// \param parent The widget's parent.
    explicit DeviceConfigurationPane(QWidget* parent = nullptr);

    /// Destructor.
    ~DeviceConfigurationPane();

    /// Overridden Qt show event. Fired when this pane is opened.
    /// \param event The show event object.
    virtual void showEvent(QShowEvent* event) Q_DECL_OVERRIDE;

    /// Reset UI state.
    virtual void Reset() Q_DECL_OVERRIDE;

private:
    /// Refresh the UI.
    void Refresh();

    Ui::DeviceConfigurationPane*   ui_;     ///< Pointer to the Qt UI design.
    rmv::DeviceConfigurationModel* model_;  ///< The model for this pane.
};

#endif  // RMV_VIEWS_TIMELINE_DEVICE_CONFIGURATION_PANE_H_
