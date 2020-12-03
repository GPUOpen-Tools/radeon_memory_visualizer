//=============================================================================
/// Copyright (c) 2018-2020 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Header for RMV's carousel allocation sizes widget
//=============================================================================

#ifndef RMV_VIEWS_CUSTOM_WIDGETS_RMV_CAROUSEL_ALLOCATION_SIZES_H_
#define RMV_VIEWS_CUSTOM_WIDGETS_RMV_CAROUSEL_ALLOCATION_SIZES_H_

#include <QGraphicsObject>

#include "models/carousel_model.h"
#include "views/custom_widgets/rmv_carousel_item.h"

/// Container class for the carousel allocation sizes component.
class RMVCarouselAllocationSizes : public RMVCarouselItem
{
    Q_OBJECT

public:
    /// Constructor.
    /// \param config A configuration struct for this object.
    explicit RMVCarouselAllocationSizes(const RMVCarouselConfig& config);

    /// Destructor
    virtual ~RMVCarouselAllocationSizes();

    /// Implementation of Qt's paint for this item.
    /// \param painter The painter object to use.
    /// \param option Provides style options for the item, such as its state, exposed area and its level-of-detail hints.
    /// \param widget Points to the widget that is being painted on if specified.
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

    /// Set new data and update.
    /// \param data new data.
    void SetData(const RMVCarouselData& data) override;

private:
    /// Draw a single allocation bar. This is currently a vertical bar representing
    /// the number of allocations in a bucket.
    /// \param painter The painter object to use.
    /// \param bar_width The width of the bar in pixels (unscaled).
    /// \param x_pos The x position of the bar, in pixels (unscaled).
    /// \param value The value represented by the bar.
    /// \param label_string The text label to display under the bar, indicating the range
    /// of values this bar represents.
    void DrawAllocationBar(QPainter* painter, int bar_width, int x_pos, int32_t value, const QString& label_string);

    RMVCarouselAllocationSizesData data_;  ///< The model data for this carousel item.
};

#endif  // RMV_VIEWS_CUSTOM_WIDGETS_RMV_CAROUSEL_ALLOCATION_SIZES_H_
