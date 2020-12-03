//=============================================================================
/// Copyright (c) 2020 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Header for a resource item model. Used for the resource list tables
//=============================================================================

#ifndef RMV_MODELS_RESOURCE_ITEM_MODEL_H_
#define RMV_MODELS_RESOURCE_ITEM_MODEL_H_

#include <QAbstractItemModel>

#include "qt_common/custom_widgets/scaled_table_view.h"

#include "rmt_data_snapshot.h"
#include "rmt_resource_list.h"

/// Column Id's for the fields in the resource tables.
enum ResourceColumn
{
    kResourceColumnCompareId,
    kResourceColumnName,
    kResourceColumnVirtualAddress,
    kResourceColumnSize,
    kResourceColumnPreferredHeap,
    kResourceColumnMappedInvisible,
    kResourceColumnMappedLocal,
    kResourceColumnMappedHost,
    kResourceColumnMappedNone,
    kResourceColumnUsage,

    // Hidden, these columns are used as proxies for sorting by other columns.
    kResourceColumnAllocationIdInternal,
    kResourceColumnGlobalId,

    kResourceColumnCount,
};

/// Snapshot compare Id types used in the memory leak pane.
enum SnapshotCompareId
{
    kSnapshotCompareIdUndefined = 0x0,
    kSnapshotCompareIdCommon    = 0x1,
    kSnapshotCompareIdOpen      = 0x2,
    kSnapshotCompareIdCompared  = 0x4,
};

namespace rmv
{
    class ResourceItemModel : public QAbstractItemModel
    {
    public:
        /// Constructor.
        explicit ResourceItemModel(QObject* parent = nullptr);

        /// Destructor.
        ~ResourceItemModel();

        /// Set the number of rows in the table.
        /// \param rows The number of rows required.
        void SetRowCount(int rows);

        /// Set the number of columns in the table.
        /// \param columns The number of columns required.
        void SetColumnCount(int columns);

        /// Initialize the resource list table. An instance of this table
        /// is present in the resource list, allocation explorer and memory leak panes.
        /// \param resource_table The table to initialize.
        /// \param compare_visible If false, hide the compare column.
        void Initialize(ScaledTableView* resource_table, bool compare_visible);

        /// Add a resource to the table.
        /// \param snapshot The snapshot where the resource data is located.
        /// \param resource The resource to add.
        /// \param compare_id The ID when used to compare 2 resources.
        void AddResource(const RmtDataSnapshot* snapshot, const RmtResource* resource, SnapshotCompareId compare_id);

        // QAbstractItemModel overrides. See Qt documentation for parameter and return values
        virtual QVariant      data(const QModelIndex& index, int role) const Q_DECL_OVERRIDE;
        virtual Qt::ItemFlags flags(const QModelIndex& index) const Q_DECL_OVERRIDE;
        virtual QVariant      headerData(int section, Qt::Orientation orientation, int role) const Q_DECL_OVERRIDE;
        virtual QModelIndex   index(int row, int column, const QModelIndex& parent) const Q_DECL_OVERRIDE;
        virtual QModelIndex   parent(const QModelIndex& index) const Q_DECL_OVERRIDE;
        virtual int           rowCount(const QModelIndex& parent = QModelIndex()) const Q_DECL_OVERRIDE;
        virtual int           columnCount(const QModelIndex& parent = QModelIndex()) const Q_DECL_OVERRIDE;

    private:
        /// Data from the backend that needs caching for speed.
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

        int                    num_rows_;     ///< The number of rows in the table.
        int                    num_columns_;  ///< The number of columns in the table.
        std::vector<DataCache> cache_;        ///< Cached data from the backend.
    };
}  // namespace rmv

#endif  // RMV_MODELS_RESOURCE_ITEM_MODEL_H_
