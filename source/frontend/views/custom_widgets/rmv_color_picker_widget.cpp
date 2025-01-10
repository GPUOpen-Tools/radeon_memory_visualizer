//=============================================================================
// Copyright (c) 2017-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Implementation of a color picker widget.
//=============================================================================

#include "views/custom_widgets/rmv_color_picker_widget.h"

#include "views/custom_widgets/rmv_color_picker_button.h"

const static QString kDefaultPaletteString("#000,#111,#222,#333,#444,#555,#666,#777,#888,#999,#AAA,#BBB,#CCC,#DDD,#EEE,#FFF");
static const int     kDefaultPaletteSize    = 16;
static const int     kDefaultPaletteColumns = 4;
static const int     kDefaultPaletteRows    = 4;

RMVColorPickerWidget::RMVColorPickerWidget(QWidget* parent)
    : QWidget(parent)
    , palette_(kDefaultPaletteSize)
    , grid_layout_(this)
    , button_group_(this)
    , grid_row_count_(0)
    , grid_column_count_(0)
{
    // Setup grid layout.
    grid_layout_.setSpacing(0);
    grid_layout_.setContentsMargins(0, 0, 0, 0);
    setSizePolicy(QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Fixed);

    // Initial dimensions.
    SetRowAndColumnCount(kDefaultPaletteRows, kDefaultPaletteColumns);
    SetPalette(kDefaultPaletteString);

    // Set up signals/slots.
    connect(&button_group_, &QButtonGroup::idClicked, this, &RMVColorPickerWidget::ButtonClicked);
}

RMVColorPickerWidget::~RMVColorPickerWidget()
{
}

void RMVColorPickerWidget::SetRowAndColumnCount(unsigned int rows, unsigned int columns)
{
    // Store dimension values.
    grid_row_count_    = rows;
    grid_column_count_ = columns;

    // Generate a new set of buttons.
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

    // Indicate the palette has changed.
    emit PaletteChanged(palette_);
}

void RMVColorPickerWidget::GenerateButtons()
{
    // Delete any previous buttons.
    while (QLayoutItem* item = grid_layout_.takeAt(0))
    {
        if (item->widget() != nullptr)
        {
            delete item->widget();
        }

        delete item;
    }

    // Button size policy.
    QSizePolicy size_policy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    size_policy.setHorizontalStretch(0);
    size_policy.setVerticalStretch(0);

    // Generate a button for each grid spot.
    for (int i = 0; i < grid_row_count_; i++)
    {
        for (int j = 0; j < grid_column_count_; j++)
        {
            RMVColorPickerButton* button = new RMVColorPickerButton(this);
            size_policy.setHeightForWidth(true);
            button->setSizePolicy(size_policy);
            button->setCheckable(true);
            button->setCursor(QCursor(Qt::PointingHandCursor));

            // Add button to containers.
            button_group_.addButton(button);
            grid_layout_.addWidget(button, i, j, 1, 1);
        }
    }

    // Initialize button colors.
    SetButtonColors();
}

void RMVColorPickerWidget::SetButtonColors()
{
    // Set id and color for all buttons.
    for (int button_id = 0; button_id < button_group_.buttons().size(); button_id++)
    {
        RMVColorPickerButton* button = dynamic_cast<RMVColorPickerButton*>(button_group_.buttons()[button_id]);
        button_group_.setId(button, button_id);

        if (button != nullptr)
        {
            // Set button color.
            button->SetColor(palette_.GetColor(button_id));
        }
    }
}

void RMVColorPickerWidget::ButtonClicked(int button_id)
{
    emit ColorSelected(button_id, palette_.GetColor(button_id));
}
