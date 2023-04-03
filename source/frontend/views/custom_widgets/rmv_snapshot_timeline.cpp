//=============================================================================
// Copyright (c) 2017-2023 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Implementation of the snapshot timeline visualization.
//=============================================================================

#include "views/custom_widgets/rmv_snapshot_timeline.h"

#include <QDebug>
#include <QMenu>
#include <QContextMenuEvent>
#include <QApplication>
#include <QStyle>

#include "qt_common/utils/common_definitions.h"
#include "qt_common/utils/qt_util.h"
#include "qt_common/utils/scaling_manager.h"

#include "util/time_util.h"

static const int kDefaultMarkerWidth = 25;

RMVSnapshotTimeline::RMVSnapshotTimeline(QWidget* parent)
    : TimelineView(parent)
    , timeline_graph_(nullptr)
{
}

RMVSnapshotTimeline::~RMVSnapshotTimeline()
{
}

void RMVSnapshotTimeline::mousePressEvent(QMouseEvent* event)
{
    TimelineView::mousePressEvent(event);

    emit UpdateSelectedDuration(selected_end_clock_ - selected_start_clock_);
    emit UpdateHoverClock(last_hovered_clock_);
    emit UpdateZoomButtonsForZoomToSelection(RegionSelected());
}

void RMVSnapshotTimeline::mouseMoveEvent(QMouseEvent* event)
{
    TimelineView::mouseMoveEvent(event);
    UpdateToolTip(event->pos());

    emit UpdateSelectedDuration(selected_end_clock_ - selected_start_clock_);
    emit UpdateHoverClock(last_hovered_clock_);
    emit UpdateZoomButtonsForZoomToSelection(RegionSelected());
}

void RMVSnapshotTimeline::leaveEvent(QEvent* event)
{
    Q_UNUSED(event);

    // If the mouse leaves the view, hide the tooltip.
    if (timeline_graph_ != nullptr)
    {
        timeline_tooltip_.HideToolTip();
    }
}

RMVSnapshotMarker* RMVSnapshotTimeline::AddSnapshot(RmtSnapshotPoint* snapshot_point)
{
    RMVSnapshotMarkerConfig config = {};
    config.width                   = kDefaultMarkerWidth;
    config.height                  = height() - kDefaultRulerHeight;
    config.snapshot_point          = snapshot_point;

    TimelineItem       content_object = {};
    RMVSnapshotMarker* marker         = new RMVSnapshotMarker(config);
    marker->SetState(kSnapshotStateViewed);

    content_object.item  = marker;
    content_object.clock = snapshot_point->timestamp;

    content_object.item->setY(ScalingManager::Get().Scaled(kDefaultRulerHeight));
    content_.push_back(content_object);

    scene_->addItem(content_object.item);

    UpdateScene();

    return dynamic_cast<RMVSnapshotMarker*>(content_object.item);
}

RMVTimelineGraph* RMVSnapshotTimeline::AddTimelineGraph(rmv::TimelineModel* timeline_model, rmv::TimelineColorizer* colorizer)
{
    RMVTimelineGraphConfig config = {};
    config.model_data             = timeline_model;
    config.colorizer              = colorizer;
    config.width                  = 25;
    config.height                 = height() - kDefaultRulerHeight;

    timeline_model_             = timeline_model;
    TimelineItem content_object = {};
    content_object.item         = new RMVTimelineGraph(config);

    content_object.item->setY(ScalingManager::Get().Scaled(kDefaultRulerHeight));
    content_object.item->setX(100);

    content_.push_back(content_object);

    scene_->addItem(content_object.item);

    UpdateScene();

    timeline_graph_ = dynamic_cast<RMVTimelineGraph*>(content_object.item);
    timeline_tooltip_.CreateToolTip(scene_, true);
    return timeline_graph_;
}

void RMVSnapshotTimeline::SelectSnapshot(const RmtSnapshotPoint* snapshot_point)
{
    for (int32_t i = 0; i < content_.size(); i++)
    {
        RMVSnapshotMarker* current_marker = dynamic_cast<RMVSnapshotMarker*>(content_[i].item);

        if (current_marker != nullptr)
        {
            if (current_marker->GetSnapshotPoint() == snapshot_point)
            {
                current_marker->SetSelected(true);
            }
            else
            {
                current_marker->SetSelected(false);
            }

            current_marker->update();
        }
    }
}

void RMVSnapshotTimeline::Clear()
{
    for (int i = 0; i < content_.size(); i++)
    {
        scene_->removeItem(content_[i].item);
    }

    content_.clear();

    UpdateScene();
}

