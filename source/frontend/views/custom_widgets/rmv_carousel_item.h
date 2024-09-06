//=============================================================================
// Copyright (c) 2019-2024 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Header for the carousel item widget base class.
//=============================================================================

#ifndef RMV_VIEWS_CUSTOM_WIDGETS_RMV_CAROUSEL_ITEM_H_
#define RMV_VIEWS_CUSTOM_WIDGETS_RMV_CAROUSEL_ITEM_H_

#include <QGraphicsObject>
#include <QPainter>

#include "models/carousel_model.h"

static const int    kCarouselItemHeight      = 275;
static const int    kCarouselItemWidth       = 450;
static const QColor kDefaultCarouselBarColor = QColor(127, 127, 127);

/// @brief Enum for the carousel type.
enum CarouselDataType
{
    kCarouselDataTypeRegular,
    kCarouselDataTypeDelta
};

/// @brief Configuration struct for carousel.
struct RMVCarouselConfig
{
    int              width;      ///< Width.
    int              height;     ///< Height.
    CarouselDataType data_type;  ///< Either a regular carousel, or a delta carousel.
};

/// @brief Class to describe an item on the carousel.
class RMVCarouselItem : public QGraphicsObject
{
    Q_OBJECT

public:
    /// @brief Constructor.
    explicit RMVCarouselItem(const RMVCarouselConfig& config);

    /// @brief Destructor.
    virtual ~RMVCarouselItem();

    /// @brief Implementation of Qt's bounding volume for this item.
    ///
    /// @return The item's bounding rectangle.
    virtual QRectF boundingRect() const Q_DECL_OVERRIDE;

    /// @brief Update the dimensions of this object.
    ///
    /// @param [in] width The new width.
    /// @param [in] height The new height.
    void UpdateDimensions(int width, int height);

    /// @brief Set the UI data.
    ///
    /// @param [in] data The data from the model.
    virtual void SetData(const rmv::RMVCarouselData& data) = 0;

protected:
    /// @brief Helper func to draw a carousel box with a title.
    ///
    /// @param [in] painter The Qt painter.
    /// @param [in] title How to name it.
    void DrawCarouselBaseComponents(QPainter* painter, const QString& title) const;

    /// @brief Helper function to draw a horizontal carousel bar item with text.
    ///
    /// @param [in] painter      The Qt painter.
    /// @param [in] bar_title    The title of the bar. If empty string, title will not be displayed.
    /// @param [in] x_pos        The x position of the bar, relative to the parent carousel box.
    /// @param [in] y_pos        The y position of the bar, relative to the parent carousel box.
    /// @param [in] bar_length   The length of the bar (horizontally).
    /// @param [in] bar_width    The width of the bar (the vertical height of the bar).
    /// @param [in] value        The value to display.
    /// @param [in] max          The maximum value.
    /// @param [in] show_summary If true, show a summary of the value and max values.
    void DrawHorizontalBarComponent(QPainter*      painter,
                                    const QString& bar_title,
                                    uint32_t       x_pos,
                                    uint32_t       y_pos,
                                    uint32_t       bar_length,
                                    uint32_t       bar_width,
                                    int64_t        value,
                                    int64_t        max,
                                    bool           show_summary) const;

    /// @brief Helper function to draw a horizontal carousel bar item with text.
    ///
    /// @param [in] painter      The Qt painter.
    /// @param [in] bar_title    The title of the bar. If empty string, title will not be displayed.
    /// @param [in] bar_color    The color of the bar if a single snapshot. Compare color will override this.
    /// @param [in] x_pos        The x position of the bar, relative to the parent carousel box.
    /// @param [in] y_pos        The y position of the bar, relative to the parent carousel box.
    /// @param [in] bar_length   The length of the bar (horizontally).
    /// @param [in] bar_width    The width of the bar (the vertical height of the bar).
    /// @param [in] value        The value to display.
    /// @param [in] max          The maximum value.
    /// @param [in] show_summary If true, show a summary of the value and max values.
    void DrawColoredHorizontalBarComponent(QPainter*      painter,
                                           const QString& bar_title,
                                           const QColor&  bar_color,
                                           uint32_t       x_pos,
                                           uint32_t       y_pos,
                                           uint32_t       bar_length,
                                           uint32_t       bar_width,
                                           int64_t        value,
                                           int64_t        max,
                                           bool           show_summary) const;

    RMVCarouselConfig config_;  ///< The configuration for the carousel.

private:
};

#endif  // RMV_VIEWS_CUSTOM_WIDGETS_RMV_CAROUSEL_ITEM_H_
