//=============================================================================
// Copyright (c) 2020-2023 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Implementation for an allocation item model.
///
/// Used for the allocation list tables.
///
//=============================================================================

#include "models/allocation_item_model.h"

#include "rmt_assert.h"
#include "rmt_print.h"
#include "rmt_util.h"

#include "managers/trace_manager.h"
#include "models/snapshot/allocation_explorer_model.h"
#include "util/string_util.h"

namespace rmv
{
    AllocationItemModel::AllocationItemModel(QObject* parent)
        : QAbstractItemModel(parent)
        , num_rows_(0)
        , num_columns_(0)
    {
    }

    AllocationItemModel::~AllocationItemModel()
    {
    }

    void AllocationItemModel::SetRowCount(int rows)
    {
        num_rows_ = rows;
        cache_.clear();
    }

    void AllocationItemModel::SetColumnCount(int columns)
    {
        num_columns_ = columns;
    }

    void AllocationItemModel::AddAllocation(const RmtDataSnapshot* snapshot, const RmtVirtualAllocation* virtual_allocation)
    {
        uint64_t histogram_total                            = 0;
        uint64_t histogram[kRmtResourceBackingStorageCount] = {0};
        RmtVirtualAllocationGetBackingStorageHistogram(snapshot, virtual_allocation, histogram, &histogram_total);

        DataCache cache;
        cache.virtual_allocation = virtual_allocation;
        cache.allocation_size    = RmtVirtualAllocationGetSizeInBytes(virtual_allocation);
        cache.bound_size         = RmtVirtualAllocationGetTotalResourceMemoryInBytes(snapshot, virtual_allocation);
        if (cache.allocation_size >= cache.bound_size)
        {
            cache.unbound_size = RmtVirtualAllocationGetTotalUnboundSpaceInAllocation(snapshot, virtual_allocation);
        }
        else
        {
            cache.unbound_size = 0;
        }
        cache.avg_resource_size     = RmtVirtualAllocationGetAverageResourceSizeInBytes(snapshot, virtual_allocation);
        cache.std_dev_resource_size = RmtVirtualAllocationGetResourceStandardDeviationInBytes(snapshot, virtual_allocation);
        cache.local_bytes           = histogram[kRmtHeapTypeLocal];
        cache.invisible_bytes       = histogram[kRmtHeapTypeInvisible];
        cache.host_bytes            = histogram[kRmtHeapTypeSystem];
        cache.unmapped_bytes        = histogram[kRmtResourceBackingStorageUnmapped];

        cache_.push_back(cache);
    }