void RMVSnapshotTimeline::ClearSnapshotMarkers()
{
    QVector<int32_t> snapshot_markers;  // Vector of all objects in the scene added by child implementations.

    for (int current_index = 0; current_index < content_.size(); ++current_index)
    {
        RMVSnapshotMarker* current_marker = dynamic_cast<RMVSnapshotMarker*>(content_[current_index].item);
        if (current_marker == nullptr)
            continue;

        scene_->removeItem(content_[current_index].item);
        snapshot_markers.push_back(current_index);
    }

    for (int32_t current_snapshot_marker_index = snapshot_markers.size() - 1; current_snapshot_marker_index >= 0; --current_snapshot_marker_index)
    {
        int32_t remove_index = snapshot_markers[current_snapshot_marker_index];
        content_.remove(remove_index);
    }

    UpdateScene();
}

void RMVSnapshotTimeline::contextMenuEvent(QContextMenuEvent* event)
{
    QGraphicsView::contextMenuEvent(event);

    QAction action("Add snapshot at " + rmv::time_util::ClockToTimeUnit(last_hovered_clock_));

    QMenu menu;
    menu.addAction(&action);

    QAction* menu_action = menu.exec(event->globalPos());

    if (menu_action != nullptr)
    {
        emit GenerateSnapshotAtTime(last_hovered_clock_);
    }

    // Swallow the event so we don't pass it out to parent controls.
    event->accept();
}

void RMVSnapshotTimeline::wheelEvent(QWheelEvent* event)
{
    Qt::KeyboardModifiers keyboard_modifiers = QApplication::keyboardModifiers();

    if (keyboard_modifiers & Qt::ControlModifier)
    {
        const int delta = event->angleDelta().y();
        if (delta < 0)
        {
            bool zoom = ZoomOutMousePosition();
            emit UpdateZoomButtonsForZoomOut(zoom);
        }
        else
        {
            bool zoom = ZoomInMousePosition();
            emit UpdateZoomButtonsForZoomIn(zoom);
        }

#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
        UpdateToolTip(event->position());
#else
        UpdateToolTip(event->pos());
#endif
    }
    else
    {
        QGraphicsView::wheelEvent(event);
    }
}

void RMVSnapshotTimeline::resizeEvent(QResizeEvent* event)
{
    TimelineView::resizeEvent(event);

    UpdateScene();
}

void RMVSnapshotTimeline::UpdateContent()
{
    UpdateSnapshotMarkers();
}

void RMVSnapshotTimeline::UpdateSnapshotMarkers()
{
    for (int32_t i = 0; i < content_.size(); i++)
    {
        RMVSnapshotMarker* marker = dynamic_cast<RMVSnapshotMarker*>(content_[i].item);

        if (marker != nullptr)
        {
            marker->UpdateDimensions(ScalingManager::Get().Scaled(kDefaultMarkerWidth), height());
            marker->setX(ClockToSceneCoordinate(content_[i].clock));
            marker->setY(ScalingManager::Get().Scaled(kDefaultRulerHeight));
        }

        RMVTimelineGraph* allocation = dynamic_cast<RMVTimelineGraph*>(content_[i].item);
        if (allocation != nullptr)
        {
            allocation->setX(ClockToSceneCoordinate(ViewableStartClk()));
            allocation->setY(ScalingManager::Get().Scaled(kDefaultRulerHeight));
            allocation->UpdateDimensions(width(), BasePosY() - ScalingManager::Get().Scaled(kDefaultRulerHeight));
        }
    }
}

void RMVSnapshotTimeline::UpdateTimeUnits(TimeUnitType time_unit, double time_to_clock_ratio)
{
    ruler_config_.unit_type           = time_unit;
    ruler_config_.time_to_clock_ratio = time_to_clock_ratio;
    UpdateScene();

    // Update the time values below the timeline.
    emit UpdateSelectedDuration(selected_end_clock_ - selected_start_clock_);
    emit UpdateHoverClock(last_hovered_clock_);
}

void RMVSnapshotTimeline::UpdateToolTip(const QPointF& mouse_pos)
{
    if (timeline_graph_ != nullptr)
    {
        const QPointF scene_pos         = mapToScene(mouse_pos.x(), mouse_pos.y());
        const int     scroll_bar_height = qApp->style()->pixelMetric(QStyle::PM_ScrollBarExtent);
        if (timeline_graph_->isUnderMouse())
        {
            QString text_string;
            QString color_string;
            qreal   x_pos = static_cast<qreal>(mouse_pos.x()) / static_cast<qreal>(width());
            if (timeline_model_->GetTimelineTooltipInfo(x_pos, text_string, color_string))
            {
                timeline_tooltip_.SetText(text_string);
                timeline_tooltip_.SetColors(color_string);
                timeline_tooltip_.UpdateToolTip(mouse_pos, scene_pos, width(), height() - scroll_bar_height);
            }
        }
        else
        {
            timeline_tooltip_.HideToolTip();
        }
    }
}