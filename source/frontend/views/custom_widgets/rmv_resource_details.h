//=============================================================================
// Copyright (c) 2018-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Header for the resource details widget.
///
/// This is the details widget at the bottom of the resource overview that's
/// shown when a resource is clicked on.
///
//=============================================================================

#ifndef RMV_VIEWS_CUSTOM_WIDGETS_RMV_RESOURCE_DETAILS_H_
#define RMV_VIEWS_CUSTOM_WIDGETS_RMV_RESOURCE_DETAILS_H_

#include <QGraphicsObject>

#include "models/colorizer.h"

static const int kResourceDetailsHeight = 50;

/// @brief Describes a resource details widget.
struct RMVResourceDetailsConfig
{
    uint32_t              width;                 ///< Widget width.
    uint32_t              height;                ///< Widget height.
    RmtResource           resource;              ///< A copy of the resource.
    bool                  resource_valid;        ///< Is the resource above valid.
    bool                  allocation_thumbnail;  ///< Should the allocation rendering on the left be included?
    const rmv::Colorizer* colorizer;             ///< The colorizer used to color this widget.
};

/// @brief Container class for resource details which is rendered in the resource overview and allocation delta pane.
class RMVResourceDetails : public QGraphicsObject
{
    Q_OBJECT

public:
    /// @brief Constructor.
    ///
    /// @param [in] config A configuration struct for this object.
    explicit RMVResourceDetails(const RMVResourceDetailsConfig& config);

    /// @brief Destructor.
    virtual ~RMVResourceDetails();

    /// @brief Implementation of Qt's bounding volume for this item.
    ///
    /// @return The item's bounding rectangle.
    virtual QRectF boundingRect() const Q_DECL_OVERRIDE;

    /// @brief Implementation of Qt's paint for this item.
    ///
    /// @param [in] painter The painter object to use.
    /// @param [in] item    Provides style options for the item, such as its state, exposed area and its level-of-detail hints.
    /// @param [in] widget  Points to the widget that is being painted on if specified.
    virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* item, QWidget* widget) Q_DECL_OVERRIDE;

    /// @brief Update the current resource.
    ///
    /// @param [in] resource The resource to update.
    void UpdateResource(const RmtResource* resource);

    /// @brief Set width and height of the QGraphicsView being drawn into.
    ///
    /// @param [in] width  The width.
    /// @param [in] height The height.
    void UpdateDimensions(int width, int height);

    /// @brief Get the resource being displayed.
    ///
    /// @return The resource.
    const RmtResource* GetResource() const;

private:
    RMVResourceDetailsConfig config_;  ///< Description of this widget.
};

#endif  // RMV_VIEWS_CUSTOM_WIDGETS_RMV_RESOURCE_DETAILS_H_
