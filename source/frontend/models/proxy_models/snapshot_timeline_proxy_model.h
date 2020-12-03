//=============================================================================
/// Copyright (c) 2018-2020 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Header for a proxy filter that processes multiple columns.
//=============================================================================

#ifndef RMV_MODELS_PROXY_MODELS_SNAPSHOT_TIMELINE_PROXY_MODEL_H_
#define RMV_MODELS_PROXY_MODELS_SNAPSHOT_TIMELINE_PROXY_MODEL_H_

#include "models/proxy_models/table_proxy_model.h"

namespace rmv
{
    /// Class to filter out and sort the snapshot table on the timeline pane.
    class SnapshotTimelineProxyModel : public TableProxyModel
    {
        Q_OBJECT

    public:
        /// Constructor.
        /// \param parent The parent widget.
        explicit SnapshotTimelineProxyModel(QObject* parent = nullptr);

        /// Destructor.
        virtual ~SnapshotTimelineProxyModel();

    protected:
        /// Make the filter run across multiple columns.
        /// \param source_row the target row.
        /// \param source_parent the source parent.
        /// \return pass or not.
        virtual bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const override;

        /// Implement the comparison for sorting.
        /// \param left the left item to compare.
        /// \param right the right item to compare.
        /// \return If left < right then true, else false.
        virtual bool lessThan(const QModelIndex& left, const QModelIndex& right) const override;
    };
}  // namespace rmv

#endif  // RMV_MODELS_PROXY_MODELS_SNAPSHOT_TIMELINE_PROXY_MODEL_H_
