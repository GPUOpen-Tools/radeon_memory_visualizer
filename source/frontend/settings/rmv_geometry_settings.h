//=============================================================================
/// Copyright (c) 2017-2020 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief Define RMV's window geometry settings.
//=============================================================================

#ifndef RMV_SETTINGS_RMV_GEOMETRY_SETTINGS_H_
#define RMV_SETTINGS_RMV_GEOMETRY_SETTINGS_H_

#include <QWidget>

#include "settings/rmv_settings.h"

class RMVGeometrySettings
{
public:
    /// Constructor.
    RMVGeometrySettings();

    /// Destructor.
    ~RMVGeometrySettings();

    /// Saves a widget's position, size and state in the RMV settings file as a hex string.
    /// \param widget Pointer to the widget who's geometry is to be saved.
    static void Save(QWidget* widget);

    /// Updates a widget's position, size and state from the RMV settings file.
    /// \param widget Pointer to the widget who's geometry is to be restored.
    static bool Restore(QWidget* widget);

    /// Adjust a widget's geometry so that it fit on a single monitor.
    /// \param widget Pointer to the widget who's geometry is to be adjusted.
    static void Adjust(QWidget* widget);
};
#endif  // RMV_SETTINGS_RMV_GEOMETRY_SETTINGS_H_
