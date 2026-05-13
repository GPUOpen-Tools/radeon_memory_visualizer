//=============================================================================
// Copyright (c) 2020-2026 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Header for a resource item model.
///
/// Used for the resource list tables.
///
//=============================================================================

#ifndef RMV_MODELS_RESOURCE_ITEM_MODEL_H_
#define RMV_MODELS_RESOURCE_ITEM_MODEL_H_

#include <QAbstractItemModel>
#include <QTextStream>

#include "qt_common/custom_widgets/scaled_table_view.h"

#include "rmt_data_snapshot.h"
#include "rmt_resource_list.h"

#include "models/proxy_models/table_proxy_model.h"

namespace rmv
{
    /// @brief Column Id's for the fields in the resource tables.
    enum ResourceColumn
    {
        kResourceColumnCompareId,
        kResourceColumnName,
        kResourceColumnVirtualAddress,
        kResourceColumnUsage,
        kResourceColumnDimension,
        kResourceColumnMipLevel,
        kResourceColumnFormat,
        kResourceColumnSize,
        kResourceColumnPreferredHeap,
        kResourceColumnMappedInvisible,
        kResourceColumnMappedLocal,
        kResourceColumnMappedHost,
        kResourceColumnMappedNone,

        // Hidden, these columns are used as proxies for sorting by other columns.
        kResourceColumnAllocationIdInternal,
        kResourceColumnGlobalId,

        kResourceColumnCount,
    };

    /// @brief Snapshot compare Id types used in the memory leak pane.
    enum SnapshotCompareId
    {
        kSnapshotCompareIdUndefined = 0x0,
        kSnapshotCompareIdCommon    = 0x1,
        kSnapshotCompareIdOpen      = 0x2,
        kSnapshotCompareIdCompared  = 0x4,
    };

    /// @brief A class to handle the model data associated with a resource table.
    class ResourceItemModel : public QAbstractItemModel
    {
    public:
        /// @brief Constructor.
        explicit ResourceItemModel(QObject* parent = nullptr);

        /// @brief Destructor.
        virtual ~ResourceItemModel();

        /// @brief Set the number of rows in the table.
        ///
        /// @param [in] rows The number of rows required.
        void SetRowCount(int rows);

        /// @brief Set the number of columns in the table.
        ///
        /// @param [in] columns The number of columns required.
        void SetColumnCount(int columns);

        /// @brief Initialize the resource list table.
        ///
        /// An instance of this table is present in the resource list, allocation
        /// explorer and memory leak panes.
        ///
        /// @param [in] resource_table  The table to initialize.
        /// @param [in] compare_visible If false, hide the compare column.
        void Initialize(ScaledTableView* resource_table, bool compare_visible);

        /// @brief Add a resource to the table.
        ///
        /// @param [in] snapshot   The snapshot where the resource data is located.
        /// @param [in] resource   The resource to add.
        /// @param [in] compare_id The ID when used to compare 2 resources.
        void AddResource(const RmtDataSnapshot* snapshot, const RmtResource* resource, SnapshotCompareId compare_id);

        /// @brief Dump the resource table to disk.
        ///
        /// Determines the resource usage and dumps out the relevent information.
        ///
        /// @param [in] file_name    The name of the file to dump the output to.
        /// @param [in] proxy_model  The proxy model.
        /// @param [in] base_name    For the memory leak pane, the name of the base snapshot. Set to nullptr for other panes.
        /// @param [in] diff_name    For the memory leak pane, the name of the diff snapshot. Set to nullptr for other panes.
        void DumpResourceTable(QWidget* parent, TableProxyModel* proxy_model, const char* base_name, const char* diff_name) const;

