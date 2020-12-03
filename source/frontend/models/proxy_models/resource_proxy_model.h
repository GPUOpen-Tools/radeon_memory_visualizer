//=============================================================================
/// Copyright (c) 2018-2020 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Header for a proxy filter that processes multiple resource columns.
//=============================================================================

#ifndef RMV_MODELS_PROXY_MODELS_RESOURCE_PROXY_MODEL_H_
#define RMV_MODELS_PROXY_MODELS_RESOURCE_PROXY_MODEL_H_

#include <QTableView>

#include "models/proxy_models/table_proxy_model.h"
#include "models/resource_item_model.h"

namespace rmv
{
    /// Class to filter out and sort a resource table
    class ResourceProxyModel : public TableProxyModel
    {
        Q_OBJECT

    public:
        /// Constructor.
        /// \param parent The parent widget.
        explicit ResourceProxyModel(QObject* parent = nullptr);

        /// Destructor.
        virtual ~ResourceProxyModel();

        /// Initialize the resource table model.
        /// \param view table view.
        /// \param num_rows row count.
        /// \param num_columns column count.
        /// \return the model for the resource table model.
        ResourceItemModel* InitializeResourceTableModels(QTableView* view, int num_rows, int num_columns);

        /// Set the preferred heap filter regular expression. Called when the user selects visible
        /// heaps from the 'preferred heap' combo box. Rather than rebuild the table, this regular
        /// expression is added to the filter to filter out heaps that don't need to be shown.
        /// \param heap_filter The regular expression for the heap filter.
        void SetPreferredHeapFilter(const QString& preferred_heap_filter);

        /// Set the preferred heap filter regular expression. Called when the user selects visible
        /// heaps from the 'preferred heap' combo box. Rather than rebuild the table, this regular
        /// expression is added to the filter to filter out heaps that don't need to be shown.
        /// \param heap_filter The regular expression for the heap filter.
        void SetResourceUsageFilter(const QString& resource_usage_filter);

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

    private:
        QRegularExpression preferred_heap_filter_;  ///< The preferred heap filter regular expression.
        QRegularExpression resource_usage_filter_;  ///< The resource usage filter regular expression.
    };
}  // namespace rmv

#endif  // RMV_MODELS_PROXY_MODELS_RESOURCE_PROXY_MODEL_H_
