//=============================================================================
/// Copyright (c) 2020 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Header for RMV's custom timeline tooltip
//=============================================================================

#ifndef RMV_VIEWS_CUSTOM_WIDGETS_RMV_TIMELINE_TOOLTIP_H_
#define RMV_VIEWS_CUSTOM_WIDGETS_RMV_TIMELINE_TOOLTIP_H_

#include <QGraphicsItem>
#include <QPainter>
#include <QColor>

struct TooltipInfo
{
    QString text;
    QColor  color;
};

/// Container class for the carousel's memory types component.
class RMVTimelineTooltip : public QGraphicsSimpleTextItem
{
public:
    /// Constructor.
    explicit RMVTimelineTooltip(QGraphicsItem* parent = nullptr);

    /// Destructor.
    virtual ~RMVTimelineTooltip();

    /// Qt's overridden boundingRect method.
    QRectF boundingRect() const override;

    /// Qt's overridden paint method.
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

    /// Set the text and colors for the tooltip.
    /// \param tooltip_info_list A list of structs containing the tooltip data (text and color swatches).
    void SetData(const QList<TooltipInfo>& tooltip_info_list);

private:
    QList<TooltipInfo> tooltip_data_;  ///< A list containing the data for each tooltip item.
    int                text_height_;   ///< The text height.
    int                icon_size_;     ///< The icon size.
};

#endif  // RMV_VIEWS_CUSTOM_WIDGETS_RMV_TIMELINE_TOOLTIP_H_
