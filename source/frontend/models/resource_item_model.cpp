//=============================================================================
// Copyright (c) 2020-2021 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Implementation for a resource item model.
///
/// Used for the resource list tables.
///
//=============================================================================

#include "models/resource_item_model.h"

#include "rmt_assert.h"
#include "rmt_data_snapshot.h"
#include "rmt_resource_list.h"
#include "rmt_util.h"

#include "managers/trace_manager.h"
#include "util/string_util.h"

namespace rmv
{
    ResourceItemModel::ResourceItemModel(QObject* parent)
        : QAbstractItemModel(parent)
        , num_rows_(0)
        , num_columns_(0)
    {
    }

    ResourceItemModel::~ResourceItemModel()
    {
    }

    void ResourceItemModel::SetRowCount(int rows)
    {
        num_rows_ = rows;
        cache_.clear();
    }

    void ResourceItemModel::SetColumnCount(int columns)
    {
        num_columns_ = columns;
    }

    void ResourceItemModel::Initialize(ScaledTableView* resource_table, bool compare_visible)
    {
        resource_table->horizontalHeader()->setSectionsClickable(true);

        // Set default column widths wide enough to show table contents.
        resource_table->SetColumnPadding(0);
        resource_table->SetColumnWidthEms(kResourceColumnCompareId, 8);
        resource_table->SetColumnWidthEms(kResourceColumnName, 20);
        resource_table->SetColumnWidthEms(kResourceColumnVirtualAddress, 11);
        resource_table->SetColumnWidthEms(kResourceColumnSize, 8);
        resource_table->SetColumnWidthEms(kResourceColumnPreferredHeap, 11);
        resource_table->SetColumnWidthEms(kResourceColumnMappedInvisible, 13);
        resource_table->SetColumnWidthEms(kResourceColumnMappedLocal, 11);
        resource_table->SetColumnWidthEms(kResourceColumnMappedHost, 11);
        resource_table->SetColumnWidthEms(kResourceColumnMappedNone, 8);
        resource_table->SetColumnWidthEms(kResourceColumnUsage, 10);
        resource_table->SetColumnWidthEms(kResourceColumnAllocationIdInternal, 10);
        resource_table->SetColumnWidthEms(kResourceColumnGlobalId, 10);

        // Allow users to resize columns if desired.
        resource_table->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::Interactive);

        if (!compare_visible)
        {
            resource_table->hideColumn(kResourceColumnCompareId);
        }

