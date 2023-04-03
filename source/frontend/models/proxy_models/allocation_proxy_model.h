//=============================================================================
// Copyright (c) 2019-2023 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Header for a proxy filter that processes multiple columns
/// of an allocation table.
//=============================================================================

#ifndef RMV_MODELS_PROXY_MODELS_ALLOCATION_PROXY_MODEL_H_
#define RMV_MODELS_PROXY_MODELS_ALLOCATION_PROXY_MODEL_H_

#include <QTableView>

#include "models/allocation_item_model.h"
#include "models/proxy_models/table_proxy_model.h"

namespace rmv
{
    /// @brief Class to filter out and sort an allocation table.
    class AllocationProxyModel : public TableProxyModel
    {
        Q_OBJECT

    public:
        /// @brief Constructor.
        ///
        /// @param [in] parent The parent widget.
        explicit AllocationProxyModel(QObject* parent = nullptr);

        /// @brief Destructor.
        virtual ~AllocationProxyModel();

        /// @brief Initialize the allocation table model.
        ///
        /// @param [in] table_view  The table view.
        /// @param [in] num_rows    The table row count.
        /// @param [in] num_columns The table column count.
        ///
        /// @return the model for the allocation table model.
        AllocationItemModel* InitializeAllocationTableModels(QTableView* table_view, int num_rows, int num_columns);

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
    };
}  // namespace rmv

#endif  // RMV_MODELS_PROXY_MODELS_ALLOCATION_PROXY_MODEL_H_