        // QAbstractItemModel overrides. See Qt documentation for parameter and return values
        virtual QVariant      data(const QModelIndex& index, int role) const Q_DECL_OVERRIDE;
        virtual Qt::ItemFlags flags(const QModelIndex& index) const Q_DECL_OVERRIDE;
        virtual QVariant      headerData(int section, Qt::Orientation orientation, int role) const Q_DECL_OVERRIDE;
        virtual QModelIndex   index(int row, int column, const QModelIndex& parent) const Q_DECL_OVERRIDE;
        virtual QModelIndex   parent(const QModelIndex& index) const Q_DECL_OVERRIDE;
        virtual int           rowCount(const QModelIndex& parent = QModelIndex()) const Q_DECL_OVERRIDE;
        virtual int           columnCount(const QModelIndex& parent = QModelIndex()) const Q_DECL_OVERRIDE;

    private:
        /// @brief Data from the backend that needs caching for speed.
        struct DataCache
        {
            DataCache()
                : resource(nullptr)
                , local_bytes(0)
                , invisible_bytes(0)
                , host_bytes(0)
                , unmapped_bytes(0)
                , compare_id(kSnapshotCompareIdUndefined)
            {
            }

            const RmtResource* resource;         ///< The resource.
            double             local_bytes;      ///< Amount of local memory.
            double             invisible_bytes;  ///< Amount of invisible memory.
            double             host_bytes;       ///< Amount of host memory.
            double             unmapped_bytes;   ///< Amount of unmapped memory.
            SnapshotCompareId  compare_id;       ///< The comparison id (if any).
            QString            resource_name;    ///< The resource name.
        };

        /// @brief The sort comparator for sorting the resource list.
        ///
        /// Currently sort by resource usage.
        ///
        /// @param [in] resource_a The first resource to compare.
        /// @param [in] resource_b The second resource to compare.
        ///
        /// return true if resource_a > resource_b, false otherwise.
        static bool SortComparator(const DataCache* resource_a, const DataCache* resource_b);

        /// @brief Dump out the info common to all resources.
        ///
        /// @param [in] stream         The stream to write the resource info to.
        /// @param [in] resource_info  The resource to dump out. If resource is nullptr, write the header info.
        /// @param [in] base_name      For the memory leak pane, the name of the base snapshot. Set to nullptr for other panes.
        /// @param [in] diff_name      For the memory leak pane, the name of the diff snapshot. Set to nullptr for other panes.
        void DumpCommonInfo(QTextStream& stream, const DataCache* resource_info, const char* base_name, const char* diff_name) const;

        /// @brief Dump out the buffer info for buffer resources.
        ///
        /// @param [in] stream         The stream to write the resource info to.
        /// @param [in] resource_info  The resource to dump out. If resource is nullptr, write the header info.
        void DumpBufferInfo(QTextStream& stream, const DataCache* resource_info) const;

        /// @brief Dump out the image info for image resources.
        ///
        /// @param [in] stream         The stream to write the resource info to.
        /// @param [in] resource_info  The resource to dump out. If resource is nullptr, write the header info.
        void DumpImageInfo(QTextStream& stream, const DataCache* resource_info) const;

        /// @brief Top level dump function.
        ///
        /// Determines the resource usage and dumps out the relevant information.
        ///
        /// @param [in] stream         The stream to write the resource info to.
        /// @param [in] resource_info  The resource to dump out. If resource is nullptr, write the header info.
        /// @param [in] usage_type     The resource usage type.
        /// @param [in] base_name      For the memory leak pane, the name of the base snapshot. Set to nullptr for other panes.
        /// @param [in] diff_name      For the memory leak pane, the name of the diff snapshot. Set to nullptr for other panes.
        void DumpInfo(QTextStream& stream, const DataCache* resource_info, RmtResourceUsageType usage_type, const char* base_name, const char* diff_name) const;

        int                    num_rows_;     ///< The number of rows in the table.
        int                    num_columns_;  ///< The number of columns in the table.
        std::vector<DataCache> cache_;        ///< Cached data from the backend.
    };
}  // namespace rmv

#endif  // RMV_MODELS_RESOURCE_ITEM_MODEL_H_
