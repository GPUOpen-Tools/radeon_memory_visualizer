//=============================================================================
// Copyright (c) 2019-2023 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Implementation for the Resource properties model.
///
/// This is a model to go with a QTableView showing the properties for each
/// type of resource. The table will be populated with properties specific to
/// a resource type.
///
//=============================================================================

#include "models/snapshot/resource_properties_model.h"

#include <QStandardItemModel>

#include "rmt_assert.h"
#include "rmt_data_snapshot.h"
#include "rmt_print.h"
#include "rmt_util.h"
#include "rmt_virtual_allocation_list.h"

#include "managers/trace_manager.h"
#include "util/string_util.h"

namespace rmv
{
    // Enum for column indices in the resource properties table.
    enum ResourcePropertiesColumn
    {
        kResourcePropertyName,
        kResourcePropertyValue,
    };

    // Some sufficiently large value to set up the number of rows in the
    // table so the row count doesn't need to be precalculated (it will
    // vary depending on the resource type).
    static const int kMaxProperties = 200;

    ResourcePropertiesModel::ResourcePropertiesModel()
        : table_model_(nullptr)
    {
    }

    ResourcePropertiesModel::~ResourcePropertiesModel()
    {
        delete table_model_;
    }

    void ResourcePropertiesModel::InitializeTableModel(QTableView* table_view, uint num_rows, uint num_columns)
    {
        table_model_ = new QStandardItemModel(num_rows, num_columns);

        int columns = 0;
        table_model_->setHorizontalHeaderItem(columns++, new QStandardItem("Property name"));
        table_model_->setHorizontalHeaderItem(columns++, new QStandardItem("Property value"));

        table_model_->setColumnCount(columns);

        table_view->setModel(table_model_);
    }

    void ResourcePropertiesModel::ResetModelValues()
    {
        table_model_->removeRows(0, table_model_->rowCount());
    }

    int32_t ResourcePropertiesModel::Update(RmtResourceIdentifier resource_identifier)
    {
        ResetModelValues();
        return UpdateTable(resource_identifier);
    }

    int32_t ResourcePropertiesModel::UpdateTable(RmtResourceIdentifier resource_identifier)
    {
        int row_index = 0;

        if (TraceManager::Get().DataSetValid())
        {
            const RmtDataSnapshot* snapshot = SnapshotManager::Get().GetOpenSnapshot();
            if (snapshot != nullptr)
            {
                const RmtResource* resource   = nullptr;
                const RmtErrorCode error_code = RmtResourceListGetResourceByResourceId(&snapshot->resource_list, resource_identifier, &resource);
                if (error_code == kRmtOk)
                {
                    table_model_->setRowCount(kMaxProperties);

                    switch (resource->resource_type)
                    {
                    case kRmtResourceTypeImage:
                        row_index = AddImageTableData(resource, row_index);
                        break;

                    case kRmtResourceTypeBuffer:
                        row_index = AddBufferTableData(resource, row_index);
                        break;

                    case kRmtResourceTypeGpuEvent:
                        row_index = AddGPUEventTableData(resource, row_index);
                        break;

                    case kRmtResourceTypeBorderColorPalette:
                        row_index = AddBorderColorPaletteTableData(resource, row_index);
                        break;

                    case kRmtResourceTypePerfExperiment:
                        row_index = AddPerfExperimentTableData(resource, row_index);
                        break;

                    case kRmtResourceTypeQueryHeap:
                        row_index = AddQueryHeapTableData(resource, row_index);
                        break;

                    case kRmtResourceTypeVideoDecoder:
                        row_index = AddVideoDecoderTableData(resource, row_index);
                        break;

                    case kRmtResourceTypeVideoEncoder:
                        row_index = AddVideoEncoderTableData(resource, row_index);
                        break;

                    case kRmtResourceTypeHeap:
                        row_index = AddHeapTableData(resource, row_index);
                        break;

                    case kRmtResourceTypePipeline:
                        row_index = AddPipelineTableData(resource, row_index);
                        break;

                    case kRmtResourceTypeDescriptorHeap:
                        row_index = AddDescriptorHeapTableData(resource, row_index);
                        break;

                    case kRmtResourceTypeDescriptorPool:
                        row_index = AddDescriptorPoolTableData(resource, row_index);
                        break;

                    case kRmtResourceTypeCommandAllocator:
                        row_index = AddCommandAllocatorTableData(resource, row_index);
                        break;

                    case kRmtResourceTypeIndirectCmdGenerator:
                    case kRmtResourceTypeMotionEstimator:
                    case kRmtResourceTypeTimestamp:
                    default:
                        break;
                    }

                    table_model_->setRowCount(row_index);
                }
            }
        }
        return row_index;
    }

