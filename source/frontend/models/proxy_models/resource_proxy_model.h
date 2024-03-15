//=============================================================================
// Copyright (c) 2018-2024 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Header for a proxy filter that processes multiple columns
/// of a resource table.
//=============================================================================

#ifndef RMV_MODELS_PROXY_MODELS_RESOURCE_PROXY_MODEL_H_
#define RMV_MODELS_PROXY_MODELS_RESOURCE_PROXY_MODEL_H_

#include <QTableView>

#include "models/proxy_models/table_proxy_model.h"
#include "models/resource_item_model.h"

namespace rmv
{
    /// @brief Class to filter out and sort a resource table
    class ResourceProxyModel : public TableProxyModel
    {
        Q_OBJECT

    public:
        /// @brief Constructor.
        ///
        /// @param [in] parent The parent widget.
        explicit ResourceProxyModel(QObject* parent = nullptr);

        /// @brief Destructor.
        virtual ~ResourceProxyModel();

        /// @brief Initialize the resource table model.
        ///
        /// @param [in] view        The table view.
        /// @param [in] num_rows    The table row count.
        /// @param [in] num_columns The table column count.
        ///
        /// @return the model for the resource table model.
        ResourceItemModel* InitializeResourceTableModels(QTableView* view, int num_rows, int num_columns);

        /// @brief Set the preferred heap filter regular expression.
        ///
        /// Called when the user selects visible heaps from the 'preferred heap' combo box.
        /// Rather than rebuild the table, this regular expression is added to the filter to
        /// filter out heaps that don't need to be shown.
        ///
        /// @param [in] preferred_heap_filter The regular expression for the preferred heap filter.
        void SetPreferredHeapFilter(const QString& preferred_heap_filter);

        /// @brief Set the preferred heap filter regular expression.
        ///
        /// Called when the user selects visible heaps from the 'resource usage' combo box.
        /// Rather than rebuild the table, this regular expression is added to the filter to
        /// filter out heaps that don't need to be shown.
        ///
        /// @param resource_usage_filter The regular expression for the resource usage filter.
        void SetResourceUsageFilter(const QString& resource_usage_filter);

    protected:
        /// @brief Make the filter run across multiple columns.
        ///
        /// @param [in] source_row    The target row.
        /// @param [in] source_parent The source parent.
        ///
        /// @return true if the row passed the filter, false if not.
        virtual bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const override;

        /// @brief The sort comparator.
        ///
        /// @param [in] left  The left item to compare.
        /// @param [in] right The right item to compare.
        ///
        /// @return true if left is less than right, false otherwise.
        virtual bool lessThan(const QModelIndex& left, const QModelIndex& right) const override;

    private:
        /// @brief Handle sorting when two resource parameters are identical.
        ///
        /// In this case, the heap resource is listed first.
        ///
        /// @param [in] left  The left item to compare.
        /// @param [in] right The right item to compare.
        ///
        /// @return true if left is less than right, false otherwise.
        bool SortIdentical(const QModelIndex& left, const QModelIndex& right) const;

        QRegularExpression preferred_heap_filter_;  ///< The preferred heap filter regular expression.
        QRegularExpression resource_usage_filter_;  ///< The resource usage filter regular expression.
    };
}  // namespace rmv

#endif  // RMV_MODELS_PROXY_MODELS_RESOURCE_PROXY_MODEL_H_
