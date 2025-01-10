//=============================================================================
// Copyright (c) 2018-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Header for a proxy filter that processes multiple columns of the
/// snapshot timeline table.
//=============================================================================

#ifndef RMV_MODELS_PROXY_MODELS_SNAPSHOT_TIMELINE_PROXY_MODEL_H_
#define RMV_MODELS_PROXY_MODELS_SNAPSHOT_TIMELINE_PROXY_MODEL_H_

#include "models/proxy_models/table_proxy_model.h"

namespace rmv
{
    /// @brief Class to filter out and sort the snapshot table on the timeline pane.
    class SnapshotTimelineProxyModel : public TableProxyModel
    {
        Q_OBJECT

    public:
        /// @brief Constructor.
        ///
        /// @param [in] parent The parent widget.
        explicit SnapshotTimelineProxyModel(QObject* parent = nullptr);

        /// @brief Destructor.
        virtual ~SnapshotTimelineProxyModel();

    protected:
        /// @brief Make the filter run across multiple columns.
        ///
        /// @param [in] source_row    The target row.
        /// @param [in] source_parent The source parent.
        ///
        /// @return true if the row passed the filter, false if not.
        virtual bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const override;

        /// @brief Implement the comparison for sorting.
        ///
        /// @param [in] left  The left item to compare.
        /// @param [in] right The right item to compare.
        ///
        /// @return If left < right then true, else false.
        virtual bool lessThan(const QModelIndex& left, const QModelIndex& right) const override;
    };
}  // namespace rmv

#endif  // RMV_MODELS_PROXY_MODELS_SNAPSHOT_TIMELINE_PROXY_MODEL_H_
