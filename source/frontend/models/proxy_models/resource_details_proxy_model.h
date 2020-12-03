//=============================================================================
/// Copyright (c) 2019-2020 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Header for a proxy filter that processes the resource details table
/// in the resource details pane.
//=============================================================================

#ifndef RMV_MODELS_PROXY_MODELS_RESOURCE_DETAILS_PROXY_MODEL_H_
#define RMV_MODELS_PROXY_MODELS_RESOURCE_DETAILS_PROXY_MODEL_H_

#include <QTableView>

#include "models/proxy_models/table_proxy_model.h"
#include "models/snapshot/resource_timeline_item_model.h"

namespace rmv
{
    /// Class to filter out and sort the resource details table.
    class ResourceDetailsProxyModel : public TableProxyModel
    {
        Q_OBJECT

    public:
        /// Constructor.
        /// \param parent The parent widget.
        explicit ResourceDetailsProxyModel(QObject* parent = nullptr);

        /// Destructor.
        virtual ~ResourceDetailsProxyModel();

        /// Initialize the resource table model.
        /// \param view The table view.
        /// \param num_rows row count.
        /// \param num_columns column count.
        /// \return the model for the resource table model.
        ResourceTimelineItemModel* InitializeResourceTableModels(QTableView* view, int num_rows, int num_columns);

    protected:
        /// Make the filter run across multiple columns.
        /// \param source_row the target row.
        /// \param source_parent the source parent.
        /// \return pass or not.
        virtual bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const override;

        /// The sort comparator.
        /// \param left the left item to compare.
        /// \param right the right item to compare.
        /// \return true if left is less than right, false otherwise.
        virtual bool lessThan(const QModelIndex& left, const QModelIndex& right) const override;
    };
}  // namespace rmv

#endif  // RMV_MODELS_PROXY_MODELS_RESOURCE_DETAILS_PROXY_MODEL_H_
