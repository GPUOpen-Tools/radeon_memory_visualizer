//=============================================================================
// Copyright (c) 2019-2021 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Header file for the colorizer base class.
///
/// Derived classes of this will implement the "color by" combo boxes
/// throughout the UI and the colorizing of the timeline.
///
//=============================================================================

#ifndef RMV_MODELS_COLORIZER_BASE_H_
#define RMV_MODELS_COLORIZER_BASE_H_

#include <QWidget>
#include <QGraphicsView>

#include "qt_common/custom_widgets/arrow_icon_combo_box.h"
#include "qt_common/custom_widgets/colored_legend_scene.h"

#include "rmt_format.h"
#include "rmt_resource_list.h"
#include "rmt_virtual_allocation_list.h"

namespace rmv
{
    /// @brief The colorizer base class. Handles basic colorizing across different selection modes.
    class ColorizerBase : public QObject
    {
    public:
        /// @brief Constructor.
        explicit ColorizerBase();

        /// @brief Destructor.
        virtual ~ColorizerBase();

        /// @brief Enum of the different 'color by' modes available.
        enum ColorMode
        {
            kColorModeResourceUsageType,
            kColorModePreferredHeap,
            kColorModeActualHeap,
            kColorModeAllocationAge,
            kColorModeResourceCreateAge,
            kColorModeResourceBindAge,
            kColorModeResourceGUID,
            kColorModeResourceCPUMapped,
            kColorModeNotAllPreferred,
            kColorModeAliasing,
            kColorModeCommitType,

            kColorModeCount,
        };

        /// @brief Initialize the colorizer.
        ///
        /// @param [in] combo_box    The combo box containing the different coloring modes.
        /// @param [in] legends_view The graphics view containing the color legends.
        void Initialize(ArrowIconComboBox* combo_box, QGraphicsView* legends_view);

        /// @brief Function to call when picking the color based on color mode.
        ///
        /// NOTE: The input parameters can be null since not all cases may be
        /// required or valid. In this case, the color indicating 'unallocated'
        /// will be returned.
        ///
        /// @param [in] allocation The allocation to use when deciding the coloring.
        /// @param [in] resource   The resource to use when deciding the color.
        ///
        /// @return The color to use.
        QColor GetColor(const RmtVirtualAllocation* allocation, const RmtResource* resource) const;

        /// @brief Function to call when picking the color based on color mode.
        ///
        /// @param [in] The index of the color for the current color mode.
        ///
        /// @return The color to use.
        QColor GetColor(const uint32_t color_index);

        /// @brief Update color legends to the UI depending on the coloring mode.
        void UpdateLegends();

        /// @brief Get the resource usage color.
        ///
        /// NOTE: Currently static so it can be used without a colorizer object.
        ///
        /// @param [in] usage_type The resource usage type.
        ///
        /// @return The color to use.
        static QColor GetResourceUsageColor(RmtResourceUsageType usage_type);

        /// @brief Get the color corresponding to the heap of a resource.
        ///
        /// @param [in] heap_type The heap type.
        ///
        /// @return The color required.
        static QColor GetHeapColor(RmtHeapType heap_type);

        /// @brief Get the number of age buckets.
        ///
        /// For now this value is shared between resources and allocations.
        ///
        /// @return The number of buckets.
        static int32_t GetNumAgeBuckets();

        /// @brief Get the age index for the age of a given allocation.
        ///
        /// The larger the value, the older it is. The allocation. Range is from 0 to GetNumResourceAgeBuckets() - 1.
        ///
        /// @param [in] timestamp The timestamp of the objects whose age is to be calculated.
        ///
        /// @return The age index, or -1 if error.
        static int32_t GetAgeIndex(const uint64_t timestamp);

    protected:
        /// @brief Update color legends to the UI depending on the coloring mode.
        ///
        /// @param [in] legends_scene The color legend view to be updated.
        /// @param [in] color_mode    The current coloring mode.
        void UpdateLegends(ColoredLegendScene* legends_scene, ColorMode color_mode);

        /// @brief Get the color corresponding to the age of a resource.
        ///
        /// @param [in] age_index The age index of the resource. A value of 0 is the oldest.
        ///
        /// @return The color required.
        QColor GetAgeColor(int32_t age_index) const;

        ArrowIconComboBox*  combo_box_;                        ///< The combo box holding the color modes available.
        ColoredLegendScene* legends_scene_;                    ///< The legends scene showing what the colors represent.
        QGraphicsView*      legends_view_;                     ///< The legends view associated with the scene.
        ColorMode           color_mode_;                       ///< The current coloring mode.
        ColorMode           color_mode_map_[kColorModeCount];  ///< The mapping of combo box index to color mode.
    };
}  // namespace rmv
#endif  // RMV_MODELS_COLORIZER_BASE_H_
