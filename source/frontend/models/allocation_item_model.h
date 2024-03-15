//=============================================================================
// Copyright (c) 2020-2024 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Header for an allocation item model.
///
/// Used for the allocation list tables.
///
//=============================================================================

#ifndef RMV_MODELS_ALLOCATION_ITEM_MODEL_H_
#define RMV_MODELS_ALLOCATION_ITEM_MODEL_H_

#include <QAbstractItemModel>

#include "rmt_data_snapshot.h"
#include "rmt_virtual_allocation_list.h"

namespace rmv
{
    /// @brief A class to handle the model data associated with an allocation table.
    class AllocationItemModel : public QAbstractItemModel
    {
    public:
        /// @brief Constructor.
        explicit AllocationItemModel(QObject* parent = nullptr);

        /// @brief Destructor.
        virtual ~AllocationItemModel();

        /// @brief Set the number of rows in the table.
        ///
        /// @param [in] rows The number of rows required.
        void SetRowCount(int rows);

        /// @brief Set the number of columns in the table.
        ///
        /// @param [in] columns The number of columns required.
        void SetColumnCount(int columns);

        /// @brief Add an allocation to the table.
        ///
        /// @param [in] snapshot           The snapshot where the allocation data is located.
        /// @param [in] virtual_allocation The allocation to add.
        void AddAllocation(const RmtDataSnapshot* snapshot, const RmtVirtualAllocation* virtual_allocation);

        // QAbstractItemModel overrides. See Qt documentation for parameter and return values
        virtual QVariant      data(const QModelIndex& index, int role) const Q_DECL_OVERRIDE;
        virtual Qt::ItemFlags flags(const QModelIndex& index) const Q_DECL_OVERRIDE;
        virtual QVariant      headerData(int section, Qt::Orientation orientation, int role) const Q_DECL_OVERRIDE;
        virtual QModelIndex   index(int row, int column, const QModelIndex& parent) const Q_DECL_OVERRIDE;
        virtual QModelIndex   parent(const QModelIndex& index) const Q_DECL_OVERRIDE;
        virtual int           rowCount(const QModelIndex& parent = QModelIndex()) const Q_DECL_OVERRIDE;
        virtual int           columnCount(const QModelIndex& parent = QModelIndex()) const Q_DECL_OVERRIDE;

    private:
        /// @brief Data from the backend that needs caching for speed.
        struct DataCache
        {
            DataCache()
                : virtual_allocation(nullptr)
                , allocation_size(0)
                , bound_size(0)
                , unbound_size(0)
                , avg_resource_size(0)
                , std_dev_resource_size(0)
                , local_bytes(0)
                , invisible_bytes(0)
                , host_bytes(0)
                , unmapped_bytes(0)
            {
            }

            const RmtVirtualAllocation* virtual_allocation;     ///< The virtual allocation.
            uint64_t                    allocation_size;        ///< The allocation size.
            uint64_t                    bound_size;             ///< The size of bound memory.
            uint64_t                    unbound_size;           ///< The size of unbound memory.
            uint64_t                    avg_resource_size;      ///< The average resource size.
            uint64_t                    std_dev_resource_size;  ///< The standard deviation of the resource size.
            double                      local_bytes;            ///< Amount of local memory.
            double                      invisible_bytes;        ///< Amount of invisible memory.
            double                      host_bytes;             ///< Amount of host memory.
            double                      unmapped_bytes;         ///< Amount of unmapped memory.
        };

        int                    num_rows_;     ///< The number of rows in the table.
        int                    num_columns_;  ///< The number of columns in the table.
        std::vector<DataCache> cache_;        ///< Cached data from the backend.
    };
}  // namespace rmv

#endif  // RMV_MODELS_ALLOCATION_ITEM_MODEL_H_
