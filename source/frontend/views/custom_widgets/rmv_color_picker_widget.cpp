//=============================================================================
/// Copyright (c) 2017-2020 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief Implementation of a color picker widget.
//=============================================================================

#include "views/custom_widgets/rmv_color_picker_widget.h"

#include <QDebug>
#include <QPushButton>
#include <QPainter>

#include "qt_common/utils/scaling_manager.h"

const static QString kDefaultPaletteString("#000,#111,#222,#333,#444,#555,#666,#777,#888,#999,#AAA,#BBB,#CCC,#DDD,#EEE,#FFF");
static const int     kDefaultPaletteSize     = 16;
static const int     kDefaultPaletteColumns  = 4;
static const int     kDefaultPaletteRows     = 4;
static const int     kDefaultButtonDimension = 60;

RMVColorPickerWidget::RMVColorPickerWidget(QWidget* parent)
    : QWidget(parent)
    , palette_(kDefaultPaletteSize)
    , grid_layout_(this)
    , button_group_(this)
    , grid_row_count_(0)
    , grid_column_count_(0)
{
    // Setup grid layout
    grid_layout_.setSpacing(0);
    grid_layout_.setContentsMargins(0, 0, 0, 0);
    setSizePolicy(QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Fixed);

    // Initial dimensions
    SetRowAndColumnCount(kDefaultPaletteRows, kDefaultPaletteColumns);
    SetPalette(kDefaultPaletteString);

    // Set up signals/slots
    connect(&button_group_, SIGNAL(buttonClicked(int)), this, SLOT(ButtonClicked(int)));

    connect(&ScalingManager::Get(), &ScalingManager::ScaleFactorChanged, this, &QWidget::updateGeometry);
}

RMVColorPickerWidget::~RMVColorPickerWidget()
{
    disconnect(&ScalingManager::Get(), &ScalingManager::ScaleFactorChanged, this, &QWidget::updateGeometry);
}

void RMVColorPickerWidget::SetRowAndColumnCount(unsigned int rows, unsigned int columns)
{
    // Store dimension values
    grid_row_count_    = rows;
    grid_column_count_ = columns;

    // Generate a new set of buttons
    GenerateButtons();
}

QColor RMVColorPickerWidget::GetSelectedColor()
{
    return palette_.GetColor(button_group_.checkedId());
}

int RMVColorPickerWidget::GetSelectedPaletteId()
{
    return button_group_.checkedId();
}

ColorPalette RMVColorPickerWidget::GetPalette()
{
    return palette_;
}

void RMVColorPickerWidget::Select(int id)
{
    button_group_.button(id)->setChecked(true);
}

void RMVColorPickerWidget::SetPalette(const ColorPalette& palette)
{
    palette_ = palette;

    SetButtonColors();
    update();

    // Indicate the palette has changed
    emit PaletteChanged(palette_);
}

void RMVColorPickerWidget::GenerateButtons()
{
    // Delete any previous buttons
    while (QLayoutItem* item = grid_layout_.takeAt(0))
    {
        if (item->widget() != nullptr)
        {
            delete item->widget();
        }

        delete item;
    }

    // Button size policy
    QSizePolicy size_policy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    size_policy.setHorizontalStretch(0);
    size_policy.setVerticalStretch(0);

    // Generate a button for each grid spot
    for (int i = 0; i < grid_row_count_; i++)
    {
        for (int j = 0; j < grid_column_count_; j++)
        {
            PickerButton* button = new PickerButton(this);
            size_policy.setHeightForWidth(true);
            button->setSizePolicy(size_policy);
            button->setCheckable(true);
            button->setCursor(QCursor(Qt::PointingHandCursor));

            // Add button to containers
            button_group_.addButton(button);
            grid_layout_.addWidget(button, i, j, 1, 1);
        }
    }

    // Initialize button colors
    SetButtonColors();
}

void RMVColorPickerWidget::SetButtonColors()
{
    // Set id and color for all buttons
    for (int button_id = 0; button_id < button_group_.buttons().size(); button_id++)
    {
        PickerButton* button = static_cast<PickerButton*>(button_group_.buttons()[button_id]);
        button_group_.setId(button, button_id);

        // Set button color
        button->SetColor(palette_.GetColor(button_id));
    }
}

void RMVColorPickerWidget::ButtonClicked(int button_id)
{
    emit ColorSelected(button_id, palette_.GetColor(button_id));
}

RMVColorPickerWidget::PickerButton::PickerButton(QWidget* parent)
    : QPushButton(parent)
{
    connect(&ScalingManager::Get(), &ScalingManager::ScaleFactorChanged, this, &QPushButton::updateGeometry);
}

RMVColorPickerWidget::PickerButton::~PickerButton()
{
    disconnect(&ScalingManager::Get(), &ScalingManager::ScaleFactorChanged, this, &QPushButton::updateGeometry);
}

void RMVColorPickerWidget::PickerButton::SetColor(const QColor& color)
{
    button_color_ = color;
}

int RMVColorPickerWidget::PickerButton::heightForWidth(int width) const
{
    return width;
}

QSize RMVColorPickerWidget::PickerButton::sizeHint() const
{
    return ScalingManager::Get().Scaled(minimumSizeHint());
}

QSize RMVColorPickerWidget::PickerButton::minimumSizeHint() const
{
    return QSize(kDefaultButtonDimension, kDefaultButtonDimension);
}

void RMVColorPickerWidget::PickerButton::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);

    ScalingManager& sm = ScalingManager::Get();
    QPainter        painter(this);

    const int pos_adj       = sm.Scaled(1);
    const int size_adj      = pos_adj * 2;
    const int outline_width = sm.Scaled(2);

    // Rectangle used for drawing button and its border
    QRect r1(pos_adj, pos_adj, this->size().width() - size_adj, this->size().height() - size_adj);

    const int left     = r1.left() + outline_width;
    const int right    = left + r1.width() - (outline_width * 2);
    const int top      = r1.top() + outline_width;
    const int bottom   = top + r1.height() - (outline_width * 2);
    const int center_x = (left + right) / 2;
    const int center_y = (top + bottom) / 2;

    if (this->isChecked() || this->underMouse())
    {
        // Fill rect with black to form border
        painter.fillRect(r1, QBrush(Qt::black));

        QPolygon polygon;
        QPolygon polygon2;

        // Determine polygon shapes based on button status
        if (this->isChecked())
        {
            polygon << QPoint(center_x, top) << QPoint(right, top) << QPoint(right, bottom) << QPoint(left, bottom) << QPoint(left, center_y);

            polygon2 << QPoint(center_x, top) << QPoint(right - 1, top) << QPoint(right - 1, bottom - 1) << QPoint(left, bottom - 1) << QPoint(left, center_y);
        }
        else if (this->underMouse())
        {
            polygon << QPoint(left, top) << QPoint(right, top) << QPoint(right, bottom) << QPoint(left, bottom);

            polygon2 << QPoint(left, top) << QPoint(right - 1, top) << QPoint(right - 1, bottom - 1) << QPoint(left, bottom - 1);
        }

        // Draw colored polygon
        QPainterPath path;
        path.addPolygon(polygon);
        painter.fillPath(path, QBrush(button_color_));

        // Draw white interior border
        painter.setPen(QPen(Qt::white, 1));
        painter.drawPolygon(polygon2);
    }
    else
    {
        // Fill rect with black to form border
        painter.fillRect(r1, button_color_);
    }
}