    void ResourcePropertiesModel::SetupResourceRow(const QString& name, const QString& value, int row)
    {
        table_model_->setData(table_model_->index(row, kResourcePropertyName), name);
        table_model_->setData(table_model_->index(row, kResourcePropertyValue), value);
    }

    int ResourcePropertiesModel::AddImageTableData(const RmtResource* resource, int row_index)
    {
        char flags_text[1024];

        RmtGetImageCreationNameFromImageCreationFlags(resource->image.create_flags, flags_text, 1024);
        SetupResourceRow("Create flags", flags_text, row_index++);

        RmtGetImageUsageNameFromImageUsageFlags(resource->image.usage_flags, flags_text, 1024);
        SetupResourceRow("Usage flags", flags_text, row_index++);

        SetupResourceRow("Image type", RmtGetImageTypeNameFromImageType(resource->image.image_type), row_index++);
        SetupResourceRow("X Dimension", rmv::string_util::LocalizedValue(resource->image.dimension_x), row_index++);
        SetupResourceRow("Y Dimension", rmv::string_util::LocalizedValue(resource->image.dimension_y), row_index++);
        SetupResourceRow("Z Dimension", rmv::string_util::LocalizedValue(resource->image.dimension_z), row_index++);

        SetupResourceRow("Format", QString(RmtGetFormatNameFromFormat(resource->image.format.format)), row_index++);

        char swizzle_pattern[8];
        RmtGetSwizzlePatternFromImageFormat(&resource->image.format, swizzle_pattern, sizeof(swizzle_pattern));
        SetupResourceRow("Swizzle", QString(swizzle_pattern), row_index++);

        SetupResourceRow("Mip levels", rmv::string_util::LocalizedValue(resource->image.mip_levels), row_index++);
        SetupResourceRow("Slices", rmv::string_util::LocalizedValue(resource->image.slices), row_index++);
        SetupResourceRow("Sample count", rmv::string_util::LocalizedValue(resource->image.sample_count), row_index++);
        SetupResourceRow("Fragment count", rmv::string_util::LocalizedValue(resource->image.fragment_count), row_index++);
        SetupResourceRow("Tiling type", RmtGetTilingNameFromTilingType(resource->image.tiling_type), row_index++);
        SetupResourceRow(
            "Tiling optimization mode", RmtGetTilingOptimizationModeNameFromTilingOptimizationMode(resource->image.tiling_optimization_mode), row_index++);
        SetupResourceRow("Metadata mode", rmv::string_util::LocalizedValue(resource->image.metadata_mode), row_index++);
        SetupResourceRow("Max base alignment", rmv::string_util::LocalizedValueMemory(resource->image.max_base_alignment, false, false), row_index++);
        SetupResourceRow("Image offset", rmv::string_util::LocalizedValueMemory(resource->image.image_offset, false, false), row_index++);
        SetupResourceRow("Image size", rmv::string_util::LocalizedValueMemory(resource->image.image_size, false, false), row_index++);
        SetupResourceRow("Image alignment", rmv::string_util::LocalizedValueMemory(resource->image.image_alignment, false, false), row_index++);
        SetupResourceRow("Metadata head offset", rmv::string_util::LocalizedValueMemory(resource->image.metadata_head_offset, false, false), row_index++);
        SetupResourceRow("Metadata head size", rmv::string_util::LocalizedValueMemory(resource->image.metadata_head_size, false, false), row_index++);
        SetupResourceRow("Metadata head alignment", rmv::string_util::LocalizedValueMemory(resource->image.metadata_head_alignment, false, false), row_index++);
        SetupResourceRow("Metadata tail offset", rmv::string_util::LocalizedValueMemory(resource->image.metadata_tail_offset, false, false), row_index++);
        SetupResourceRow("Metadata tail size", rmv::string_util::LocalizedValueMemory(resource->image.metadata_tail_size, false, false), row_index++);
        SetupResourceRow("Metadata tail alignment", rmv::string_util::LocalizedValueMemory(resource->image.metadata_tail_alignment, false, false), row_index++);
        SetupResourceRow("Presentable", resource->image.presentable == true ? "True" : "False", row_index++);
        SetupResourceRow("Fullscreen", resource->image.fullscreen == true ? "True" : "False", row_index++);
        return row_index;
    }

