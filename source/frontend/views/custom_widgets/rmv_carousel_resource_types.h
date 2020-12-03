//=============================================================================
/// Copyright (c) 2018-2020 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Header for RMV's carousel resource types widget
//=============================================================================

#ifndef RMV_VIEWS_CUSTOM_WIDGETS_RMV_CAROUSEL_RESOURCE_TYPES_H_
#define RMV_VIEWS_CUSTOM_WIDGETS_RMV_CAROUSEL_RESOURCE_TYPES_H_

#include <QGraphicsObject>

#include "models/carousel_model.h"
#include "views/custom_widgets/rmv_carousel_item.h"

/// Container class for the carousel's resource types component.
class RMVCarouselResourceTypes : public RMVCarouselItem
{
    Q_OBJECT

public:
    /// Constructor.
    /// \param config A configuration struct for this object.
    explicit RMVCarouselResourceTypes(const RMVCarouselConfig& config);

    /// Destructor.
    virtual ~RMVCarouselResourceTypes();

    /// Implementation of Qt's paint for this item.
    /// \param painter The painter object to use.
    /// \param option Provides style options for the item, such as its state, exposed area and its level-of-detail hints.
    /// \param widget Points to the widget that is being painted on if specified.
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

    /// Update the data for this item.
    /// \param data new data.
    void SetData(const RMVCarouselData& data) override;

private:
    /// Helper function to draw color key for a memory usage.
    /// \param painter Rhe Qt painter.
    /// \param y_offset How far down should this get drawn.
    /// \param resource_name The resource name.
    /// \param resource_color The resource color.
    /// \param usage_amount the data.
    void DrawCarouselMemoryUsageLegend(QPainter* painter, uint32_t y_offset, const QString& resource_name, const QColor& resource_color, int32_t usage_amount);

    RMVCarouselResourceTypesData data_;  ///< The model data for this carousel item.
};

#endif  // RMV_VIEWS_CUSTOM_WIDGETS_RMV_CAROUSEL_RESOURCE_TYPES_H_
