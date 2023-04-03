//=============================================================================
// Copyright (c) 2020-2023 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Implementation for the Device configuration model.
//=============================================================================

#include "models/timeline/device_configuration_model.h"

#include <QVariant>

#include "rmt_adapter_info.h"
#include "rmt_data_set.h"

#include "managers/trace_manager.h"
#include "util/string_util.h"
#include "util/widget_util.h"

namespace rmv
{
    DeviceConfigurationModel::DeviceConfigurationModel()
        : ModelViewMapper(kDeviceConfigurationNumWidgets)
    {
    }

    DeviceConfigurationModel::~DeviceConfigurationModel()
    {
    }

    void DeviceConfigurationModel::ResetModelValues()
    {
        SetModelData(kDeviceConfigurationCPUName, "-");
        SetModelData(kDeviceConfigurationCPUSpeed, "-");
        SetModelData(kDeviceConfigurationCPUPhysicalCores, "-");
        SetModelData(kDeviceConfigurationCPULogicalCores, "-");
        SetModelData(kDeviceConfigurationSystemMemorySize, "-");
        SetModelData(kDeviceConfigurationDeviceName, "-");
        SetModelData(kDeviceConfigurationDeviceID, "-");
        SetModelData(kDeviceConfigurationMemorySize, "-");
        SetModelData(kDeviceConfigurationShaderCoreClockFrequency, "-");
        SetModelData(kDeviceConfigurationMemoryClockFrequency, "-");
        SetModelData(kDeviceConfigurationLocalMemoryBandwidth, "-");
        SetModelData(kDeviceConfigurationLocalMemoryType, "-");
        SetModelData(kDeviceConfigurationLocalMemoryBusWidth, "-");
        SetModelData(kDeviceConfigurationDriverPackagingVersion, "-");
        SetModelData(kDeviceConfigurationDriverSoftwareVersion, "-");
    }

    void DeviceConfigurationModel::Update()
    {
        TraceManager& trace_manager = TraceManager::Get();
        if (trace_manager.DataSetValid())
        {
            const RmtDataSet*       data_set           = trace_manager.GetDataSet();
            const RmtRdfSystemInfo& system_info        = data_set->system_info;
            const uint32_t          compound_device_id = ((system_info.device_id & 0xffff) << 8) | (system_info.pcie_revision_id & 0xff);

            uint64_t video_memory_size = data_set->segment_info[kRmtHeapTypeLocal].size + data_set->segment_info[kRmtHeapTypeInvisible].size;

            SetModelData(kDeviceConfigurationDeviceName, system_info.name);
            SetModelData(kDeviceConfigurationDeviceID, rmv::string_util::ToUpperCase(QString::number(compound_device_id, 16)).rightJustified(6, '0'));

            SetModelData(kDeviceConfigurationMemorySize, rmv::string_util::LocalizedValueMemory(video_memory_size, false, false));

            SetModelData(kDeviceConfigurationShaderCoreClockFrequency,
                         rmv::string_util::LocalizedValue(system_info.minimum_engine_clock) + QString(" MHz (min) ") +
                             rmv::string_util::LocalizedValue(system_info.maximum_engine_clock) + QString(" MHz (max)"));

            SetModelData(kDeviceConfigurationMemoryClockFrequency,
                         rmv::string_util::LocalizedValue(system_info.minimum_memory_clock) + QString(" MHz (min) ") +
                             rmv::string_util::LocalizedValue(system_info.maximum_memory_clock) + QString(" MHz (max)"));

            uint64_t memory_bandwidth = system_info.memory_bandwidth;
            memory_bandwidth *= (1024 * 1024);
            SetModelData(kDeviceConfigurationLocalMemoryBandwidth, rmv::string_util::LocalizedValueMemory(memory_bandwidth, true, true) + QString("/s"));
            SetModelData(kDeviceConfigurationLocalMemoryType, QString(system_info.memory_type_name).toUpper());
            SetModelData(kDeviceConfigurationLocalMemoryBusWidth, QString::number(system_info.memory_bus_width) + QString("-bit"));

            if (data_set->is_rdf_trace)
            {
                // CPU information.
                SetModelData(kDeviceConfigurationCPUName, QString(system_info.cpu_name));
                SetModelData(kDeviceConfigurationCPUSpeed, rmv::string_util::LocalizedValue(system_info.cpu_max_clock_speed) + QString(" MHz"));
                SetModelData(kDeviceConfigurationCPUPhysicalCores, QString::number(system_info.num_physical_cores));
                SetModelData(kDeviceConfigurationCPULogicalCores, QString::number(system_info.num_logical_cores));

                // System memory.
                SetModelData(kDeviceConfigurationSystemMemorySize, rmv::string_util::LocalizedValueMemory(system_info.system_physical_memory_size, false, true));

                // Driver information.
                SetModelData(kDeviceConfigurationDriverPackagingVersion, QString(system_info.driver_packaging_version_name));
                SetModelData(kDeviceConfigurationDriverSoftwareVersion, QString(system_info.driver_software_version_name));
            }
        }
        else
        {
            ResetModelValues();
        }
    }

    bool DeviceConfigurationModel::ExtendedInfoAvailable()
    {
        TraceManager& trace_manager = TraceManager::Get();
        if (trace_manager.DataSetValid())
        {
            const RmtDataSet* data_set = trace_manager.GetDataSet();
            if (data_set)
            {
                return data_set->is_rdf_trace;
            }
        }
        return false;
    }
}  // namespace rmv