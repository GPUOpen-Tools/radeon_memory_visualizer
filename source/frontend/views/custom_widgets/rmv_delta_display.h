//=============================================================================
/// Copyright (c) 2018-2020 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief Header for a graphics view that implements data delta
//=============================================================================

#ifndef RMV_VIEWS_CUSTOM_WIDGETS_RMV_DELTA_DISPLAY_H_
#define RMV_VIEWS_CUSTOM_WIDGETS_RMV_DELTA_DISPLAY_H_

#include <QGraphicsView>
#include <QGraphicsScene>

#include "views/custom_widgets/rmv_delta_display_widget.h"

static const int kHeapDeltaWidgetHeight = 80;

/// Generic structure designed to hold either +/- data for an item.
struct DeltaItem
{
    DeltaItem()
        : type(kDeltaValueTypeString)
        , graphic(false)
        , value_num(0)
    {
    }

    DeltaItem(QString in_name, DeltaValueType in_type, bool in_graphic, int64_t in_value_num, QString in_value_string, QColor in_custom_color)
        : name(in_name)
        , type(in_type)
        , graphic(in_graphic)
        , value_num(in_value_num)
        , value_string(in_value_string)
        , custom_color(in_custom_color)
    {
    }

    QString        name;          ///< Component name.
    DeltaValueType type;          ///< Component type (string or value).
    bool           graphic;       ///< Should include a rendered graphic.
    int64_t        value_num;     ///< The value (numeric).
    QString        value_string;  ///< The value (string).
    QColor         custom_color;  ///< Uses a non-standard color.
};

/// Encapsulates data used to render an individual delta component.
struct DeltaComponent
{
    DeltaComponent()
        : description(nullptr)
        , widget(nullptr)
    {
    }

    DeltaItem              item_data;    ///< Backing data.
    QGraphicsTextItem*     description;  ///< Qt item to render its text.
    RMVDeltaDisplayWidget* widget;       ///< Custom Qt widget.
};

/// Graphics view that is aware of resize and mouse events.
class RMVDeltaDisplay : public QGraphicsView
{
    Q_OBJECT

public:
    /// Constructor.
    /// \param parent The parent widget.
    explicit RMVDeltaDisplay(QWidget* parent);

    /// Destructor.
    virtual ~RMVDeltaDisplay();

    /// Capture a resize event.
    /// \param event The resize event.
    virtual void resizeEvent(QResizeEvent* event) Q_DECL_OVERRIDE;

    /// Add a new item.
    /// \param title The delta title.
    /// \param item The new item.
    /// \param width_scaler How much to scale spacing by.
    void Init(const QString& title, const QVector<DeltaItem>& items, float width_scaler = 1.0F);

    /// Update a single item based on name.
    /// \param item The item to update.
    void UpdateItem(const DeltaItem& item);

private:
    /// Update view dimensions.
    void UpdateDimensions();

    /// Get the font used by the checkbox.
    QFont GetFont() const;

    QGraphicsScene          scene_;   ///< The scene containing the delta objects.
    QGraphicsTextItem       title_;   ///< The title of these delta objects.
    QVector<DeltaComponent> deltas_;  ///< The list of delta components.
};

#endif  // RMV_VIEWS_CUSTOM_WIDGETS_RMV_DELTA_DISPLAY_H_