    QVariant AllocationItemModel::data(const QModelIndex& index, int role) const
    {
        if (!index.isValid())
        {
            return QVariant();
        }

        int row = index.row();

        const DataCache& cache = cache_[row];
        if (cache.virtual_allocation == nullptr)
        {
            return QVariant();
        }

        if (role == Qt::DisplayRole)
        {
            switch (index.column())
            {
            case kVirtualAllocationColumnId:
                return QString::number(cache.virtual_allocation->base_address);
            case kVirtualAllocationColumnAllocationSize:
                return rmv::string_util::LocalizedValueMemory(cache.allocation_size, false, false);
            case kVirtualAllocationColumnBound:
                return rmv::string_util::LocalizedValueMemory(cache.bound_size, false, false);
            case kVirtualAllocationColumnUnbound:
                return rmv::string_util::LocalizedValueMemory(cache.unbound_size, false, false);
            case kVirtualAllocationColumnAverageResourceSize:
                return rmv::string_util::LocalizedValueMemory(cache.avg_resource_size, false, false);
            case kVirtualAllocationColumnResourceSizeStdDev:
                return rmv::string_util::LocalizedValueMemory(cache.std_dev_resource_size, false, false);
            case kVirtualAllocationColumnResourceCount:
                return rmv::string_util::LocalizedValue(cache.virtual_allocation->resource_count);
            case kVirtualAllocationColumnPreferredHeapName:
                return RmtGetHeapTypeNameFromHeapType(cache.virtual_allocation->heap_preferences[0]);
            case kVirtualAllocationColumnInvisiblePercentage:
                return rmv::string_util::LocalizedValueMemory(cache.invisible_bytes, false, false);
            case kVirtualAllocationColumnLocalPercentage:
                return rmv::string_util::LocalizedValueMemory(cache.local_bytes, false, false);
            case kVirtualAllocationColumnSystemPercentage:
                return rmv::string_util::LocalizedValueMemory(cache.host_bytes, false, false);
            case kVirtualAllocationColumnUnmappedPercentage:
                return rmv::string_util::LocalizedValueMemory(cache.unmapped_bytes, false, false);
            default:
                break;
            }
        }
        else if (role == Qt::UserRole)
        {
            switch (index.column())
            {
            case kVirtualAllocationColumnId:
                return QVariant::fromValue<qulonglong>((qulonglong)cache.virtual_allocation);
            case kVirtualAllocationColumnAllocationSize:
                return QVariant::fromValue<qulonglong>(cache.allocation_size);
            case kVirtualAllocationColumnBound:
                return QVariant::fromValue<qulonglong>(cache.bound_size);
            case kVirtualAllocationColumnUnbound:
                return QVariant::fromValue<qulonglong>(cache.unbound_size);
            case kVirtualAllocationColumnAverageResourceSize:
                return QVariant::fromValue<qulonglong>(cache.avg_resource_size);
            case kVirtualAllocationColumnResourceSizeStdDev:
                return QVariant::fromValue<qulonglong>(cache.std_dev_resource_size);
            case kVirtualAllocationColumnResourceCount:
                return QVariant::fromValue<int>(cache.virtual_allocation->resource_count);
            case kVirtualAllocationColumnInvisiblePercentage:
                return QVariant::fromValue<qulonglong>(cache.invisible_bytes);
            case kVirtualAllocationColumnLocalPercentage:
                return QVariant::fromValue<qulonglong>(cache.local_bytes);
            case kVirtualAllocationColumnSystemPercentage:
                return QVariant::fromValue<qulonglong>(cache.host_bytes);
            case kVirtualAllocationColumnUnmappedPercentage:
                return QVariant::fromValue<qulonglong>(cache.unmapped_bytes);

            default:
                break;
            }
        }
        else if (role == Qt::ToolTipRole)
        {
            switch (index.column())
            {
            case kVirtualAllocationColumnAllocationSize:
                return rmv::string_util::LocalizedValueBytes(cache.allocation_size);
            case kVirtualAllocationColumnBound:
                return rmv::string_util::LocalizedValueBytes(cache.bound_size);
            case kVirtualAllocationColumnUnbound:
                return rmv::string_util::LocalizedValueBytes(cache.unbound_size);
            case kVirtualAllocationColumnAverageResourceSize:
                return rmv::string_util::LocalizedValueBytes(cache.avg_resource_size);
            case kVirtualAllocationColumnResourceSizeStdDev:
                return rmv::string_util::LocalizedValueBytes(cache.std_dev_resource_size);
            case kVirtualAllocationColumnInvisiblePercentage:
                return rmv::string_util::LocalizedValueBytes(cache.invisible_bytes);
            case kVirtualAllocationColumnLocalPercentage:
                return rmv::string_util::LocalizedValueBytes(cache.local_bytes);
            case kVirtualAllocationColumnSystemPercentage:
                return rmv::string_util::LocalizedValueBytes(cache.host_bytes);
            case kVirtualAllocationColumnUnmappedPercentage:
                return rmv::string_util::LocalizedValueBytes(cache.unmapped_bytes);

            default:
                break;
            }
        }

        return QVariant();
    }

    Qt::ItemFlags AllocationItemModel::flags(const QModelIndex& index) const
    {
        return QAbstractItemModel::flags(index);
    }

    QVariant AllocationItemModel::headerData(int section, Qt::Orientation orientation, int role) const
    {
        if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        {
            switch (section)
            {
            case kVirtualAllocationColumnId:
                return "Allocation";
            case kVirtualAllocationColumnAllocationSize:
                return "Allocation size";
            case kVirtualAllocationColumnBound:
                return "Bound";
            case kVirtualAllocationColumnUnbound:
                return "Unbound";
            case kVirtualAllocationColumnAverageResourceSize:
                return "Avg. resource size";
            case kVirtualAllocationColumnResourceSizeStdDev:
                return "Resource size std. dev.";
            case kVirtualAllocationColumnResourceCount:
                return "Resource count";
            case kVirtualAllocationColumnPreferredHeapName:
                return "Preferred heap";
            case kVirtualAllocationColumnInvisiblePercentage:
                return "Committed invisible";
            case kVirtualAllocationColumnLocalPercentage:
                return "Committed local";
            case kVirtualAllocationColumnSystemPercentage:
                return "Committed host";
            case kVirtualAllocationColumnUnmappedPercentage:
                return "Unmapped";
            default:
                break;
            }
        }

        return QAbstractItemModel::headerData(section, orientation, role);
    }

    QModelIndex AllocationItemModel::index(int row, int column, const QModelIndex& parent) const
    {
        if (!hasIndex(row, column, parent))
        {
            return QModelIndex();
        }

        return createIndex(row, column);
    }

    QModelIndex AllocationItemModel::parent(const QModelIndex& index) const
    {
        Q_UNUSED(index);
        return QModelIndex();
    }

    int AllocationItemModel::rowCount(const QModelIndex& parent) const
    {
        Q_UNUSED(parent);
        return num_rows_;
    }

    int AllocationItemModel::columnCount(const QModelIndex& parent) const
    {
        Q_UNUSED(parent);
        return num_columns_;
    }
}  // namespace rmv