    int ResourcePropertiesModel::AddBufferTableData(const RmtResource* resource, int row_index)
    {
        char flags_text[1024];

        RmtGetBufferCreationNameFromBufferCreationFlags(resource->buffer.create_flags, flags_text, 1024);
        SetupResourceRow("Create flags", flags_text, row_index++);

        RmtGetBufferUsageNameFromBufferUsageFlags(resource->buffer.usage_flags, flags_text, 1024);
        SetupResourceRow("Usage flags", flags_text, row_index++);

        SetupResourceRow("Size", rmv::string_util::LocalizedValueMemory(resource->buffer.size_in_bytes, false, false), row_index++);
        return row_index;
    }

    int ResourcePropertiesModel::AddGPUEventTableData(const RmtResource* resource, int row_index)
    {
        char flags_text[1024];

        RmtGetGpuEventNameFromGpuEventFlags(resource->gpu_event.flags, flags_text, 1024);
        SetupResourceRow("Flags", flags_text, row_index++);

        return row_index;
    }

    int ResourcePropertiesModel::AddBorderColorPaletteTableData(const RmtResource* resource, int row_index)
    {
        SetupResourceRow("Size in entries", rmv::string_util::LocalizedValue(resource->border_color_palette.size_in_entries), row_index++);
        return row_index;
    }

    int ResourcePropertiesModel::AddPerfExperimentTableData(const RmtResource* resource, int row_index)
    {
        SetupResourceRow("SPM memory size", rmv::string_util::LocalizedValueMemory(resource->perf_experiment.spm_size, false, false), row_index++);
        SetupResourceRow("SQTT memory size", rmv::string_util::LocalizedValueMemory(resource->perf_experiment.sqtt_size, false, false), row_index++);
        SetupResourceRow("Counter memory size", rmv::string_util::LocalizedValueMemory(resource->perf_experiment.counter_size, false, false), row_index++);
        return row_index;
    }

    int ResourcePropertiesModel::AddQueryHeapTableData(const RmtResource* resource, int row_index)
    {
        SetupResourceRow("Heap type", rmv::string_util::LocalizedValue(resource->query_heap.heap_type), row_index++);
        SetupResourceRow("Enable CPU access", resource->query_heap.enable_cpu_access == true ? "True" : "False", row_index++);
        return row_index;
    }

    int ResourcePropertiesModel::AddVideoDecoderTableData(const RmtResource* resource, int row_index)
    {
        SetupResourceRow("Engine type", rmv::string_util::LocalizedValue(resource->video_decoder.engine_type), row_index++);
        SetupResourceRow("Decoder type", rmv::string_util::LocalizedValue(resource->video_decoder.decoder_type), row_index++);
        SetupResourceRow("Width", rmv::string_util::LocalizedValue(resource->video_decoder.width), row_index++);
        SetupResourceRow("Height", rmv::string_util::LocalizedValue(resource->video_decoder.height), row_index++);
        return row_index;
    }

    int ResourcePropertiesModel::AddVideoEncoderTableData(const RmtResource* resource, int row_index)
    {
        SetupResourceRow("Engine type", rmv::string_util::LocalizedValue(resource->video_encoder.engine_type), row_index++);
        SetupResourceRow("Encoder type", rmv::string_util::LocalizedValue(resource->video_encoder.encoder_type), row_index++);
        SetupResourceRow("Width", rmv::string_util::LocalizedValue(resource->video_encoder.width), row_index++);
        SetupResourceRow("Height", rmv::string_util::LocalizedValue(resource->video_encoder.height), row_index++);
        SetupResourceRow(
            "Format",
            QString(RmtGetFormatNameFromFormat(resource->video_encoder.format.format)) + " (" + QString::number(resource->video_encoder.format.format) + ")",
            row_index++);
        return row_index;
    }

    int ResourcePropertiesModel::AddHeapTableData(const RmtResource* resource, int row_index)
    {
        SetupResourceRow("Flags", rmv::string_util::LocalizedValue(resource->heap.flags), row_index++);
        SetupResourceRow("Size", rmv::string_util::LocalizedValueMemory(resource->heap.size, false, false), row_index++);
        SetupResourceRow("Alignment", rmv::string_util::LocalizedValue(resource->heap.alignment), row_index++);
        SetupResourceRow("Segment index", rmv::string_util::LocalizedValue(resource->heap.segment_index), row_index++);
        return row_index;
    }

