//=============================================================================
/// Copyright (c) 2018-2020 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Header for RMV's carousel memory footprint widget
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
    /// Constructor.
    /// \param config A configuration struct for this object.
    explicit RMVCarouselMemoryFootprint(const RMVCarouselConfig& config);

    /// Destructor.
    virtual ~RMVCarouselMemoryFootprint();

    /// Implementation of Qt's paint for this item.
    /// \param painter The painter object to use.
    /// \param item Provides style options for the item, such as its state, exposed area and its level-of-detail hints.
    /// \param widget Points to the widget that is being painted on if specified.
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* item, QWidget* widget) override;

    /// Set new data and update.
    /// \param data The new data.
    void SetData(const RMVCarouselData& data) override;

private:
    RMVCarouselMemoryFootprintData data_;  ///< The data required by this item.
};

#endif  // RMV_VIEWS_CUSTOM_WIDGETS_RMV_CAROUSEL_MEMORY_FOOTPRINT_H_
