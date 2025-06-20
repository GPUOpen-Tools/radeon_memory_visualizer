//=============================================================================
// Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Implementation of an allocation graphics object.
//=============================================================================

#include "views/custom_widgets/rmv_allocation_bar.h"

#include <QGraphicsSceneEvent>

#include "rmt_util.h"

#include "managers/message_manager.h"

static const int kTitleFontSize              = 8;
static const int kDefaultWidth               = 300;
static const int kDefaultBarPadding          = 5;
static const int kDefaultAllocationBarHeight = 50;
static const int kMinResourceHeight          = 4;  ///< The minimum height on screen a resource should be, in pixels.

RMVAllocationBar::RMVAllocationBar(rmv::AllocationBarModel* model, int32_t allocation_index, int32_t model_index, const rmv::Colorizer* colorizer)
    : model_(model)
    , allocation_index_(allocation_index)
    , model_index_(model_index)
    , colorizer_(colorizer)
    , item_width_(0)
    , item_height_(0)
    , max_bar_width_(0)
    , allocation_bar_height_(kDefaultAllocationBarHeight)

{
    setAcceptHoverEvents(true);

    title_font_.setPointSizeF(kTitleFontSize);
    title_font_.setBold(true);

    description_font_.setPointSizeF(kTitleFontSize);
    description_font_.setBold(false);

    UpdateDimensions(kDefaultWidth, allocation_bar_height_);
}

RMVAllocationBar::~RMVAllocationBar()
{
}

QRectF RMVAllocationBar::boundingRect() const
{
    return QRectF(0, 0, item_width_, item_height_);
}

