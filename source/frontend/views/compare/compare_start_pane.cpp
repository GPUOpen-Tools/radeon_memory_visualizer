//=============================================================================
// Copyright (c) 2018-2021 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Implementation of Compare start pane.
//=============================================================================

#include "views/compare/compare_start_pane.h"

#include "qt_common/utils/scaling_manager.h"

#include "rmt_data_snapshot.h"

#include "managers/message_manager.h"
#include "managers/pane_manager.h"
#include "settings/rmv_settings.h"
#include "util/widget_util.h"

static const qreal kCircleSeparationFactor = 5.0;
static const qreal kSceneMargin            = 10.0;

CompareStartPane::CompareStartPane(QWidget* parent)
    : BasePane(parent)
    , ui_(new Ui::CompareStartPane)
{
    ui_->setupUi(this);

    rmv::widget_util::ApplyStandardPaneStyle(this, ui_->main_content_, ui_->main_scroll_area_);

    rmv::widget_util::InitGraphicsView(ui_->graphics_view_, ScalingManager::Get().Scaled(kCircleDiameter));
    ui_->graphics_view_->setFixedWidth(ScalingManager::Get().Scaled(kCircleDiameter * 2 - kCircleDiameter / kCircleSeparationFactor));

    scene_ = new QGraphicsScene();
    ui_->graphics_view_->setScene(scene_);

    RMVCameraSnapshotWidgetConfig config = {};
    config.height                        = ui_->graphics_view_->height();
    config.width                         = ui_->graphics_view_->width();
    config.margin                        = kSceneMargin;

    config.base_color     = rmv::RMVSettings::Get().GetColorSnapshotViewed();
    snapshot_widget_left_ = new RMVCameraSnapshotWidget(config);

    config.base_color      = rmv::RMVSettings::Get().GetColorSnapshotCompared();
    snapshot_widget_right_ = new RMVCameraSnapshotWidget(config);

    scene_->addItem(snapshot_widget_left_);
    scene_->addItem(snapshot_widget_right_);

    UpdateCirclePositions();

    // Respond to the Navigate() signal from the snapshot widgets. Uses a lambda to emit a navigate-to-pane message.
    connect(snapshot_widget_left_, &RMVCameraSnapshotWidget::Navigate, [=]() {
        emit rmv::MessageManager::Get().PaneSwitchRequested(rmv::kPaneIdTimelineGenerateSnapshot);
    });
    connect(snapshot_widget_right_, &RMVCameraSnapshotWidget::Navigate, [=]() {
        emit rmv::MessageManager::Get().PaneSwitchRequested(rmv::kPaneIdTimelineGenerateSnapshot);
    });

    connect(&ScalingManager::Get(), &ScalingManager::ScaleFactorChanged, this, &CompareStartPane::OnScaleFactorChanged);
}

CompareStartPane::~CompareStartPane()
{
    disconnect(&ScalingManager::Get(), &ScalingManager::ScaleFactorChanged, this, &CompareStartPane::OnScaleFactorChanged);
}

void CompareStartPane::resizeEvent(QResizeEvent* event)
{
    UpdateCirclePositions();
    QWidget::resizeEvent(event);
}

void CompareStartPane::OnScaleFactorChanged()
{
    UpdateCirclePositions();
}

void CompareStartPane::UpdateCirclePositions()
{
    const qreal circle_diameter = kCircleDiameter - kSceneMargin * 2;

    snapshot_widget_left_->UpdateDimensions(circle_diameter, circle_diameter);
    snapshot_widget_right_->UpdateDimensions(circle_diameter, circle_diameter);
    qreal overlap_start = kSceneMargin + ScalingManager::Get().Scaled(circle_diameter);
    snapshot_widget_right_->setPos(overlap_start - (overlap_start / kCircleSeparationFactor), 0);

    QRectF scene_rect = scene_->itemsBoundingRect();
    ui_->graphics_view_->setSceneRect(scene_rect);
    ui_->graphics_view_->setFixedSize(scene_rect.toRect().size());
}

void CompareStartPane::Reset()
{
    snapshot_widget_left_->UpdateName("Current snapshot");
    snapshot_widget_left_->update();

    snapshot_widget_right_->UpdateName("Load comparison trace");
    snapshot_widget_right_->update();
}

void CompareStartPane::ChangeColoring()
{
    snapshot_widget_left_->UpdateBaseColor(rmv::RMVSettings::Get().GetColorSnapshotViewed());
    snapshot_widget_right_->UpdateBaseColor(rmv::RMVSettings::Get().GetColorSnapshotCompared());
}

void CompareStartPane::OpenSnapshot(RmtDataSnapshot* snapshot)
{
    snapshot_widget_left_->UpdateName(QString(snapshot->name));
}

void CompareStartPane::SetEmptyTitleText()
{
    ui_->title_text_->setText("The snapshots chosen for comparison are empty!");
}
