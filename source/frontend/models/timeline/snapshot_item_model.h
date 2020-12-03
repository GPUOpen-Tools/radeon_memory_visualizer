//=============================================================================
/// Copyright (c) 2020 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Header for a snapshot item model. Used for the snapshot list table
///  in the snapshot generation pane in the timeline tab
//=============================================================================

#ifndef RMV_MODELS_TIMELINE_SNAPSHOT_ITEM_MODEL_H_
#define RMV_MODELS_TIMELINE_SNAPSHOT_ITEM_MODEL_H_

#include <QAbstractItemModel>

/// Enum corresponding to table columns in the snapshot table on the timeline pane.
enum SnapshotTimelineColumn
{
    kSnapshotTimelineColumnID,
    kSnapshotTimelineColumnName,
    kSnapshotTimelineColumnTime,
    kSnapshotTimelineColumnVirtualAllocations,
    kSnapshotTimelineColumnResources,
    kSnapshotTimelineColumnAllocatedTotalVirtualMemory,
    kSnapshotTimelineColumnAllocatedBoundVirtualMemory,
    kSnapshotTimelineColumnAllocatedUnboundVirtualMemory,
    kSnapshotTimelineColumnCommittedLocal,
    kSnapshotTimelineColumnCommittedInvisible,
    kSnapshotTimelineColumnCommittedHost,

    kSnapshotTimelineColumnCount,
};

namespace rmv
{
    class SnapshotItemModel : public QAbstractItemModel
    {
    public:
        /// Constructor.
        explicit SnapshotItemModel(QObject* parent = nullptr);

        /// Destructor.
        ~SnapshotItemModel();

        /// Set the number of rows in the table.
        /// \param rows The number of rows required.
        void SetRowCount(int rows);

        /// Set the number of columns in the table.
        /// \param columns The number of columns required.
        void SetColumnCount(int columns);

        // QAbstractItemModel overrides. See Qt documentation for parameter and return values
        virtual bool          setData(const QModelIndex& index, const QVariant& value, int role) Q_DECL_OVERRIDE;
        virtual QVariant      data(const QModelIndex& index, int role) const Q_DECL_OVERRIDE;
        virtual Qt::ItemFlags flags(const QModelIndex& index) const Q_DECL_OVERRIDE;
        virtual QVariant      headerData(int section, Qt::Orientation orientation, int role) const Q_DECL_OVERRIDE;
        virtual QModelIndex   index(int row, int column, const QModelIndex& parent) const Q_DECL_OVERRIDE;
        virtual QModelIndex   parent(const QModelIndex& index) const Q_DECL_OVERRIDE;
        virtual int           rowCount(const QModelIndex& parent = QModelIndex()) const Q_DECL_OVERRIDE;
        virtual int           columnCount(const QModelIndex& parent = QModelIndex()) const Q_DECL_OVERRIDE;

    private:
        int num_rows_;     ///< The number of rows in the table.
        int num_columns_;  ///< The number of columns in the table.
    };
}  // namespace rmv

#endif  // RMV_MODELS_TIMELINE_SNAPSHOT_ITEM_MODEL_H_
