//=============================================================================
/// Copyright (c) 2018-2020 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Header for a delta display widget
//=============================================================================

#ifndef RMV_VIEWS_CUSTOM_WIDGETS_RMV_DELTA_DISPLAY_WIDGET_H_
#define RMV_VIEWS_CUSTOM_WIDGETS_RMV_DELTA_DISPLAY_WIDGET_H_

#include <QGraphicsObject>
#include <QFont>

/// Enum of delta data types.
enum DeltaValueType
{
    kDeltaValueTypeString,
    kDeltaValueTypeValue,
    kDeltaValueTypeValueLabeled,

    kDeltaValueTypeCount,
};

/// Configuration struct for a widget showing delta value.
struct RMVDeltaDisplayWidgetConfig
{
    RMVDeltaDisplayWidgetConfig()
        : width(0)
        , height(0)
        , graphic(false)
        , type(kDeltaValueTypeString)
        , value_num(0)
    {
    }

    int            width;         ///< Widget width.
    int            height;        ///< Widget height.
    bool           graphic;       ///< Should render a graphic on the left.
    QFont          font;          ///< Text font.
    DeltaValueType type;          ///< Which delta type (string or numeric).
    int64_t        value_num;     ///< Value (numeric).
    QString        value_string;  ///< Value (string).
    QColor         custom_color;  ///< Render a non-standard color.
};

/// Container class for a widget designed to display delta +/- data
class RMVDeltaDisplayWidget : public QGraphicsObject
{
    Q_OBJECT

public:
    /// Constructor.
    /// \param config A configuration struct for this object.
    explicit RMVDeltaDisplayWidget(const RMVDeltaDisplayWidgetConfig& config);

    /// Destructor.
    virtual ~RMVDeltaDisplayWidget();

    /// Implementation of Qt's bounding volume for this item.
    /// \return The item's bounding rectangle.
    virtual QRectF boundingRect() const Q_DECL_OVERRIDE;

    /// Implementation of Qt's paint for this item.
    /// \param painter The painter object to use.
    /// \param item Provides style options for the item, such as its state, exposed area and its level-of-detail hints.
    /// \param widget Points to the widget that is being painted on if specified.
    virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* item, QWidget* widget) Q_DECL_OVERRIDE;

    /// Update dimensions.
    /// \param width The width.
    /// \param height The height.
    void UpdateDimensions(int width, int height);

    /// Update data type.
    /// \param type The type.
    void UpdateDataType(DeltaValueType type);

    /// Update data value.
    /// \param value The value.
    void UpdateDataValueNum(int64_t value);

    /// Update data string.
    /// \param str The string.
    void UpdateDataValueStr(const QString& str);

    /// Update data color.
    /// \param color The color.
    void UpdateDataCustomColor(const QColor& str);

    /// Update data graphic.
    /// \param graphic The graphic.
    void UpdateDataGraphic(bool graphic);

private:
    RMVDeltaDisplayWidgetConfig config_;  ///< Structure holding the data for this widget.
};

#endif  // RMV_VIEWS_CUSTOM_WIDGETS_RMV_DELTA_DISPLAY_WIDGET_H_
