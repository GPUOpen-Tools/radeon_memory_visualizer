//=============================================================================
// Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Implementation of Snapshot start pane.
//=============================================================================

#include "views/snapshot/snapshot_start_pane.h"

#include "managers/message_manager.h"
#include "managers/pane_manager.h"
#include "settings/rmv_settings.h"
#include "util/widget_util.h"

static const qreal kSceneMargin = 10.0;

SnapshotStartPane::SnapshotStartPane(QWidget* parent)
    : BasePane(parent)
    , ui_(new Ui::SnapshotStartPane)
{
    ui_->setupUi(this);

    rmv::widget_util::ApplyStandardPaneStyle(ui_->main_scroll_area_);

    ui_->graphics_view_->setFixedWidth(kCircleDiameter);
    rmv::widget_util::InitGraphicsView(ui_->graphics_view_, kCircleDiameter);

    scene_ = new QGraphicsScene();
    ui_->graphics_view_->setScene(scene_);

    RMVCameraSnapshotWidgetConfig config = {};
    config.height                        = ui_->graphics_view_->height();
    config.width                         = ui_->graphics_view_->width();
    config.margin                        = kSceneMargin;

    config.base_color = rmv::RMVSettings::Get().GetColorSnapshotViewed();
    snapshot_widget_  = new RMVCameraSnapshotWidget(config);

    // The scene will take ownership of snapshot_widget_.
    scene_->addItem(snapshot_widget_);
}

SnapshotStartPane::~SnapshotStartPane()
{
    delete scene_;
}

void SnapshotStartPane::resizeEvent(QResizeEvent* event)
{
    ResizeGraphicsView();
    QWidget::resizeEvent(event);
}

void SnapshotStartPane::ResizeGraphicsView()
{
    const qreal circle_diameter = kCircleDiameter - kSceneMargin * 2.0;

    snapshot_widget_->UpdateDimensions(circle_diameter, circle_diameter);

    QRectF scene_rect = scene_->itemsBoundingRect();
    ui_->graphics_view_->setSceneRect(scene_rect);
    ui_->graphics_view_->setFixedSize(scene_rect.toRect().size());
}

void SnapshotStartPane::Reset()
{
    snapshot_widget_->update();
}

void SnapshotStartPane::ChangeColoring()
{
    snapshot_widget_->UpdateBaseColor(rmv::RMVSettings::Get().GetColorSnapshotViewed());
}

void SnapshotStartPane::OpenSnapshot(RmtDataSnapshot* snapshot)
{
    snapshot_widget_->UpdateName(QString(snapshot->name));
}

void SnapshotStartPane::SetEmptyTitleText()
{
    ui_->title_text_->setText("The selected snapshot is empty!");
}
