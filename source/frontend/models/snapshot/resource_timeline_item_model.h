//=============================================================================
/// Copyright (c) 2020 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Header for a resource timeline item model. Used for the resource
/// timeline table in the resource details pane
//=============================================================================

#ifndef RMV_MODELS_SNAPSHOT_RESOURCE_TIMELINE_ITEM_MODEL_H_
#define RMV_MODELS_SNAPSHOT_RESOURCE_TIMELINE_ITEM_MODEL_H_

#include <QAbstractItemModel>

#include "rmt_resource_history.h"

/// Column Id's for fields in the resource history table in the resource details pane.
enum ResourceHistoryColumn
{
    kResourceHistoryLegend,
    kResourceHistoryEvent,
    kResourceHistoryTime,
    kResourceHistoryDetails,

    kResourceHistoryCount
};

namespace rmv
{
    /// Container class that holds model data for the resource timeline table
    /// in the resource details pane.
    class ResourceTimelineItemModel : public QAbstractItemModel
    {
    public:
        /// Constructor.
        explicit ResourceTimelineItemModel(QObject* parent = nullptr);

        /// Destructor.
        ~ResourceTimelineItemModel();

        /// Set the number of rows in the table.
        /// \param rows The number of rows required.
        void SetRowCount(int rows);

        /// Set the number of columns in the table.
        /// \param columns The number of columns required.
        void SetColumnCount(int columns);

        /// Set the snapshot parameters.
        /// \param snapshot_table_index The table index when the snapshot occurred.
        /// \param snapshot_timestamp The time when the snapshot was taken.
        /// \param resource_history Pointer to the generated reosurce history data.
        void SetSnapshotParameters(int32_t snapshot_table_index, uint64_t snapshot_timestamp, RmtResourceHistory* resource_history);

        // QAbstractItemModel overrides. See Qt documentation for parameter and return values
        QVariant      data(const QModelIndex& index, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
        Qt::ItemFlags flags(const QModelIndex& index) const Q_DECL_OVERRIDE;
        QVariant      headerData(int section, Qt::Orientation orientation, int role) const Q_DECL_OVERRIDE;
        QModelIndex   index(int row, int column, const QModelIndex& parent = QModelIndex()) const Q_DECL_OVERRIDE;
        QModelIndex   parent(const QModelIndex& index) const Q_DECL_OVERRIDE;
        int           rowCount(const QModelIndex& parent = QModelIndex()) const Q_DECL_OVERRIDE;
        int           columnCount(const QModelIndex& parent = QModelIndex()) const Q_DECL_OVERRIDE;

    private:
        /// Get the text based on the event type.
        /// \param event_type The type of resource event.
        QString GetTextFromEventType(RmtResourceHistoryEventType event_type) const;

        int                 num_rows_;              ///< The number of rows in the table.
        int                 num_columns_;           ///< The number of columns in the table.
        int32_t             snapshot_table_index_;  ///< The table index when the snapshot was taken.
        uint64_t            snapshot_timestamp_;    ///< The time the snapshot was taken.
        RmtResourceHistory* resource_history_;      ///< Pointer to generated resource history.
    };
}  // namespace rmv

#endif  // RMV_MODELS_SNAPSHOT_RESOURCE_TIMELINE_ITEM_MODEL_H_
