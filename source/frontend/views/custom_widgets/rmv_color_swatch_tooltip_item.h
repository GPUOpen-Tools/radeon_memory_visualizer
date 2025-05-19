//=============================================================================
// Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Header for a custom tooltip comprising of a multiline text
/// string with a color swatch before each line of text.
///
/// Implemented on top of QGraphicsSimpleTextItem and uses the base class
/// setText() to store the text and the custom color data is set in setData().
///
//=============================================================================

#ifndef RMV_VIEWS_CUSTOM_WIDGETS_RMV_COLOR_SWATCH_TOOLTIP_ITEM_H_
#define RMV_VIEWS_CUSTOM_WIDGETS_RMV_COLOR_SWATCH_TOOLTIP_ITEM_H_

#include <QColor>
#include <QGraphicsItem>
#include <QPainter>

/// @brief Container class for the custom tooltip with color swatch.
class RMVColorSwatchTooltipItem : public QGraphicsSimpleTextItem
{
public:
    /// @brief Constructor.
    ///
    /// @param [in] parent The parent widget.
    explicit RMVColorSwatchTooltipItem(QGraphicsItem* parent = nullptr);

    /// @brief Destructor.
    virtual ~RMVColorSwatchTooltipItem();

    /// @brief Qt's overridden boundingRect method.
    ///
    /// @return The item's bounding rectangle.
    virtual QRectF boundingRect() const Q_DECL_OVERRIDE;

    /// @brief Qt's overridden paint method.
    ///
    /// @param [in] painter The painter object to use.
    /// @param [in] option  Provides style options for the item, such as its state, exposed area and its level-of-detail hints.
    /// @param [in] widget  Points to the widget that is being painted on if specified.
    virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) Q_DECL_OVERRIDE;

private:
    int icon_size_;  ///< The icon size.
};

#endif  // RMV_VIEWS_CUSTOM_WIDGETS_RMV_COLOR_SWATCH_TOOLTIP_ITEM_H_
