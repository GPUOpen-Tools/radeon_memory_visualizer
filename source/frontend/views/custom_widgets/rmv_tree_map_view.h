//=============================================================================
/// Copyright (c) 2017-2020 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Header for a tree map view
//=============================================================================

#ifndef RMV_VIEWS_CUSTOM_WIDGETS_RMV_TREE_MAP_VIEW_H_
#define RMV_VIEWS_CUSTOM_WIDGETS_RMV_TREE_MAP_VIEW_H_

#include <QGraphicsView>
#include <QGraphicsScene>

#include "rmt_resource_list.h"

#include "models/snapshot/resource_overview_model.h"
#include "util/definitions.h"
#include "views/custom_widgets/rmv_tree_map_blocks.h"

/// Holds and controls the entire queue timings visualization.
class RMVTreeMapView : public QGraphicsView
{
    Q_OBJECT

public:
    /// Constructor.
    /// \param parent The parent widget.
    explicit RMVTreeMapView(QWidget* parent);

    /// Destructor.
    virtual ~RMVTreeMapView();

    /// Handle resizing.
    /// \param event The resize event.
    virtual void resizeEvent(QResizeEvent* event) Q_DECL_OVERRIDE;

    /// Detect a mouse press event.
    /// \param event The mouse press event.
    virtual void mousePressEvent(QMouseEvent* event) Q_DECL_OVERRIDE;

    /// Capture a mouse move event.
    /// \param event The mouse move event.
    virtual void mouseMoveEvent(QMouseEvent* event) Q_DECL_OVERRIDE;

    /// Set the models.
    /// \param overview_model The resource overview model.
    /// \param tree_map_models The models needed for the treemap.
    /// \param colorizer The colorizer object to use.
    void SetModels(const rmv::ResourceOverviewModel* overview_model, const TreeMapModels* tree_map_models, const Colorizer* colorizer);

    /// Update the treemap view.
    void UpdateTreeMap();

    /// Reset UI state.
    void Reset();

    /// Update the color cache.
    void UpdateColorCache();

    /// Return the blocks widget for upper-level Qt connection.
    /// \return pointer to RMVTreeMapBlocks.
    RMVTreeMapBlocks* BlocksWidget();

    /// Update slicing types coming in from UI.
    /// \param slice_types types in a vector.
    void UpdateSliceTypes(const QVector<RMVTreeMapBlocks::SliceType>& slice_types);

public slots:
    /// Select a resource.
    /// \param resource_identifier The resource idemtofoer of the resource to select.
    void SelectResource(RmtResourceIdentifier resource_identifier);

private:
    QGraphicsScene*                   scene_;            ///< The graphics scene associated with this view.
    RMVTreeMapBlocks*                 blocks_;           ///< Pointer to the tree map blocks graphics object.
    const TreeMapModels*              tree_map_models_;  ///< The models needed for the tree map.
    const rmv::ResourceOverviewModel* overview_model_;   ///< The resource overview model.
};

#endif  // RMV_VIEWS_CUSTOM_WIDGETS_RMV_TREE_MAP_VIEW_H_
