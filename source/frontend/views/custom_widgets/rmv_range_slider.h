//=============================================================================
// Copyright (c) 2024-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Header for the custom range slider widget.
//=============================================================================

#ifndef RMV_VIEWS_CUSTOM_WIDGETS_RMV_RANGE_SLIDER_H_
#define RMV_VIEWS_CUSTOM_WIDGETS_RMV_RANGE_SLIDER_H_

#include "qt_common/custom_widgets/double_slider_widget.h"

#include "views/custom_widgets/rmv_fixed_width_label.h"
#include <QWidget>

/// @brief Range slider that extends the double slider widget by adding a range value label.
class RmvRangeSlider : public DoubleSliderWidget
{
public:
    /// @brief Constructor
    ///
    /// @param [in] parent                               The parent of slider widget.
    explicit RmvRangeSlider(QWidget* parent = nullptr);

    /// @brief Destructor.
    virtual ~RmvRangeSlider();

    /// @brief Intializes the range slider and adds the range value label.
    void Init();

private slots:
    /// @brief Slot to update the range value label when the slide is adjusted.
    ///
    /// @param [in] min_value                       The minimum index value for the slider.
    /// @param [in] max_value                       The maximum index value for the slider.
    void UpdateValues(const int min_value, const int max_value);

private:
    RmvFixedWidthLabel* range_value_label_;  ///< A pointer to the label widget that displays the range values.
};
#endif  // RMV_VIEWS_CUSTOM_WIDGETS_RMV_RANGE_SLIDER_H_