    int ResourcePropertiesModel::AddPipelineTableData(const RmtResource* resource, int row_index)
    {
        char flags_text[1024];
        RmtGetPipelineCreationNameFromPipelineCreationFlags(resource->pipeline.create_flags, flags_text, 1024);
        SetupResourceRow("Create flags", flags_text, row_index++);

        SetupResourceRow(
            "Internal Pipeline hash",
            rmv::string_util::Convert128BitHashToString(resource->pipeline.internal_pipeline_hash_hi, resource->pipeline.internal_pipeline_hash_lo),
            row_index++);

        RmtGetPipelineStageNameFromPipelineStageFlags(resource->pipeline.stage_mask, flags_text, 1024);
        SetupResourceRow("Stage mask", flags_text, row_index++);

        SetupResourceRow("Is NGG", resource->pipeline.is_ngg == true ? "True" : "False", row_index++);
        return row_index;
    }

    int ResourcePropertiesModel::AddDescriptorHeapTableData(const RmtResource* resource, int row_index)
    {
        SetupResourceRow("Descriptor Type", rmv::string_util::LocalizedValue(resource->descriptor_heap.descriptor_type), row_index++);
        SetupResourceRow("Shader visible", resource->descriptor_heap.shader_visible == true ? "True" : "False", row_index++);
        SetupResourceRow("GPU mask", rmv::string_util::LocalizedValue(resource->descriptor_heap.gpu_mask), row_index++);
        SetupResourceRow("Num descriptors", rmv::string_util::LocalizedValue(resource->descriptor_heap.num_descriptors), row_index++);
        return row_index;
    }

    int ResourcePropertiesModel::AddDescriptorPoolTableData(const RmtResource* resource, int row_index)
    {
        SetupResourceRow("Max sets", rmv::string_util::LocalizedValue(resource->descriptor_pool.max_sets), row_index++);
        SetupResourceRow("Pools count", rmv::string_util::LocalizedValue(resource->descriptor_pool.pools_count), row_index++);

        RMT_ASSERT(resource->descriptor_pool.pools_count < RMT_MAX_POOLS);
        for (int i = 0; i < resource->descriptor_pool.pools_count; i++)
        {
            QString typeString       = QString("Pool[%1] type").arg(i);
            QString descriptorString = QString("Pool[%1] descriptor count").arg(i);

            SetupResourceRow(typeString, rmv::string_util::LocalizedValue(resource->descriptor_pool.pools[i].type), row_index++);
            SetupResourceRow(descriptorString, rmv::string_util::LocalizedValue(resource->descriptor_pool.pools[i].num_descriptors), row_index++);
        }
        return row_index;
    }

    int ResourcePropertiesModel::AddCommandAllocatorTableData(const RmtResource* resource, int row_index)
    {
        char flags_text[1024];
        RmtGetCmdAllocatorNameFromCmdAllocatorFlags(resource->command_allocator.flags, flags_text, 1024);

        SetupResourceRow("Flags", flags_text, row_index++);
        SetupResourceRow("Executable preferred heap", rmv::string_util::LocalizedValue(resource->command_allocator.cmd_data_heap), row_index++);
        SetupResourceRow("Executable size", rmv::string_util::LocalizedValueMemory(resource->command_allocator.cmd_data_size, false, false), row_index++);
        SetupResourceRow(
            "Executable suballoc size", rmv::string_util::LocalizedValueMemory(resource->command_allocator.cmd_data_suballoc_size, false, false), row_index++);
        SetupResourceRow("Embedded preferred heap", rmv::string_util::LocalizedValue(resource->command_allocator.embed_data_heap), row_index++);
        SetupResourceRow("Embedded size", rmv::string_util::LocalizedValueMemory(resource->command_allocator.embed_data_size, false, false), row_index++);
        SetupResourceRow(
            "Embedded suballoc size", rmv::string_util::LocalizedValueMemory(resource->command_allocator.embed_data_suballoc_size, false, false), row_index++);
        SetupResourceRow("GPU scratch preferred heap", rmv::string_util::LocalizedValue(resource->command_allocator.embed_data_heap), row_index++);
        SetupResourceRow("GPU scratch size", rmv::string_util::LocalizedValueMemory(resource->command_allocator.embed_data_size, false, false), row_index++);
        SetupResourceRow("GPU scratch suballoc size",
                         rmv::string_util::LocalizedValueMemory(resource->command_allocator.embed_data_suballoc_size, false, false),
                         row_index++);
        return row_index;
    }

}  // namespace rmv