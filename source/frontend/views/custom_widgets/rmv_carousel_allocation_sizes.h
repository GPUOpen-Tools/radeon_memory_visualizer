//=============================================================================
// Copyright (c) 2018-2024 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Header for the carousel allocation sizes widget.
//=============================================================================

#ifndef RMV_VIEWS_CUSTOM_WIDGETS_RMV_CAROUSEL_ALLOCATION_SIZES_H_
#define RMV_VIEWS_CUSTOM_WIDGETS_RMV_CAROUSEL_ALLOCATION_SIZES_H_

#include <QGraphicsObject>

#include "models/carousel_model.h"
#include "views/custom_widgets/rmv_carousel_item.h"

/// @brief Container class for the carousel allocation sizes component.
class RMVCarouselAllocationSizes : public RMVCarouselItem
{
    Q_OBJECT

public:
    /// @brief Constructor.
    ///
    /// @param [in] config A configuration struct for this object.
    explicit RMVCarouselAllocationSizes(const RMVCarouselConfig& config);

    /// @brief Destructor.
    virtual ~RMVCarouselAllocationSizes();

    /// @brief Implementation of Qt's paint for this item.
    ///
    /// @param [in] painter The painter object to use.
    /// @param [in] option  Provides style options for the item, such as its state, exposed area and its level-of-detail hints.
    /// @param [in] widget  Points to the widget that is being painted on if specified.
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) Q_DECL_OVERRIDE;

    /// @brief Set new data and update.
    ///
    /// @param [in] data New data.
    void SetData(const rmv::RMVCarouselData& data) Q_DECL_OVERRIDE;

private:
    /// @brief Draw a single allocation bar.
    ///
    /// This is currently a vertical bar representing the number of allocations in a bucket.
    ///
    /// @param [in] painter      The painter object to use.
    /// @param [in] bar_width    The width of the bar in pixels (unscaled).
    /// @param [in] x_pos        The x position of the bar, in pixels (unscaled).
    /// @param [in] value        The value represented by the bar.
    /// @param [in] label_string The text label to display under the bar, indicating the range of values this bar represents.
    void DrawAllocationBar(QPainter* painter, int bar_width, int x_pos, int32_t value, const QString& label_string);

    rmv::RMVCarouselAllocationSizesData data_;  ///< The model data for this carousel item.
};

#endif  // RMV_VIEWS_CUSTOM_WIDGETS_RMV_CAROUSEL_ALLOCATION_SIZES_H_