        // Hide columns used for proxy models.
        resource_table->hideColumn(kResourceColumnAllocationIdInternal);
        resource_table->hideColumn(kResourceColumnGlobalId);
    }

    void ResourceItemModel::AddResource(const RmtDataSnapshot* snapshot, const RmtResource* resource, SnapshotCompareId compare_id)
    {
        uint64_t memory_segment_histogram[kRmtResourceBackingStorageCount] = {0};

        RmtResourceGetBackingStorageHistogram(snapshot, resource, memory_segment_histogram);

        uint64_t total_memory_mapped = 0;
        for (int32_t currentMemoryHistogramIndex = 0; currentMemoryHistogramIndex < kRmtResourceBackingStorageCount; ++currentMemoryHistogramIndex)
        {
            total_memory_mapped += memory_segment_histogram[currentMemoryHistogramIndex];
        }

        const char  buffer[RMT_MAXIMUM_NAME_LENGTH] = " - ";
        const char* buf_ptr                         = &buffer[0];
        RmtResourceGetName(resource, RMT_MAXIMUM_NAME_LENGTH, (char**)&buf_ptr);

        DataCache cache;
        cache.resource        = resource;
        cache.compare_id      = compare_id;
        cache.resource_name   = QString(buffer);
        cache.local_bytes     = 0;
        cache.invisible_bytes = 0;
        cache.host_bytes      = 0;
        cache.unmapped_bytes  = 0;
        if (total_memory_mapped > 0)
        {
            cache.local_bytes     = memory_segment_histogram[kRmtHeapTypeLocal];
            cache.invisible_bytes = memory_segment_histogram[kRmtHeapTypeInvisible];
            cache.host_bytes      = memory_segment_histogram[kRmtHeapTypeSystem];
            cache.unmapped_bytes  = memory_segment_histogram[kRmtResourceBackingStorageUnmapped];
        }
        cache_.push_back(cache);
    }

    QVariant ResourceItemModel::data(const QModelIndex& index, int role) const
    {
        if (!index.isValid())
        {
            return QVariant();
        }

        int                row      = index.row();
        const RmtResource* resource = cache_[row].resource;
        if (resource == nullptr)
        {
            return QVariant();
        }

        if (role == Qt::DisplayRole)
        {
            switch (index.column())
            {
            case kResourceColumnCompareId:
                return QString::number(cache_[row].compare_id);
            case kResourceColumnName:
                return cache_[row].resource_name;
            case kResourceColumnVirtualAddress:
                return rmv::string_util::LocalizedValueAddress(RmtResourceGetVirtualAddress(resource));
            case kResourceColumnSize:
                return rmv::string_util::LocalizedValueMemory(resource->size_in_bytes, false, false);
            case kResourceColumnMappedInvisible:
                return rmv::string_util::LocalizedValueMemory(cache_[row].invisible_bytes, false, false);
            case kResourceColumnMappedLocal:
                return rmv::string_util::LocalizedValueMemory(cache_[row].local_bytes, false, false);
            case kResourceColumnMappedHost:
                return rmv::string_util::LocalizedValueMemory(cache_[row].host_bytes, false, false);
            case kResourceColumnMappedNone:
                return rmv::string_util::LocalizedValueMemory(cache_[row].unmapped_bytes, false, false);
            case kResourceColumnPreferredHeap:
                return RmtResourceGetHeapTypeName(resource);
            case kResourceColumnUsage:
            {
                const RmtResourceUsageType resource_usage_type = RmtResourceGetUsageType(resource);
                return rmv::string_util::GetResourceUsageString(resource_usage_type);
            }
            case kResourceColumnAllocationIdInternal:
            {
                if (resource->bound_allocation != nullptr)
                {
                    return QString::number(resource->bound_allocation->guid);
                }
                else
                {
                    return RmtResourceGetHeapTypeName(resource);
                }
            }
            case kResourceColumnGlobalId:
                return QString::number(resource->identifier);

            default:
                break;
            }
        }
        else if (role == Qt::UserRole)
        {
            switch (index.column())
            {
            case kResourceColumnCompareId:
                return QVariant::fromValue<int>(cache_[row].compare_id);
            case kResourceColumnName:
                return QVariant::fromValue<qulonglong>(resource->identifier);
            case kResourceColumnVirtualAddress:
                return QVariant::fromValue<qulonglong>(RmtResourceGetVirtualAddress(resource));
            case kResourceColumnSize:
                return QVariant::fromValue<qulonglong>(resource->size_in_bytes);
            case kResourceColumnMappedInvisible:
                return QVariant::fromValue<qulonglong>(cache_[row].invisible_bytes);
            case kResourceColumnMappedLocal:
                return QVariant::fromValue<qulonglong>(cache_[row].local_bytes);
            case kResourceColumnMappedHost:
                return QVariant::fromValue<qulonglong>(cache_[row].host_bytes);
            case kResourceColumnMappedNone:
                return QVariant::fromValue<qulonglong>(cache_[row].unmapped_bytes);
            case kResourceColumnGlobalId:
                return QVariant::fromValue<qulonglong>(resource->identifier);

            default:
                break;
            }
        }
        else if (role == Qt::ToolTipRole)
        {
            switch (index.column())
            {
            case kResourceColumnSize:
                return rmv::string_util::LocalizedValueBytes(resource->size_in_bytes);
            case kResourceColumnMappedInvisible:
                return rmv::string_util::LocalizedValueBytes(cache_[row].invisible_bytes);
            case kResourceColumnMappedLocal:
                return rmv::string_util::LocalizedValueBytes(cache_[row].local_bytes);
            case kResourceColumnMappedHost:
                return rmv::string_util::LocalizedValueBytes(cache_[row].host_bytes);
            case kResourceColumnMappedNone:
                return rmv::string_util::LocalizedValueBytes(cache_[row].unmapped_bytes);

            default:
                break;
            }
        }

        return QVariant();
    }

    Qt::ItemFlags ResourceItemModel::flags(const QModelIndex& index) const
    {
        return QAbstractItemModel::flags(index);
    }

    QVariant ResourceItemModel::headerData(int section, Qt::Orientation orientation, int role) const
    {
        if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        {
            switch (section)
            {
            case kResourceColumnCompareId:
                return "Compare ID";
            case kResourceColumnName:
                return "Name";
            case kResourceColumnVirtualAddress:
                return "Virtual address";
            case kResourceColumnSize:
                return "Size";
            case kResourceColumnPreferredHeap:
                return "Preferred heap";
            case kResourceColumnMappedLocal:
                return "Committed local";
            case kResourceColumnMappedInvisible:
                return "Committed invisible";
            case kResourceColumnMappedHost:
                return "Committed host";
            case kResourceColumnMappedNone:
                return "Unmapped";
            case kResourceColumnUsage:
                return "Usage";
            default:
                break;
            }
        }

        return QAbstractItemModel::headerData(section, orientation, role);
    }

    QModelIndex ResourceItemModel::index(int row, int column, const QModelIndex& parent) const
    {
        if (!hasIndex(row, column, parent))
        {
            return QModelIndex();
        }

        return createIndex(row, column);
    }

    QModelIndex ResourceItemModel::parent(const QModelIndex& index) const
    {
        Q_UNUSED(index);
        return QModelIndex();
    }

    int ResourceItemModel::rowCount(const QModelIndex& parent) const
    {
        Q_UNUSED(parent);
        return num_rows_;
    }

    int ResourceItemModel::columnCount(const QModelIndex& parent) const
    {
        Q_UNUSED(parent);
        return num_columns_;
    }
}  // namespace rmv