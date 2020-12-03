//=============================================================================
/// Copyright (c) 2019-2020 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \brief  Header file for the colorizer base class.
/// Derived classes of this will implement the "color by" combo boxes
/// throughout the UI and the colorizing of the timeline
//=============================================================================

#ifndef RMV_VIEWS_COLORIZER_BASE_H_
#define RMV_VIEWS_COLORIZER_BASE_H_

#include <QWidget>
#include <QGraphicsView>

#include "qt_common/custom_widgets/arrow_icon_combo_box.h"
#include "qt_common/custom_widgets/colored_legend_scene.h"

#include "rmt_format.h"
#include "rmt_resource_list.h"
#include "rmt_virtual_allocation_list.h"

/// The colorizer base class. Handles basic colorizing across different
/// selection modes.
class ColorizerBase : public QObject
{
public:
    /// Constructor.
    explicit ColorizerBase();

    /// Destructor.
    virtual ~ColorizerBase();

    /// enum of the different 'color by' modes available.
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

    /// Initialize the colorizer.
    /// \param combo_box The combo box containing the different coloring modes.
    /// \param legends_view The graphics view containing the color legends.
    void Initialize(ArrowIconComboBox* combo_box, QGraphicsView* legends_view);

    /// Function to call when picking the color based on color mode.
    /// NOTE: The input parameters can be null since not all cases may be
    /// required or valid. In this case, the color indicating 'unallocated'
    /// will be returned.
    /// \param allocation The allocation to use when deciding the coloring.
    /// \param resource The resource to use when deciding the color.
    /// \return The color to use.
    QColor GetColor(const RmtVirtualAllocation* allocation, const RmtResource* resource) const;

    /// Function to call when picking the color based on color mode.
    /// \param The index of the color for the current color mode.
    /// \return The color to use.
    QColor GetColor(const uint32_t color_index);

    /// Update color legends to the UI depending on the coloring mode.
    void UpdateLegends();

    /// Get the resource usage color.
    /// NOTE: currently static so it can be used without a colorizer object
    /// \param usage_type The resource usage type.
    /// \return The color to use.
    static QColor GetResourceUsageColor(RmtResourceUsageType usage_type);

    /// Get the color corresponding to the heap of a resource.
    /// \return the color required.
    static QColor GetHeapColor(RmtHeapType heap_type);

    /// Get the number of age buckets. For now this value is shared between
    /// resources and allocations.
    /// \return The number of buckets.
    static int32_t GetNumAgeBuckets();

    /// Get the age index for the age of a given allocation. The larger the value, the older it is.
    /// The allocation. Range is from 0 to GetNumResourceAgeBuckets() - 1
    /// \param timestamp The timestamp of the objects whose age is to be calculated.
    /// \return The age index, or -1 if error.
    static int32_t GetAgeIndex(const uint64_t timestamp);

protected:
    /// Update color legends to the UI depending on the coloring mode.
    void UpdateLegends(ColoredLegendScene* legends_scene, ColorMode color_mode);

    /// Get the color corresponding to the age of a resource.
    /// \param age_index The age index of the resource. A value of 0
    /// is the oldest.
    /// \return the color required.
    QColor GetAgeColor(int32_t age_index) const;

    ArrowIconComboBox*  combo_box_;                        ///< The combo box holding the color modes available.
    ColoredLegendScene* legends_scene_;                    ///< The legends scene showing what the colors represent.
    QGraphicsView*      legends_view_;                     ///< The legends view associated with the scene.
    ColorMode           color_mode_;                       ///< THe current coloring mode.
    ColorMode           color_mode_map_[kColorModeCount];  ///< The mapping of combo box index to color mode.
};

#endif  // RMV_VIEWS_COLORIZER_BASE_H_
