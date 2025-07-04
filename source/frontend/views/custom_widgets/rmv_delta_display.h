//=============================================================================
// Copyright (c) 2018-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Header for a graphics view that implements data delta.
//=============================================================================

#ifndef RMV_VIEWS_CUSTOM_WIDGETS_RMV_DELTA_DISPLAY_H_
#define RMV_VIEWS_CUSTOM_WIDGETS_RMV_DELTA_DISPLAY_H_

#include <QGraphicsScene>
#include <QGraphicsView>

#include "views/custom_widgets/rmv_delta_display_widget.h"

static const int kHeapDeltaWidgetHeight = 80;

/// @brief Generic structure designed to hold either +/- data for an item.
struct DeltaItem
{
    /// Default constructor.
    DeltaItem()
        : type(kDeltaValueTypeString)
        , graphic(false)
        , value_num(0)
    {
    }

    /// Constructor.
    ///
    /// @param [in] in_name         The item name.
    /// @param [in] in_type         The item type.
    /// @param [in] in_graphic      Whether to use a rendered graphic.
    /// @param [in] in_value_num    The value number.
    /// @param [in] in_value_string The value string.
    /// @param [in] in_custom_color The custom color.
    DeltaItem(const QString& in_name,
              DeltaValueType in_type,
              bool           in_graphic,
              int64_t        in_value_num,
              const QString& in_value_string,
              const QColor&  in_custom_color)
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

/// @brief Encapsulates data used to render an individual delta component.
struct DeltaComponent
{
    /// Constructor.
    DeltaComponent()
        : description(nullptr)
        , widget(nullptr)
    {
    }

    DeltaItem              item_data;    ///< Backing data.
    QGraphicsTextItem*     description;  ///< Qt item to render its text.
    RMVDeltaDisplayWidget* widget;       ///< Custom Qt widget.
};

/// @brief Graphics view that is aware of resize and mouse events.
class RMVDeltaDisplay : public QGraphicsView
{
    Q_OBJECT

public:
    /// @brief Constructor.
    ///
    /// @param [in] parent The parent widget.
    explicit RMVDeltaDisplay(QWidget* parent);

    /// @brief Destructor.
    virtual ~RMVDeltaDisplay();

    /// @brief Capture a resize event.
    ///
    /// @param [in] event The resize event.
    virtual void resizeEvent(QResizeEvent* event) Q_DECL_OVERRIDE;

    /// @brief Add a new item.
    ///
    /// @param [in] title        The delta title.
    /// @param [in] item         The new item.
    /// @param [in] width_scaler How much to scale spacing by.
    void Init(const QString& title, const QVector<DeltaItem>& items, float width_scaler = 1.0F);

    /// @brief Update a single item based on name.
    ///
    /// @param [in] item The item to update.
    void UpdateItem(const DeltaItem& item);

private:
    /// @brief Update the widget based on the color theme.
    void OnColorThemeUpdated();

    /// @brief Update view dimensions.
    void UpdateDimensions();

    /// @brief Get the font used by the checkbox.
    ///
    /// @return The checkbox font.
    QFont GetFont() const;

    QGraphicsScene          scene_;   ///< The scene containing the delta objects.
    QGraphicsTextItem       title_;   ///< The title of these delta objects.
    QVector<DeltaComponent> deltas_;  ///< The list of delta components.
};

#endif  // RMV_VIEWS_CUSTOM_WIDGETS_RMV_DELTA_DISPLAY_H_
