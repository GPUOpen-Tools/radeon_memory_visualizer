//=============================================================================
/// Copyright (c) 2017-2020 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Implementation of a tree map view
//=============================================================================

#include "views/custom_widgets/rmv_tree_map_view.h"

#include <QDebug>

#include "qt_common/utils/qt_util.h"
#include "qt_common/utils/scaling_manager.h"

#include "rmt_assert.h"

#include "models/message_manager.h"

static const uint32_t kViewMargin = 8;

RMVTreeMapView::RMVTreeMapView(QWidget* parent)
    : QGraphicsView(parent)
    , tree_map_models_(nullptr)
    , overview_model_(nullptr)
{
    setMouseTracking(true);

    setFrameStyle(QFrame::NoFrame);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    scene_ = new QGraphicsScene();
    setScene(scene_);

    RMVTreeMapBlocksConfig config = {};

    blocks_ = new RMVTreeMapBlocks(config);

    scene_->addItem(blocks_);
}

RMVTreeMapView::~RMVTreeMapView()
{
}

void RMVTreeMapView::mousePressEvent(QMouseEvent* event)
{
    QGraphicsView::mousePressEvent(event);
}

void RMVTreeMapView::mouseMoveEvent(QMouseEvent* event)
{
    QGraphicsView::mouseMoveEvent(event);

#if 0
    QPoint mouseCoords = mapFromGlobal(QCursor::pos());
    QPointF sceneCoords = mapToScene(mouseCoords);
    qDebug() << "mouse: " << mouseCoords;
    qDebug() << "sceneCoords: " << sceneCoords;
    qDebug() << "---------";
#endif
}

void RMVTreeMapView::resizeEvent(QResizeEvent* event)
{
    QGraphicsView::resizeEvent(event);

    const uint32_t view_width  = width();
    const uint32_t view_height = height();

    const QRectF scene_rect = QRectF(1, 1, view_width - kViewMargin, view_height - kViewMargin);
    scene_->setSceneRect(scene_rect);
    blocks_->UpdateDimensions(view_width - kViewMargin, view_height - kViewMargin);

    UpdateTreeMap();
}

void RMVTreeMapView::SetModels(const rmv::ResourceOverviewModel* overview_model, const TreeMapModels* tree_map_models, const Colorizer* colorizer)
{
    overview_model_  = overview_model;
    tree_map_models_ = tree_map_models;
    blocks_->SetColorizer(colorizer);
}

void RMVTreeMapView::UpdateTreeMap()
{
    RMT_ASSERT(tree_map_models_ != nullptr);
    RMT_ASSERT(overview_model_ != nullptr);
    if (tree_map_models_ != nullptr && overview_model_ != nullptr)
    {
        blocks_->GenerateTreemap(overview_model_, *tree_map_models_, width() - kViewMargin, height() - kViewMargin);
        blocks_->update();
    }
}

void RMVTreeMapView::Reset()
{
    blocks_->Reset();
}

void RMVTreeMapView::SelectResource(RmtResourceIdentifier resource_identifier)
{
    blocks_->SelectResource(resource_identifier);
}

void RMVTreeMapView::UpdateColorCache()
{
    blocks_->update();
}

RMVTreeMapBlocks* RMVTreeMapView::BlocksWidget()
{
    return blocks_;
}

void RMVTreeMapView::UpdateSliceTypes(const QVector<RMVTreeMapBlocks::SliceType>& slice_types)
{
    blocks_->UpdateSliceTypes(slice_types);
}
