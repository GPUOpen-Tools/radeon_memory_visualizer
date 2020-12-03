//=============================================================================
/// Copyright (c) 2018-2020 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Header for a proxy filter that processes multiple columns.
//=============================================================================

#ifndef RMV_MODELS_PROXY_MODELS_MEMORY_LEAK_FINDER_PROXY_MODEL_H_
#define RMV_MODELS_PROXY_MODELS_MEMORY_LEAK_FINDER_PROXY_MODEL_H_

#include "models/proxy_models/resource_proxy_model.h"

namespace rmv
{
    /// Class to filter out and sort the memory leak table.
    class MemoryLeakFinderProxyModel : public ResourceProxyModel
    {
        Q_OBJECT

    public:
        /// Constructor.
        /// \param compare_id_filter The compare filter.
        /// \param parent The parent widget.
        explicit MemoryLeakFinderProxyModel(uint32_t compare_id_filter, QObject* parent = nullptr);

        /// Destructor.
        virtual ~MemoryLeakFinderProxyModel();

        /// Update the filter.
        /// \param compare_filter The new filter.
        void UpdateCompareFilter(SnapshotCompareId compare_filter);

    protected:
        /// Make the filter run across multiple columns.
        /// \param source_row The target row.
        /// \param source_parent The source parent.
        /// \return true if the row passed the filter, false if not.
        virtual bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const override;

        uint32_t compare_id_filter_;  ///< Filtering flags specified in the UI.
    };
}  // namespace rmv

#endif  // RMV_MODELS_PROXY_MODELS_MEMORY_LEAK_FINDER_PROXY_MODEL_H_
