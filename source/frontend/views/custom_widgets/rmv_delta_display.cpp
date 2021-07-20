//=============================================================================
// Copyright (c) 2018-2021 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Implementation of a graphics view that implements data delta.
//=============================================================================

#include "views/custom_widgets/rmv_delta_display.h"

#include <QDebug>
#include <QScrollBar>

#include "qt_common/utils/qt_util.h"
#include "qt_common/utils/scaling_manager.h"

static const qreal kDeltaDisplayWidth  = 200.0;
static const qreal kDeltaDisplayHeight = 20.0;

RMVDeltaDisplay::RMVDeltaDisplay(QWidget* parent)
    : QGraphicsView(parent)
{
    setMouseTracking(true);

    setFrameStyle(QFrame::NoFrame);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    verticalScrollBar()->blockSignals(true);
    horizontalScrollBar()->blockSignals(true);

    setFixedHeight(kHeapDeltaWidgetHeight);

    title_.setPlainText("Title");
    title_.setPos(0, 0);

    QFont font = title_.font();
    font.setBold(true);
    title_.setFont(font);
    scene_.addItem(&title_);

    UpdateDimensions();

    setScene(&scene_);
}

RMVDeltaDisplay::~RMVDeltaDisplay()
{
}

void RMVDeltaDisplay::Init(const QString& title, const QVector<DeltaItem>& items, float width_scaler)
{
    title_.setPlainText(title);

    for (int i = 0; i < deltas_.size(); i++)
    {
        RMVDeltaDisplayWidget* widget = deltas_[i].widget;
        if (widget != nullptr)
        {
            scene_.removeItem(widget);
            delete widget;
        }
        QGraphicsTextItem* description = deltas_[i].description;
        if (description != nullptr)
        {
            scene_.removeItem(description);
            delete description;
        }
    }

    deltas_.clear();

    const double scale_factor  = ScalingManager::Get().Scaled(1.0);
    const double y_base_pos    = title.isEmpty() == true ? 12.0 : 25.0;
    const double display_width = kDeltaDisplayWidth * width_scaler;

    for (int i = 0; i < items.size(); i++)
    {
        DeltaComponent deltaComponent = {};
        deltaComponent.item_data      = items[i];

        qreal x_pos = static_cast<qreal>(i) * display_width * scale_factor;

        deltaComponent.description = new QGraphicsTextItem();
        deltaComponent.description->setPlainText(items[i].name);
        deltaComponent.description->setPos(x_pos, y_base_pos * scale_factor);
        scene_.addItem(deltaComponent.description);

        RMVDeltaDisplayWidgetConfig config;
        config.width  = display_width;
        config.height = kDeltaDisplayHeight;
        config.font   = GetFont();

        deltaComponent.widget = new RMVDeltaDisplayWidget(config);
        deltaComponent.widget->setPos(x_pos, (y_base_pos + 25.0) * scale_factor);
        scene_.addItem(deltaComponent.widget);

        deltas_.push_back(deltaComponent);
    }
}

void RMVDeltaDisplay::UpdateItem(const DeltaItem& item)
{
    for (int32_t i = 0; i < deltas_.size(); i++)
    {
        if (item.name.compare(deltas_[i].item_data.name) == 0)
        {
            deltas_[i].description->setPlainText(item.name);
            deltas_[i].widget->UpdateDataType(item.type);
            deltas_[i].widget->UpdateDataValueNum(item.value_num);
            deltas_[i].widget->UpdateDataValueString(item.value_string);
            deltas_[i].widget->UpdateDataCustomColor(item.custom_color);
            deltas_[i].widget->UpdateDataGraphic(item.graphic);

            break;
        }
    }
}

void RMVDeltaDisplay::resizeEvent(QResizeEvent* event)
{
    QGraphicsView::resizeEvent(event);

    UpdateDimensions();
}

void RMVDeltaDisplay::UpdateDimensions()
{
    scene_.setSceneRect(0, 0, width(), height());
}

QFont RMVDeltaDisplay::GetFont() const
{
    const double scaling_factor = ScalingManager::Get().Scaled(1.0);

    QFont font;
    font.setPixelSize(11 * scaling_factor);

    return font;
}
