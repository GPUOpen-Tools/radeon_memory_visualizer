//=============================================================================
// Copyright (c) 2017-2024 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Header for a tree map view.
//=============================================================================

#ifndef RMV_VIEWS_CUSTOM_WIDGETS_RMV_TREE_MAP_VIEW_H_
#define RMV_VIEWS_CUSTOM_WIDGETS_RMV_TREE_MAP_VIEW_H_

#include <QGraphicsView>
#include <QGraphicsScene>

#include "models/snapshot/resource_overview_model.h"
#include "util/definitions.h"
#include "views/custom_widgets/rmv_tree_map_blocks.h"
#include "views/custom_widgets/rmv_tooltip.h"

/// @brief Holds and controls the entire queue timings visualization.
class RMVTreeMapView : public QGraphicsView
{
    Q_OBJECT

public:
    /// @brief Constructor.
    ///
    /// @param [in] parent The parent widget.
    explicit RMVTreeMapView(QWidget* parent);

    /// @brief Destructor.
    virtual ~RMVTreeMapView();

    /// @brief Handle resizing.
    ///
    /// @param [in] event The resize event.
    virtual void resizeEvent(QResizeEvent* event) Q_DECL_OVERRIDE;

    /// @brief Detect a mouse press event.
    ///
    /// @param [in] event The mouse press event.
    virtual void mousePressEvent(QMouseEvent* event) Q_DECL_OVERRIDE;

    /// @brief Capture a mouse move event.
    ///
    /// @param [in] event The mouse move event.
    virtual void mouseMoveEvent(QMouseEvent* event) Q_DECL_OVERRIDE;

    /// @brief Event triggered when the mouse is no longer over the view.
    ///
    /// @param [in] event Pointer to the event object.
    virtual void leaveEvent(QEvent* event) Q_DECL_OVERRIDE;

    /// @brief Set the models.
    ///
    /// @param [in] overview_model  The resource overview model.
    /// @param [in] tree_map_models The models needed for the treemap.
    /// @param [in] colorizer       The colorizer object to use.
    void SetModels(const rmv::ResourceOverviewModel* overview_model, const TreeMapModels* tree_map_models, const rmv::Colorizer* colorizer);

    /// @brief Update the treemap view.
    void UpdateTreeMap();

    /// @brief Reset UI state.
    void Reset();

    /// @brief Update the color cache.
    void UpdateColorCache();

    /// @brief Return the blocks widget for upper-level Qt connection.
    ///
    /// @return pointer to RMVTreeMapBlocks.
    RMVTreeMapBlocks* BlocksWidget();

    /// @brief Update slicing types coming in from UI.
    ///
    /// @param [in] slice_types types in a vector.
    void UpdateSliceTypes(const QVector<RMVTreeMapBlocks::SliceType>& slice_types);

    /// @brief Update the tool tip.
    ///
    /// Make sure the tool tip contains the correct data for what is currently under the mouse position.
    ///
    /// @param [in] mouse_pos The mouse position in the view.
    void UpdateToolTip(const QPoint& mouse_pos);

public slots:
    /// @brief Select a resource.
    ///
    /// @param [in] resource_identifier The resource identifier of the resource to select.
    void SelectResource(RmtResourceIdentifier resource_identifier);

private:
    QGraphicsScene*                   scene_;             ///< The graphics scene associated with this view.
    RMVTreeMapBlocks*                 blocks_;            ///< Pointer to the tree map blocks graphics object.
    const TreeMapModels*              tree_map_models_;   ///< The models needed for the tree map.
    const rmv::ResourceOverviewModel* overview_model_;    ///< The resource overview model.
    RMVTooltip                        resource_tooltip_;  ///< The tooltip on the timeline.
};

#endif  // RMV_VIEWS_CUSTOM_WIDGETS_RMV_TREE_MAP_VIEW_H_
