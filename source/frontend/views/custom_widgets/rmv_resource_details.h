//=============================================================================
/// Copyright (c) 2018-2020 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Header for the resource details widget. This is the details
/// widget at the bottom of the resource overview that's shown when a resource
/// is clicked on.
//=============================================================================

#ifndef RMV_VIEWS_CUSTOM_WIDGETS_RMV_RESOURCE_DETAILS_H_
#define RMV_VIEWS_CUSTOM_WIDGETS_RMV_RESOURCE_DETAILS_H_

#include <QGraphicsObject>

#include "rmt_resource_list.h"
#include "rmt_virtual_allocation_list.h"

#include "views/colorizer.h"

static const int kResourceDetailsHeight = 50;

/// Describes an allocation details widget.
struct RMVResourceDetailsConfig
{
    uint32_t         width;                 ///< Widget width.
    uint32_t         height;                ///< Widget height.
    RmtResource      resource;              ///< A copy of the resource.
    bool             resource_valid;        ///< Is the resource above valid.
    bool             allocation_thumbnail;  ///< Should the allocation rendering on the left be included?
    const Colorizer* colorizer;             ///< The colorizer used to color this widget.
};

/// Container class for resource details which is rendered in resource overview and allocation delta pane.
class RMVResourceDetails : public QGraphicsObject
{
    Q_OBJECT

public:
    /// Constructor.
    /// \param config A configuration struct for this object.
    explicit RMVResourceDetails(const RMVResourceDetailsConfig& config);

    /// Destructor.
    virtual ~RMVResourceDetails();

    /// Implementation of Qt's bounding volume for this item.
    /// \return The item's bounding rectangle.
    virtual QRectF boundingRect() const Q_DECL_OVERRIDE;

    /// Implementation of Qt's paint for this item.
    /// \param painter The painter object to use.
    /// \param item Provides style options for the item, such as its state, exposed area and its level-of-detail hints.
    /// \param widget Points to the widget that is being painted on if specified.
    virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* item, QWidget* widget) Q_DECL_OVERRIDE;

    /// Update the current resource.
    /// \param resource The resource to update.
    void UpdateResource(const RmtResource* resource);

    /// Set width and height of the QGraphicsView being drawn into.
    /// \param width The width.
    /// \param height The height.
    void UpdateDimensions(int width, int height);

    /// Get the resource being displayed.
    /// \return The resource.
    const RmtResource* GetResource() const;

private:
    RMVResourceDetailsConfig config_;  ///< Description of this widget.
};

#endif  // RMV_VIEWS_CUSTOM_WIDGETS_RMV_RESOURCE_DETAILS_H_
