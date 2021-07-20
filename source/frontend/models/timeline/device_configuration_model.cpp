//=============================================================================
// Copyright (c) 2020-2021 Advanced Micro Devices, Inc. All rights reserved.
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
        SetModelData(kDeviceConfigurationDeviceName, "-");
        SetModelData(kDeviceConfigurationDeviceID, "-");
        SetModelData(kDeviceConfigurationMemorySize, "-");
        SetModelData(kDeviceConfigurationShaderCoreClockFrequency, "-");
        SetModelData(kDeviceConfigurationMemoryClockFrequency, "-");
        SetModelData(kDeviceConfigurationLocalMemoryBandwidth, "-");
        SetModelData(kDeviceConfigurationLocalMemoryType, "-");
        SetModelData(kDeviceConfigurationLocalMemoryBusWidth, "-");
    }

    void DeviceConfigurationModel::Update()
    {
        TraceManager& trace_manager = TraceManager::Get();
        if (trace_manager.DataSetValid())
        {
            const RmtDataSet*     data_set           = trace_manager.GetDataSet();
            const RmtAdapterInfo& adapter            = data_set->adapter_info;
            const uint32_t        compound_device_id = ((adapter.device_id & 0xffff) << 8) | (adapter.pcie_revision_id & 0xff);

            uint64_t video_memory_size = data_set->segment_info[kRmtHeapTypeLocal].size + data_set->segment_info[kRmtHeapTypeInvisible].size;

            SetModelData(kDeviceConfigurationDeviceName, adapter.name);
            SetModelData(kDeviceConfigurationDeviceID, rmv::string_util::ToUpperCase(QString::number(compound_device_id, 16)).rightJustified(6, '0'));

            SetModelData(kDeviceConfigurationMemorySize, rmv::string_util::LocalizedValueMemory(video_memory_size, false, false));

            SetModelData(kDeviceConfigurationShaderCoreClockFrequency,
                         rmv::string_util::LocalizedValue(adapter.minimum_engine_clock) + QString(" MHz (min) ") +
                             rmv::string_util::LocalizedValue(adapter.maximum_engine_clock) + QString(" MHz (max)"));

            SetModelData(kDeviceConfigurationMemoryClockFrequency,
                         rmv::string_util::LocalizedValue(adapter.minimum_memory_clock) + QString(" MHz (min) ") +
                             rmv::string_util::LocalizedValue(adapter.maximum_memory_clock) + QString(" MHz (max)"));

            uint64_t memory_bandwidth = adapter.memory_bandwidth;
            memory_bandwidth *= (1024 * 1024);
            SetModelData(kDeviceConfigurationLocalMemoryBandwidth, rmv::string_util::LocalizedValueMemory(memory_bandwidth, true, true) + QString("/s"));
            SetModelData(kDeviceConfigurationLocalMemoryType, RmtAdapterInfoGetVideoMemoryType(&adapter));
            SetModelData(kDeviceConfigurationLocalMemoryBusWidth, QString::number(adapter.memory_bus_width) + QString("-bit"));
        }
        else
        {
            ResetModelValues();
        }
    }
}  // namespace rmv