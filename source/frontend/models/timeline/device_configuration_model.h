//=============================================================================
// Copyright (c) 2020-2021 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Header for the Device configuration model.
//=============================================================================

#ifndef RMV_MODELS_TIMELINE_DEVICE_CONFIGURATION_MODEL_H_
#define RMV_MODELS_TIMELINE_DEVICE_CONFIGURATION_MODEL_H_

#include "qt_common/utils/model_view_mapper.h"

namespace rmv
{
    /// @brief An enum of widgets used by the UI and model.
    ///
    /// Used to map UI widgets to their corresponding model data.
    enum DeviceConfigurationWidgets
    {
        kDeviceConfigurationDeviceName,
        kDeviceConfigurationDeviceID,
        kDeviceConfigurationMemorySize,
        kDeviceConfigurationShaderCoreClockFrequency,
        kDeviceConfigurationMemoryClockFrequency,
        kDeviceConfigurationLocalMemoryBandwidth,
        kDeviceConfigurationLocalMemoryType,
        kDeviceConfigurationLocalMemoryBusWidth,

        kDeviceConfigurationNumWidgets
    };

    /// @brief Container class that holds model data for the device configuration pane.
    class DeviceConfigurationModel : public ModelViewMapper
    {
    public:
        /// @brief Constructor.
        explicit DeviceConfigurationModel();

        /// @brief Destructor.
        virtual ~DeviceConfigurationModel();

        /// @brief Initialize blank data for the model.
        void ResetModelValues();

        /// @brief Update the model with data from the back end.
        void Update();
    };

}  // namespace rmv

#endif  // RMV_MODELS_TIMELINE_DEVICE_CONFIGURATION_MODEL_H_
