//=============================================================================
/// Copyright (c) 2020 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Implementation of RMV's custom timeline tooltip
//=============================================================================

#include "views/custom_widgets/rmv_timeline_tooltip.h"

RMVTimelineTooltip::RMVTimelineTooltip(QGraphicsItem* parent)
    : QGraphicsSimpleTextItem(parent)
    , text_height_(0)
    , icon_size_(0)
{
}

RMVTimelineTooltip::~RMVTimelineTooltip()
{
}

QRectF RMVTimelineTooltip::boundingRect() const
{
    QRectF rect  = QGraphicsSimpleTextItem::boundingRect();
    qreal  width = rect.width();
    rect.setWidth(width + icon_size_);
    return rect;
}

void RMVTimelineTooltip::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    painter->setFont(font());

    int offset = 0;
    for (auto index = 0; index < tooltip_data_.size(); index++)
    {
        painter->fillRect(0, offset + 1, icon_size_, icon_size_, tooltip_data_[index].color);
        painter->drawText(text_height_, icon_size_ + offset, tooltip_data_[index].text);
        offset += text_height_;
    }
}

void RMVTimelineTooltip::SetData(const QList<TooltipInfo>& tooltip_info_list)
{
    // set the text string for the base class
    QString text;
    for (int loop = 0; loop < tooltip_info_list.size(); loop++)
    {
        if (loop > 0)
        {
            text += "\n";
        }
        text += tooltip_info_list[loop].text;
    }
    setText(text);

    tooltip_data_ = tooltip_info_list;

    int height = boundingRect().height();

    text_height_ = height / tooltip_data_.size();
    icon_size_   = text_height_ - 2;
}
