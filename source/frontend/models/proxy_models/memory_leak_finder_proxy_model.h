//=============================================================================
// Copyright (c) 2018-2024 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Header for a proxy filter that processes multiple columns of the
/// memory leak finder table.
//=============================================================================

#ifndef RMV_MODELS_PROXY_MODELS_MEMORY_LEAK_FINDER_PROXY_MODEL_H_
#define RMV_MODELS_PROXY_MODELS_MEMORY_LEAK_FINDER_PROXY_MODEL_H_

#include "models/proxy_models/resource_proxy_model.h"

namespace rmv
{
    /// @brief Class to filter out and sort the memory leak table.
    class MemoryLeakFinderProxyModel : public ResourceProxyModel
    {
        Q_OBJECT

    public:
        /// @brief Constructor.
        ///
        /// @param [in] compare_id_filter The compare filter.
        /// @param [in] parent            The parent widget.
        explicit MemoryLeakFinderProxyModel(uint32_t compare_id_filter, QObject* parent = nullptr);

        /// @brief Destructor.
        virtual ~MemoryLeakFinderProxyModel();

        /// @brief Update the filter.
        ///
        /// @param [in] compare_filter The new filter.
        void UpdateCompareFilter(SnapshotCompareId compare_filter);

    protected:
        /// @brief Make the filter run across multiple columns.
        ///
        /// @param [in] source_row    The target row.
        /// @param [in] source_parent The source parent.
        ///
        /// @return true if the row passed the filter, false if not.
        virtual bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const override;

        uint32_t compare_id_filter_;  ///< Filtering flags specified in the UI.
    };
}  // namespace rmv

#endif  // RMV_MODELS_PROXY_MODELS_MEMORY_LEAK_FINDER_PROXY_MODEL_H_
