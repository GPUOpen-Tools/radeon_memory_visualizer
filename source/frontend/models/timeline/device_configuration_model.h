//=============================================================================
// Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
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
        kDeviceConfigurationCPUName,
        kDeviceConfigurationCPUSpeed,
        kDeviceConfigurationCPUPhysicalCores,
        kDeviceConfigurationCPULogicalCores,
        kDeviceConfigurationSystemMemorySize,
        kDeviceConfigurationDeviceName,
        kDeviceConfigurationDeviceID,
        kDeviceConfigurationMemorySize,
        kDeviceConfigurationShaderCoreClockFrequency,
        kDeviceConfigurationMemoryClockFrequency,
        kDeviceConfigurationLocalMemoryBandwidth,
        kDeviceConfigurationLocalMemoryType,
        kDeviceConfigurationLocalMemoryBusWidth,
        kDeviceConfigurationDriverPackagingVersion,
        kDeviceConfigurationDriverSoftwareVersion,
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

        /// @brief Is the extended driver information available in the trace file.
        ///
        /// Extended information (such as CPU and driver information) is only available with
        /// The RDF file format and the SystemInfo chunk.
        ///
        /// @return true if extended information is available, false if not.
        bool ExtendedInfoAvailable();

        /// @brief Does the UI need to show the driver software version.
        ///
        /// This will be true if the loaded trace comes from a Windows
        /// machine.
        ///
        /// @return true if the driver software version is to be shown.
        bool IsDriverSoftwareVersionNeeded();
    };

}  // namespace rmv

#endif  // RMV_MODELS_TIMELINE_DEVICE_CONFIGURATION_MODEL_H_
