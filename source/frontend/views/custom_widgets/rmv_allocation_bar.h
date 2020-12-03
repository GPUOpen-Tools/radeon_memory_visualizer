//=============================================================================
/// Copyright (c) 2020 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Header for an allocation graphics object
//=============================================================================

#ifndef RMV_VIEWS_CUSTOM_WIDGETS_RMV_ALLOCATION_BAR_H_
#define RMV_VIEWS_CUSTOM_WIDGETS_RMV_ALLOCATION_BAR_H_

#include <QGraphicsObject>
#include <QFont>

#include "rmt_types.h"

#include "models/allocation_bar_model.h"
#include "util/definitions.h"
#include "views/colorizer.h"

/// Container class for a memory block widget.
class RMVAllocationBar : public QGraphicsObject
{
    Q_OBJECT

public:
    /// Constructor.
    /// \param model The underlying model holding the backend data.
    /// \param allocation_index The index of the allocation in the model containing the raw allocation data.
    /// \param model_index The allocation model index this graphic item refers to (for panes with multiple allocation displays).
    /// \param colorizer The colorizer used to color this widget.
    explicit RMVAllocationBar(rmv::AllocationBarModel* model, int32_t allocation_index, int32_t model_index, const Colorizer* colorizer);

    /// Destructor.
    virtual ~RMVAllocationBar();

    /// Implementation of Qt's bounding volume for this item.
    /// \return The item's bounding rectangle.
    virtual QRectF boundingRect() const Q_DECL_OVERRIDE;

    /// Implementation of Qt's paint for this item.
    /// \param painter The painter object to use.
    /// \param option Provides style options for the item, such as its state, exposed area and its level-of-detail hints.
    /// \param widget Points to the widget that is being painted on if specified.
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* item, QWidget* widget) Q_DECL_OVERRIDE;

    /// Mouse hover over event.
    /// \param event the QGraphicsSceneHoverEvent.
    void hoverMoveEvent(QGraphicsSceneHoverEvent* event) Q_DECL_OVERRIDE;

    /// Mouse hover leave event.
    /// \param event the QGraphicsSceneHoverEvent.
    void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) Q_DECL_OVERRIDE;

    /// Mouse press event.
    /// \param event the QGraphicsSceneHoverEvent.
    void mousePressEvent(QGraphicsSceneMouseEvent* event) Q_DECL_OVERRIDE;

    /// Mouse double click event.
    /// \param event the QGraphicsSceneHoverEvent.
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) Q_DECL_OVERRIDE;

    /// Set Dimensions of the item.
    /// Since the text has a fixed height, this method has the desired effect
    /// of controlling the maximum width of the bar, and the height of the bar.
    /// \param width The new mamximum width.
    /// \param height The new total height.
    /// height of the items adjusted for DPI scaling.
    void UpdateDimensions(const int width, const int height);

signals:
    /// signal that a resource has been selected.
    /// \param resource_identifier The selected resource.
    /// \param navigate_to_pane If true, indicate that navigation to another pane is requested.
    /// It is up the the slot to decide which pane to navigate to.
    void ResourceSelected(RmtResourceIdentifier resource_Identifier, bool navigate_to_pane);

private:
    rmv::AllocationBarModel* model_;             ///< The underlying model holding the backend data.
    int32_t                  allocation_index_;  ///< The index of this object in the scene.
    int32_t                  model_index_;       ///< The allocation model index this graphic item refers to (for panes with multiple allocation displays).
    const Colorizer*         colorizer_;         ///< The colorizer used to color this widget.
    QFont                    title_font_;        ///< Font used for painting the title.
    QFont                    description_font_;  ///< Font used for painting the description.

    int item_width_;             ///< Pixel width of this item (ie: bounding rect width), see UpdateDimensions().
    int item_height_;            ///< Pixel height of this item (ie: bounding rect height), see UpdateDimensions().
    int max_bar_width_;          ///< Maximum bar width after accounting for the bar padding along the right side, see UpdateDimensions().
    int allocation_bar_height_;  ///< Pixel height of the allocation bar; already includes scaling factor, see UpdateDimensions().
};

#endif  // RMV_VIEWS_CUSTOM_WIDGETS_RMV_ALLOCATION_BAR_H_
