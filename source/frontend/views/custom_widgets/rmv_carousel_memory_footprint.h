//=============================================================================
// Copyright (c) 2018-2024 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Header for the carousel memory footprint widget.
//=============================================================================

#ifndef RMV_VIEWS_CUSTOM_WIDGETS_RMV_CAROUSEL_MEMORY_FOOTPRINT_H_
#define RMV_VIEWS_CUSTOM_WIDGETS_RMV_CAROUSEL_MEMORY_FOOTPRINT_H_

#include <QGraphicsObject>

#include "models/carousel_model.h"
#include "views/custom_widgets/rmv_carousel_item.h"

/// Container class for the carousel's memory footprint component.
class RMVCarouselMemoryFootprint : public RMVCarouselItem
{
    Q_OBJECT

public:
    /// @brief Constructor.
    ///
    /// @param [in] config A configuration struct for this object.
    explicit RMVCarouselMemoryFootprint(const RMVCarouselConfig& config);

    /// @brief Destructor.
    virtual ~RMVCarouselMemoryFootprint();

    /// @brief Implementation of Qt's paint for this item.
    ///
    /// @param [in] painter The painter object to use.
    /// @param [in] item    Provides style options for the item, such as its state, exposed area and its level-of-detail hints.
    /// @param [in] widget  Points to the widget that is being painted on if specified.
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* item, QWidget* widget) Q_DECL_OVERRIDE;

    /// @brief Set new data and update.
    ///
    /// @param [in] data The new data.
    void SetData(const rmv::RMVCarouselData& data) Q_DECL_OVERRIDE;

private:
    rmv::RMVCarouselMemoryFootprintData data_;  ///< The data required by this item.
};

#endif  // RMV_VIEWS_CUSTOM_WIDGETS_RMV_CAROUSEL_MEMORY_FOOTPRINT_H_