void RMVAllocationBar::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    // In extreme cases, some optimizations are needed if allocations contain more resources than can
    // physically fit on screen.
    // In the case where there is no aliasing, an allocation could have thousands of small resources ie
    // shader pipelines. In this case, if a resource overlaps a previously rendered resource in screen
    // space, it is ignored (resources would be overlapped anyway and would only occupy a single pixel)
    // In the case of aliasing, resources that are aliased are stacked on top of each other. In extreme
    // cases, these stacks can be hundreds of resources high, and subsequently, each row of a stack could
    // be less than a pixel. In this case, a minimum height for a row is chosen (currently 4 pixels),
    // and any overlapping rows are ignored. Not all resources are shown in both these cases, but the
    // resources can be explored in more detail in the allocation explorer pane.
    Q_UNUSED(option);
    Q_UNUSED(widget);

    const RmtVirtualAllocation* allocation = model_->GetAllocation(allocation_index_, model_index_);
    if (allocation == nullptr || max_bar_width_ <= 0)
    {
        return;
    }

    double scaled_bar_y_offset = kDefaultBarPadding;

    // Draw the details if necessary.
    if (model_->ShowDetails())
    {
        // Measure title text.
        QString      title_text   = model_->GetTitleText(allocation_index_, model_index_);
        QFontMetrics font_metrics = QFontMetrics(title_font_);
        QSize        title_size   = font_metrics.size(0, title_text);

        const int width  = title_size.width();
        const int height = title_size.height();

        // Draw title text.
        painter->setFont(title_font_);
        painter->drawText(0, height, title_text);

        // Draw description text.
        painter->setFont(description_font_);
        QString description_string = model_->GetDescriptionText(allocation_index_, model_index_);
        painter->drawText(width, height, description_string);

        // Update bar y offset based on the font size.
        scaled_bar_y_offset += height;
    }

    // Calculate width of the current allocation bar.
    // May be affected by Normalization in GetBytesPerPixel.
    const uint64_t allocation_size      = RmtVirtualAllocationGetSizeInBytes(allocation);
    const double   bytes_per_pixel      = model_->GetBytesPerPixel(allocation_index_, model_index_, max_bar_width_);
    const int      allocation_bar_width = (double)allocation_size / bytes_per_pixel;

    // Paint the background first. Needs to be colored based on the coloring mode.
    QColor background_brush_color = colorizer_->GetColor(allocation, nullptr);
    painter->setPen(Qt::NoPen);
    painter->setBrush(background_brush_color);
    painter->drawRect(0, scaled_bar_y_offset, allocation_bar_width, allocation_bar_height_);

    // Now paint all the resources on top.
    int num_rows = model_->GetNumRows(allocation);
    if (num_rows > 0)
    {
        // Calculate the resource height, based on the minimum.
        double resource_height = allocation_bar_height_;
        resource_height /= num_rows;

        int scale = 1;
        if (resource_height < kMinResourceHeight)
        {
            scale = kMinResourceHeight / resource_height;
            resource_height *= scale;
        }

        const int resource_count = allocation->resource_count;

        // If there's more rows that there's room for, make a larger array.
        // This is needed when switching allocations.
        if (num_rows > static_cast<int>(offset_array_.size()))
        {
            offset_array_.resize(num_rows);
        }

        // Clear out the array. Needs to be done on each redraw.
        std::fill(offset_array_.begin(), offset_array_.end(), 0);

        for (int current_resource_index = 0; current_resource_index < resource_count; current_resource_index++)
        {
            const RmtResource* resource = allocation->resources[current_resource_index];

            if (resource->resource_type == kRmtResourceTypeHeap)
            {
                continue;
            }

            int row = model_->GetRowForResourceAtIndex(allocation, current_resource_index);

            // Calculate the size of the resource.
            const uint64_t offset_in_bytes    = RmtResourceGetOffsetFromBoundAllocation(resource);
            const int      x_pos              = (double)offset_in_bytes / bytes_per_pixel;
            const int      resource_bar_width = RMT_MAXIMUM(1, (double)resource->size_in_bytes / bytes_per_pixel);

            // Don't render any rows that overlap with previously displayed rows.
            if ((row % scale) != 0)
            {
                continue;
            }

            if (row > 1)
            {
                row /= scale;
            }

            // Don't render another resource if it overlaps the last one in screen space.
            if (x_pos + resource_bar_width <= offset_array_[row])
            {
                continue;
            }

            offset_array_[row] = x_pos + resource_bar_width + 1;
            double y_offset    = scaled_bar_y_offset + (resource_height * row);

            // Render the resource.
            QPen resource_border_pen(Qt::black);
            if (current_resource_index == model_->GetSelectedResourceForAllocation(allocation_index_, model_index_))
            {
                resource_border_pen.setWidth(2);
            }
            else
            {
                resource_border_pen.setWidth(1);
            }
            painter->setPen(resource_border_pen);

            QColor resource_background_brush_color = colorizer_->GetColor(resource->bound_allocation, resource);

            if (current_resource_index == model_->GetHoveredResourceForAllocation(allocation_index_, model_index_))
            {
                resource_background_brush_color = resource_background_brush_color.darker(rmv::kHoverDarkenColor);
            }

            Qt::BrushStyle style = ((RmtResourceGetAliasCount(resource) > 0) ? Qt::BrushStyle::Dense1Pattern : Qt::BrushStyle::SolidPattern);
            const QBrush   curr_brush(resource_background_brush_color, style);
            painter->setBrush(curr_brush);
            painter->drawRect(x_pos, y_offset, resource_bar_width + 1, resource_height);
        }
    }

    // Render border around the whole allocation.
    painter->setPen(QColor(0, 0, 0));
    painter->setBrush(Qt::NoBrush);
    painter->drawRect(0, scaled_bar_y_offset, allocation_bar_width, allocation_bar_height_);
}

