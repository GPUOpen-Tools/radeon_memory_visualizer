//=============================================================================
// Copyright (c) 2019-2021 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Header for a widget that shows how memory usage changes per
/// process over time.
//=============================================================================

#ifndef RMV_VIEWS_CUSTOM_WIDGETS_RMV_TIMELINE_GRAPH_H_
#define RMV_VIEWS_CUSTOM_WIDGETS_RMV_TIMELINE_GRAPH_H_

#include <QGraphicsObject>

#include "models/timeline/timeline_colorizer.h"
#include "models/timeline/timeline_model.h"
#include "util/definitions.h"

/// @brief Describes the data needed for the timeline.
struct RMVTimelineGraphConfig
{
    int                     width;       ///< Widget width.
    int                     height;      ///< Widget height.
    rmv::TimelineModel*     model_data;  ///< Pointer to the timeline model.
    rmv::TimelineColorizer* colorizer;   ///< Pointer to the timeline colorizer.
};

/// @brief Container class for a widget which shows how memory allocations change per process over time.
class RMVTimelineGraph : public QGraphicsObject
{
    Q_OBJECT

public:
    /// @brief Constructor.
    ///
    /// @param [in] config The configuration for this widget.
    explicit RMVTimelineGraph(const RMVTimelineGraphConfig& config);

    /// @brief Destructor.
    virtual ~RMVTimelineGraph();

    /// Implementation of Qt's bounding volume for this item.
    ///
    /// @return The item's bounding rectangle.
    virtual QRectF boundingRect() const Q_DECL_OVERRIDE;

    /// @brief Implementation of Qt's paint for this item.
    ///
    /// @param [in] painter The painter object to use.
    /// @param [in] option  Provides style options for the item, such as its state, exposed area and its level-of-detail hints.
    /// @param [in] widget  Points to the widget that is being painted on if specified.
    virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) Q_DECL_OVERRIDE;

    /// @brief Update the dimensions of this widget.
    ///
    /// This widget is such that it is the same size as the view and placed in the scene where it is always visible.
    ///
    /// @param [in] width  The new width.
    /// @param [in] height The new height.
    void UpdateDimensions(int width, int height);

private:
    /// @brief Get scaled width.
    ///
    /// @return scaled width.
    int32_t ScaledWidth() const;

    /// @brief Get scaled height.
    ///
    /// @return scaled height.
    int32_t ScaledHeight() const;

    RMVTimelineGraphConfig config_;  ///< Description of this widget.
};

#endif  // RMV_VIEWS_CUSTOM_WIDGETS_RMV_TIMELINE_GRAPH_H_
