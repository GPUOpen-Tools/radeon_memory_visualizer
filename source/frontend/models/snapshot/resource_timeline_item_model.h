//=============================================================================
// Copyright (c) 2020-2021 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Header for a resource timeline item model.
///
/// Used for the resource timeline table in the resource details pane.
///
//=============================================================================

#ifndef RMV_MODELS_SNAPSHOT_RESOURCE_TIMELINE_ITEM_MODEL_H_
#define RMV_MODELS_SNAPSHOT_RESOURCE_TIMELINE_ITEM_MODEL_H_

#include <QAbstractItemModel>

#include "rmt_resource_history.h"

namespace rmv
{
    /// @brief Column Id's for fields in the resource history table in the resource details pane.
    enum ResourceHistoryColumn
    {
        kResourceHistoryLegend,
        kResourceHistoryEvent,
        kResourceHistoryTime,
        kResourceHistoryDetails,

        kResourceHistoryCount
    };

    /// @brief Container class that holds model data for the resource timeline table in the resource details pane.
    class ResourceTimelineItemModel : public QAbstractItemModel
    {
    public:
        /// @brief Constructor.
        ///
        /// @param [in] parent The parent widget.
        explicit ResourceTimelineItemModel(QObject* parent = nullptr);

        /// @brief Destructor.
        ~ResourceTimelineItemModel();

        /// @brief Set the number of rows in the table.
        ///
        /// @param [in] rows The number of rows required.
        void SetRowCount(int rows);

        /// @brief Set the number of columns in the table.
        ///
        /// @param [in] columns The number of columns required.
        void SetColumnCount(int columns);

        /// @brief Set the snapshot parameters.
        ///
        /// @param [in] snapshot_table_index The table index when the snapshot occurred.
        /// @param [in] snapshot_timestamp   The time when the snapshot was taken.
        /// @param [in] resource_history     Pointer to the generated resource history data.
        void SetSnapshotParameters(int32_t snapshot_table_index, uint64_t snapshot_timestamp, RmtResourceHistory* resource_history);

        // QAbstractItemModel overrides. See Qt documentation for parameter and return values
        virtual QVariant      data(const QModelIndex& index, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
        virtual Qt::ItemFlags flags(const QModelIndex& index) const Q_DECL_OVERRIDE;
        virtual QVariant      headerData(int section, Qt::Orientation orientation, int role) const Q_DECL_OVERRIDE;
        virtual QModelIndex   index(int row, int column, const QModelIndex& parent = QModelIndex()) const Q_DECL_OVERRIDE;
        virtual QModelIndex   parent(const QModelIndex& index) const Q_DECL_OVERRIDE;
        virtual int           rowCount(const QModelIndex& parent = QModelIndex()) const Q_DECL_OVERRIDE;
        virtual int           columnCount(const QModelIndex& parent = QModelIndex()) const Q_DECL_OVERRIDE;

    private:
        /// @brief Get the text based on the event type.
        ///
        /// @param [in] event_type The type of resource event.
        QString GetTextFromEventType(RmtResourceHistoryEventType event_type) const;

        int                 num_rows_;              ///< The number of rows in the table.
        int                 num_columns_;           ///< The number of columns in the table.
        int32_t             snapshot_table_index_;  ///< The table index when the snapshot was taken.
        uint64_t            snapshot_timestamp_;    ///< The time the snapshot was taken.
        RmtResourceHistory* resource_history_;      ///< Pointer to generated resource history.
    };
}  // namespace rmv

#endif  // RMV_MODELS_SNAPSHOT_RESOURCE_TIMELINE_ITEM_MODEL_H_