void RMVAllocationBar::UpdateDimensions(const int width, const int height)
{
    const QString      title_text   = model_->GetTitleText(allocation_index_, model_index_);
    const QFontMetrics font_metrics = QFontMetrics(title_font_);
    const QSize        title_size   = font_metrics.size(0, title_text);

    const int title_height   = title_size.height();
    const int scaled_padding = kDefaultBarPadding;

    // Let the scene know that this object is changing sizes.
    prepareGeometryChange();

    // Update the width.
    item_width_    = width;
    max_bar_width_ = item_width_ - scaled_padding;

    Q_ASSERT(height != -1);

    // This will calculate bar_height_ based on the scaled font size and supplied overall height.

    // Overall item_height_ needs no adjusting since the user is specifying it.
    item_height_ = height;

    // Calculate the bar height based on what remains after accounting for:
    // the title, padding, (the bar), and another padding.
    // This should be similar, but reversed, math to what is done above.

    // These are the items that have a fixed height.
    int fixed_height = scaled_padding + scaled_padding;

    if (model_->ShowDetails())
    {
        fixed_height += title_height;
    }

    // The supplied height must be greater than these fixed heights in order for the bar
    // to get displayed at all.
    if (height > fixed_height)
    {
        allocation_bar_height_ = item_height_ - fixed_height;
    }
    else
    {
        Q_ASSERT(height > fixed_height);
        allocation_bar_height_ = 0;
    }
}

void RMVAllocationBar::hoverMoveEvent(QGraphicsSceneHoverEvent* event)
{
    qreal scaled_bar_y_offset = kDefaultBarPadding;
    if (model_->ShowDetails())
    {
        // Measure title text.
        const QString      title_text   = model_->GetTitleText(allocation_index_, model_index_);
        const QFontMetrics font_metrics = QFontMetrics(title_font_);
        const QSize        title_size   = font_metrics.size(0, title_text);

        // Update bar y offset based on the font size.
        const int height = title_size.height();
        scaled_bar_y_offset += height;
    }

    QPointF mouse_pos = QPointF(event->pos().x(), event->pos().y() - scaled_bar_y_offset);
    if (mouse_pos.y() >= 0)
    {
        setCursor(Qt::PointingHandCursor);
        model_->SetHoveredResourceForAllocation(allocation_index_, model_index_, max_bar_width_, allocation_bar_height_, mouse_pos);
    }
    else
    {
        setCursor(Qt::ArrowCursor);
        model_->SetHoveredResourceForAllocation(allocation_index_, -1, model_index_);
    }
    update();
}

void RMVAllocationBar::hoverLeaveEvent(QGraphicsSceneHoverEvent* event)
{
    Q_UNUSED(event);

    model_->SetHoveredResourceForAllocation(allocation_index_, -1, model_index_);
    update();
}

void RMVAllocationBar::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    Q_UNUSED(event);
    model_->SetSelectedResourceForAllocation(allocation_index_, -1, model_index_);

    RmtResourceIdentifier resource_identifier = model_->FindResourceIdentifier(allocation_index_, model_index_);
    {
        emit ResourceSelected(resource_identifier, false);
    }
}

void RMVAllocationBar::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event)
{
    Q_UNUSED(event);

    const RmtVirtualAllocation* allocation = model_->GetAllocation(allocation_index_, model_index_);
    if (allocation == nullptr)
    {
        return;
    }

    int32_t selected_resource = model_->GetSelectedResourceForAllocation(allocation_index_, model_index_);

    if (selected_resource >= 0)
    {
        // Selected a resource so emit a signal indicating so.
        Q_ASSERT(allocation->resources[selected_resource]);
        RmtResourceIdentifier resource_id = allocation->resources[selected_resource]->identifier;

        emit ResourceSelected(resource_id, true);
    }
    else
    {
        // Didn't find a resource so probably clicked on an unbound area, so select the allocation and
        // navigate to the allocation explorer.
        emit rmv::MessageManager::Get().UnboundResourceSelected(allocation);
        emit rmv::MessageManager::Get().PaneSwitchRequested(rmv::kPaneIdSnapshotAllocationExplorer);
    }
}
