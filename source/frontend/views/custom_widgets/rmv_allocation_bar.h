//=============================================================================
// Copyright (c) 2020-2024 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Header for an allocation graphics object.
//=============================================================================

#ifndef RMV_VIEWS_CUSTOM_WIDGETS_RMV_ALLOCATION_BAR_H_
#define RMV_VIEWS_CUSTOM_WIDGETS_RMV_ALLOCATION_BAR_H_

#include <QGraphicsObject>
#include <QFont>

#include "models/allocation_bar_model.h"
#include "models/colorizer.h"
#include "util/definitions.h"

/// Container class for a memory block widget.
class RMVAllocationBar : public QGraphicsObject
{
    Q_OBJECT

public:
    /// @brief Constructor.
    ///
    /// @param [in] model            The underlying model holding the backend data.
    /// @param [in] allocation_index The index of the allocation in the model containing the raw allocation data.
    /// @param [in] model_index      The allocation model index this graphic item refers to (for panes with multiple allocation displays).
    /// @param [in] colorizer        The colorizer used to color this widget.
    explicit RMVAllocationBar(rmv::AllocationBarModel* model, int32_t allocation_index, int32_t model_index, const rmv::Colorizer* colorizer);

    /// @brief Destructor.
    virtual ~RMVAllocationBar();

    /// @brief Implementation of Qt's bounding volume for this item.
    ///
    /// @return The item's bounding rectangle.
    virtual QRectF boundingRect() const Q_DECL_OVERRIDE;

    /// @brief Implementation of Qt's paint for this item.
    ///
    /// @param [in] painter The painter object to use.
    /// @param [in] option  Provides style options for the item, such as its state, exposed area and its level-of-detail hints.
    /// @param [in] widget  Points to the widget that is being painted on if specified.
    virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* item, QWidget* widget) Q_DECL_OVERRIDE;

    /// @brief Mouse hover over event.
    ///
    /// @param [in] event The QGraphicsSceneHoverEvent event object.
    virtual void hoverMoveEvent(QGraphicsSceneHoverEvent* event) Q_DECL_OVERRIDE;

    /// @brief Mouse hover leave event.
    ///
    /// @param [in] event The QGraphicsSceneHoverEvent event object.
    virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) Q_DECL_OVERRIDE;

    /// @brief Mouse press event.
    ///
    /// @param [in] event The QGraphicsSceneMouseEvent event object.
    virtual void mousePressEvent(QGraphicsSceneMouseEvent* event) Q_DECL_OVERRIDE;

    /// @brief Mouse double click event.
    ///
    /// @param [in] event the QGraphicsSceneMouseEvent event object.
    virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) Q_DECL_OVERRIDE;

    /// @brief Set Dimensions of the item.
    ///
    /// Since the text has a fixed height, this method has the desired effect
    /// of controlling the maximum width of the bar, and the height of the bar.
    ///
    /// @param [in] width  The new maximum width.
    /// @param [in] height The new total height of the items adjusted for DPI scaling.
    void UpdateDimensions(const int width, const int height);

signals:
    /// @brief Signal that a resource has been selected.
    ///
    /// It is up the the slot to decide which pane to navigate to.
    ///
    /// @param [in] resource_identifier The selected resource.
    /// @param [in] navigate_to_pane    If true, indicate that navigation to another pane is requested.
    void ResourceSelected(RmtResourceIdentifier resource_Identifier, bool navigate_to_pane);

private:
    rmv::AllocationBarModel* model_;             ///< The underlying model holding the backend data.
    int32_t                  allocation_index_;  ///< The index of this object in the scene.
    int32_t                  model_index_;       ///< The allocation model index this graphic item refers to (for panes with multiple allocation displays).
    const rmv::Colorizer*    colorizer_;         ///< The colorizer used to color this widget.
    QFont                    title_font_;        ///< Font used for painting the title.
    QFont                    description_font_;  ///< Font used for painting the description.

    int item_width_;             ///< Pixel width of this item (ie: bounding rect width), see UpdateDimensions().
    int item_height_;            ///< Pixel height of this item (ie: bounding rect height), see UpdateDimensions().
    int max_bar_width_;          ///< Maximum bar width after accounting for the bar padding along the right side, see UpdateDimensions().
    int allocation_bar_height_;  ///< Pixel height of the allocation bar; already includes scaling factor, see UpdateDimensions().
};

#endif  // RMV_VIEWS_CUSTOM_WIDGETS_RMV_ALLOCATION_BAR_H_
