//=============================================================================
// Copyright (c) 2020-2026 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Implementation for a resource item model.
///
/// Used for the resource list tables.
///
//=============================================================================

#include "models/resource_item_model.h"

#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QTextStream>

#include "rmt_assert.h"
#include "rmt_print.h"
#include "rmt_util.h"

#include "managers/snapshot_manager.h"
#include "models/proxy_models/table_proxy_model.h"
#include "settings/rmv_settings.h"
#include "util/rmv_util.h"
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
        resource_table->SetColumnWidthEms(kResourceColumnDimension, 11);
        resource_table->SetColumnWidthEms(kResourceColumnMipLevel, 6);
        resource_table->SetColumnWidthEms(kResourceColumnFormat, 8);
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
            case kResourceColumnDimension:
                return resource->resource_type == kRmtResourceTypeImage
                           ? QString::asprintf("%dx%dx%d", resource->image.dimension_x, resource->image.dimension_y, resource->image.dimension_z)
                           : QString("-");
            case kResourceColumnMipLevel:
                return resource->resource_type == kRmtResourceTypeImage ? QString::asprintf("%d", resource->image.mip_levels) : QString("-");
            case kResourceColumnFormat:
                return resource->resource_type == kRmtResourceTypeImage ? QString(RmtGetFormatNameFromFormat(resource->image.format.format)) : QString("-");
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
                return RmtGetResourceUsageTypeNameFromResourceUsageType(resource_usage_type);
            }
            case kResourceColumnAllocationIdInternal:
            {
                if (resource->bound_allocation != nullptr)
                {
                    return rmv_util::GetVirtualAllocationName(resource->bound_allocation);
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
            case kResourceColumnDimension:
                return resource->resource_type == kRmtResourceTypeImage
                           ? QList<QVariant>({resource->image.dimension_x, resource->image.dimension_y, resource->image.dimension_z})
                           : QList<QVariant>({0, 0, 0});
            case kResourceColumnMipLevel:
                return resource->resource_type == kRmtResourceTypeImage ? resource->image.mip_levels : 0;
            case kResourceColumnFormat:
                return resource->resource_type == kRmtResourceTypeImage ? QString(RmtGetFormatNameFromFormat(resource->image.format.format)) : QString("-");
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
            case kResourceColumnUsage:
            {
                const RmtResourceUsageType resource_usage_type = RmtResourceGetUsageType(resource);
                return QVariant::fromValue<int>(resource_usage_type);
            }
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
            case kResourceColumnName:
                return cache_[row].resource_name;
            case kResourceColumnDimension:
                return resource->resource_type == kRmtResourceTypeImage
                           ? QString::asprintf("%dx%dx%d", resource->image.dimension_x, resource->image.dimension_y, resource->image.dimension_z)
                           : QString("-");
            case kResourceColumnMipLevel:
                return resource->resource_type == kRmtResourceTypeImage ? QString::asprintf("%d", resource->image.mip_levels) : QString("-");
            case kResourceColumnFormat:
                return resource->resource_type == kRmtResourceTypeImage ? QString(RmtGetFormatNameFromFormat(resource->image.format.format)) : QString("-");
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
            case kResourceColumnDimension:
                return "Dimension";
            case kResourceColumnMipLevel:
                return "Mip Level";
            case kResourceColumnFormat:
                return "Format";
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

    bool ResourceItemModel::SortComparator(const DataCache* resource_a, const DataCache* resource_b)
    {
        auto usage_a = RmtResourceGetUsageType(resource_a->resource);
        auto usage_b = RmtResourceGetUsageType(resource_b->resource);
        return usage_a > usage_b;
    }

    void ResourceItemModel::DumpCommonInfo(QTextStream& stream, const DataCache* resource_info, const char* base_name, const char* diff_name) const
    {
        if (resource_info == nullptr)
        {
            if (base_name != nullptr && diff_name != nullptr)
            {
                stream << base_name << ", " << diff_name << ", ";
            }

            stream << "Name, Virtual address, Usage, Size, Preferred heap, Committed invisible, Committed local, Committed host, Unmapped";
        }
        else if (resource_info->resource != nullptr)
        {
            if (base_name != nullptr && diff_name != nullptr)
            {
                if (resource_info->compare_id == kSnapshotCompareIdCommon)
                {
                    stream << "*, *, ";
                }
                else if (resource_info->compare_id == kSnapshotCompareIdOpen)
                {
                    stream << "*, , ";
                }
                else if (resource_info->compare_id == kSnapshotCompareIdCompared)
                {
                    stream << " , *, ";
                }
                else
                {
                    stream << " , , ";
                }
            }

            char        name_buffer[1024] = {};
            const char* buf_ptr           = &name_buffer[0];
            RmtResourceGetName(resource_info->resource, 1024, (char**)&buf_ptr);
            const QString& usage_name = RmtGetResourceUsageTypeNameFromResourceUsageType(RmtResourceGetUsageType(resource_info->resource));

            stream << name_buffer;
            stream << QString(", 0x%1").arg(static_cast<unsigned long long>(resource_info->resource->address), 10, 16, QChar('0'));
            stream << ", " << usage_name.toLatin1().data();
            stream << ", \"" << rmv::string_util::LocalizedValueMemory(resource_info->resource->size_in_bytes, false, false).toLatin1().data() << "\"";

            stream << ", " << RmtResourceGetHeapTypeName(resource_info->resource);
            stream << ", \"" << rmv::string_util::LocalizedValueMemory(resource_info->invisible_bytes, false, false).toLatin1().data() << "\"";
            stream << ", \"" << rmv::string_util::LocalizedValueMemory(resource_info->local_bytes, false, false).toLatin1().data() << "\"";
            stream << ", \"" << rmv::string_util::LocalizedValueMemory(resource_info->host_bytes, false, false).toLatin1().data() << "\"";
            stream << ", \"" << rmv::string_util::LocalizedValueMemory(resource_info->unmapped_bytes, false, false).toLatin1().data() << "\"";
        }
    }

    void ResourceItemModel::DumpBufferInfo(QTextStream& stream, const DataCache* resource_info) const
    {
        if (resource_info == nullptr)
        {
            stream << ", Create flags, Usage flags";
        }
        else if (resource_info->resource != nullptr)
        {
            char flags_text[1024];

            RmtGetBufferCreationNameFromBufferCreationFlags(resource_info->resource->buffer.create_flags, flags_text, 1024);
            stream << ", " << flags_text;

            RmtGetBufferUsageNameFromBufferUsageFlags(resource_info->resource->buffer.usage_flags, flags_text, 1024);
            stream << ", " << flags_text;
        }
    }

    void ResourceItemModel::DumpImageInfo(QTextStream& stream, const DataCache* resource_info) const
    {
        if (resource_info == nullptr)
        {
            stream << ", Image Type, X, Y, Z, Format, Swizzle, Mip levels, Create flags, Usage flags";
        }
        else if (resource_info->resource != nullptr)
        {
            stream << ", " << RmtGetImageTypeNameFromImageType(resource_info->resource->image.image_type);
            stream << ", " << resource_info->resource->image.dimension_x;
            stream << ", " << resource_info->resource->image.dimension_y;
            stream << ", " << resource_info->resource->image.dimension_z;
            stream << ", " << RmtGetFormatNameFromFormat(resource_info->resource->image.format.format);

            char swizzle_pattern[8] = {};
            RmtGetSwizzlePatternFromImageFormat(&resource_info->resource->image.format, swizzle_pattern, sizeof(swizzle_pattern));
            stream << ", " << swizzle_pattern;
            stream << ", " << rmv::string_util::LocalizedValue(resource_info->resource->image.mip_levels).toLatin1().data();

            char flags_text[1024] = {};

            RmtGetImageCreationNameFromImageCreationFlags(resource_info->resource->image.create_flags, flags_text, 1024);
            stream << ", " << flags_text;

            RmtGetImageUsageNameFromImageUsageFlags(resource_info->resource->image.usage_flags, flags_text, 1024);
            stream << ", " << flags_text;
        }
    }

    void ResourceItemModel::DumpInfo(QTextStream&         stream,
                                     const DataCache*     resource_info,
                                     RmtResourceUsageType usage_type,
                                     const char*          base_name,
                                     const char*          diff_name) const
    {
        DumpCommonInfo(stream, resource_info, base_name, diff_name);

        switch (usage_type)
        {
        case kRmtResourceUsageTypeRenderTarget:
        case kRmtResourceUsageTypeTexture:
        case kRmtResourceUsageTypeDepthStencil:
            DumpImageInfo(stream, resource_info);
            break;
        case kRmtResourceUsageTypeBuffer:
            DumpBufferInfo(stream, resource_info);
            break;
        default:
            break;
        }
        stream << "\n";
    }

    void ResourceItemModel::DumpResourceTable(QWidget* parent, TableProxyModel* proxy_model, const char* base_name, const char* diff_name) const
    {
        QString   file_name = rmv::RMVSettings::Get().GetLastFileOpenLocation();
        QFileInfo file_info(file_name);
        QString   file_path = QFileDialog::getSaveFileName(parent, "Save resources", file_info.completeBaseName(), "CSV (*.csv);;All files (*)");

        if (file_path.isNull())
        {
            return;
        }

        QFile qt_file(file_path);
        if (qt_file.isOpen())
        {
            return;
        }

        if (!qt_file.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            return;
        }

        QTextStream stream(&qt_file);

        std::vector<const DataCache*> resource_vector;

        // The proxy model contains the data that's visible in the table.
        // Use this data to map back to the real data and only save what is visible in the table in the UI.
        // This will allow the user to filter out resources they don't care about before saving to disk.
        for (int row = 0; row < proxy_model->rowCount(); ++row)
        {
            QModelIndex proxy_index  = proxy_model->index(row, 0);
            QModelIndex source_index = proxy_model->mapToSource(proxy_index);
            int         source_row   = source_index.row();
            resource_vector.push_back(&cache_[source_row]);
        }

        // Run through the list and sort resources into separate arrays for the ones we're interested in.
        // Everything has flags and usage.
        auto last_usage_type = -1;
        std::stable_sort(resource_vector.begin(), resource_vector.end(), &ResourceItemModel::SortComparator);

        for (auto it : resource_vector)
        {
            // See if header needs updating.
            auto this_usage_type = RmtResourceGetUsageType(it->resource);
            if (this_usage_type != last_usage_type)
            {
                last_usage_type = this_usage_type;
                stream << "\n";
                DumpInfo(stream, nullptr, this_usage_type, base_name, diff_name);
            }

            DumpInfo(stream, it, this_usage_type, base_name, diff_name);
        }
        stream << "\n";

        qt_file.flush();
        qt_file.close();
    }

}  // namespace rmv
