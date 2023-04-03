//=============================================================================
// Copyright (c) 2018-2023 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Header for the carousel resource types widget.
//=============================================================================

#ifndef RMV_VIEWS_CUSTOM_WIDGETS_RMV_CAROUSEL_RESOURCE_TYPES_H_
#define RMV_VIEWS_CUSTOM_WIDGETS_RMV_CAROUSEL_RESOURCE_TYPES_H_

#include <QGraphicsObject>

#include "models/carousel_model.h"
#include "views/custom_widgets/rmv_carousel_item.h"

/// @brief Container class for the carousel's resource types component.
class RMVCarouselResourceTypes : public RMVCarouselItem
{
    Q_OBJECT

public:
    /// @brief Constructor.
    ///
    /// @param [in] config A configuration struct for this object.
    explicit RMVCarouselResourceTypes(const RMVCarouselConfig& config);

    /// @brief Destructor.
    virtual ~RMVCarouselResourceTypes();

    /// @brief Implementation of Qt's paint for this item.
    ///
    /// @param [in] painter The painter object to use.
    /// @param [in] option  Provides style options for the item, such as its state, exposed area and its level-of-detail hints.
    /// @param [in] widget  Points to the widget that is being painted on if specified.
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) Q_DECL_OVERRIDE;

    /// @brief Update the data for this item.
    ///
    /// @param [in] data New data.
    void SetData(const rmv::RMVCarouselData& data) Q_DECL_OVERRIDE;

private:
    /// @brief Helper function to draw color key for a memory usage.
    ///
    /// @param [in] painter        The Qt painter.
    /// @param [in] y_offset       How far down should this get drawn.
    /// @param [in] resource_name  The resource name.
    /// @param [in] resource_color The resource color.
    /// @param [in] usage_amount   The data.
    void DrawCarouselMemoryUsageLegend(QPainter* painter, uint32_t y_offset, const QString& resource_name, const QColor& resource_color, int32_t usage_amount);

    rmv::RMVCarouselResourceTypesData data_;  ///< The model data for this carousel item.
};

#endif  // RMV_VIEWS_CUSTOM_WIDGETS_RMV_CAROUSEL_RESOURCE_TYPES_H_
