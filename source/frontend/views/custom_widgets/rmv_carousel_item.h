//=============================================================================
/// Copyright (c) 2019-2020 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Header for the carousel item widget base class.
//=============================================================================

#ifndef RMV_VIEWS_CUSTOM_WIDGETS_RMV_CAROUSEL_ITEM_H_
#define RMV_VIEWS_CUSTOM_WIDGETS_RMV_CAROUSEL_ITEM_H_

#include <QGraphicsObject>
#include <QPainter>

#include "models/carousel_model.h"

static const int    kCarouselItemHeight      = 275;
static const int    kCarouselItemWidth       = 450;
static const QColor kCarouselBaseColor       = QColor(240, 240, 240);
static const QColor kDefaultCarouselBarColor = QColor(127, 127, 127);

/// Enum for the carousel type.
enum CarouselDataType
{
    kCarouselDataTypeRegular,
    kCarouselDataTypeDelta
};

/// Configuration struct for carousel.
struct RMVCarouselConfig
{
    int              width;      ///< Width.
    int              height;     ///< Height.
    CarouselDataType data_type;  ///< Either a regular carousel, or a delta carousel.
};

/// Class to describe an item on the carousel.
class RMVCarouselItem : public QGraphicsObject
{
    Q_OBJECT

public:
    /// Constructor.
    explicit RMVCarouselItem(const RMVCarouselConfig& config);

    /// Destructor.
    virtual ~RMVCarouselItem();

    /// Implementation of Qt's bounding volume for this item.
    /// \return The item's bounding rectangle.
    virtual QRectF boundingRect() const Q_DECL_OVERRIDE;

    /// Update the dimensions of this object.
    /// \param width The new width.
    /// \param height The new height.
    void UpdateDimensions(int width, int height);

    /// Set the UI data.
    /// \param data The data from the model.
    virtual void SetData(const RMVCarouselData& data) = 0;

protected:
    /// Helper func to draw a carousel box with a title.
    /// \param painter The Qt painter.
    /// \param title How to name it.
    void DrawCarouselBaseComponents(QPainter* painter, const QString& title) const;

    /// Helper function to draw a horizontal carousel bar item with text.
    /// \param painter The Qt painter.
    /// \param bar_title The title of the bar. If empty string, title will not be displayed.
    /// \param x_pos The x position of the bar, relative to the parent carousel box.
    /// \param y_pos The y position of the bar, relative to the parent carousel box.
    /// \param bar_length The length of the bar (horizontally).
    /// \param bar_width The width of the bar (the vertical height of the bar).
    /// \param value The value to display.
    /// \param max The maximum value.
    /// \param show_summary If true, show a summary of the value and max values.
    void DrawHorizontalBarComponent(QPainter*      painter,
                                    const QString& bar_title,
                                    uint32_t       x_pos,
                                    uint32_t       y_pos,
                                    uint32_t       bar_length,
                                    uint32_t       bar_width,
                                    int64_t        value,
                                    int64_t        max,
                                    bool           show_summary) const;

    /// Helper function to draw a horizontal carousel bar item with text.
    /// \param painter The Qt painter.
    /// \param bar_title The title of the bar. If empty string, title will not be displayed.
    /// \param bar_color The color of the bar if a single snapshot. Compare color will override this.
    /// \param x_pos The x position of the bar, relative to the parent carousel box.
    /// \param y_pos The y position of the bar, relative to the parent carousel box.
    /// \param bar_length The length of the bar (horizontally).
    /// \param bar_width The width of the bar (the vertical height of the bar).
    /// \param value The value to display.
    /// \param max The maximum value.
    /// \param show_summary If true, show a summary of the value and max values.
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
