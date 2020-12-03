//=============================================================================
/// Copyright (c) 2019-2020 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Header for the memory map model.
//=============================================================================

#ifndef RMV_MODELS_SNAPSHOT_MEMORY_MAP_MODEL_H_
#define RMV_MODELS_SNAPSHOT_MEMORY_MAP_MODEL_H_

#include <QColor>
#include <QWidget>

#include "rmt_resource_list.h"
#include "rmt_types.h"
#include "rmt_virtual_allocation_list.h"

#include "views/colorizer.h"

namespace rmv
{
    class MemoryMapModel
    {
    public:
        /// Constructor.
        explicit MemoryMapModel(Colorizer* colorizer);

        /// Destructor.
        virtual ~MemoryMapModel();

        /// Update the granularity. Also causes the block data to be recalculated.
        /// \param granularity The new granularity to use, in bytes.
        void UpdateGranularity(int granularity);

        /// Get the color of a memory cell. This will depend on the granularity and the coloring mode.
        /// \param block_offset The offset in blocks of the cell to color. The offset is based on the granularity.
        /// \return The color to use.
        const QColor GetColor(uint64_t block_offset) const;

        /// Get the block offset.
        ///\ return The block offset.
        uint64_t GetBlockOffset() const;

        /// Get the total number of memory blocks at the current granularity.
        /// \return The number of memory blocks.
        uint64_t GetNumBlocks() const;

    private:
        /// Get the allocation corresponding to a given base address.
        /// \param baseAddress The base address to find the allocation for.
        /// \return The virtual allocation corresponding to the given base address.
        const RmtVirtualAllocation* GetAllocation(uint64_t base_address) const;

        /// Get the resource corresponding to a given base address.
        /// \param baseAddress The base address to find the resource for.
        /// \return The resource corresponding to the given base address.
        const RmtResource* GetResource(uint64_t base_address) const;

        int              granularity_;              ///< The current granularity set in the UI, in bytes.
        uint64_t         num_blocks_;               ///< Total number of blocks at the current granularity.
        uint64_t         block_offset_;             ///< The offset into the memory map, in blocks at the current granularity.
        RmtGpuAddress    minimum_virtual_address_;  ///< Trimmed start address of the memory map. Anything before this is not important.
        RmtGpuAddress    maximum_virtual_address_;  ///< Trimmed end address of the memory map. Anything after this is not important.
        const Colorizer* colorizer_;                ///< The colorizer used by the 'color by' combo box.
    };
}  // namespace rmv

#endif  // RMV_MODELS_SNAPSHOT_MEMORY_MAP_MODEL_H_
