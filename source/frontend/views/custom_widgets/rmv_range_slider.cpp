//=============================================================================
// Copyright (c) 2024 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Implementation of the custom range slider widget.
//=============================================================================

#include "views/custom_widgets/rmv_range_slider.h"
#include "rmt_constants.h"
#include "util/rmv_util.h"
#include "util/string_util.h"

#include <QHBoxLayout>
#include <QLabel>

RmvRangeSlider::RmvRangeSlider(QWidget* parent)
    : DoubleSliderWidget(parent)
    , range_value_label_(nullptr)
{
}

RmvRangeSlider::~RmvRangeSlider()
{
}

void RmvRangeSlider::Init()
{
    DoubleSliderWidget::Init();

    // Create a container with a layout for the slider and value label.
    // Replace the slider on the UI with this container.
    QWidget* container = new QWidget(parentWidget());
    container->setObjectName("rmv_range_slider_container_");
    container->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    QHBoxLayout* layout = new QHBoxLayout(container);
    layout->setObjectName("rmv_range_slider_layout_");
    range_value_label_ = new RmvFixedWidthLabel(container);
    range_value_label_->setObjectName("range_value_label_");

    // Build a formatted string with the maximum expected width.
    // The label will reserve this much horizontal space in the layout so that the slider to the left isn't affected when the value string changes length.
    QString widest_range_string =
        rmv::string_util::LocalizedValueMemory(999, false, false, false) + " - " + rmv::string_util::LocalizedValueMemory(999, false, false, false);
    range_value_label_->SetWidestTextString(widest_range_string);
    layout->addWidget(range_value_label_);
    parentWidget()->layout()->replaceWidget(this, container);
    layout->insertWidget(0, this);

    // Initialize the label range values to match the slider.
    UpdateValues(LowerPosition(), UpperPosition());

    connect(this, &DoubleSliderWidget::SpanChanged, this, &RmvRangeSlider::UpdateValues);
}

void RmvRangeSlider::UpdateValues(const int min_value, const int max_value)
{
    if (range_value_label_ != nullptr)
    {
        const uint64_t lower_range = rmv_util::CalculateSizeThresholdFromStepValue(min_value, rmv::kSizeSliderRange - 1);
        const uint64_t upper_range = rmv_util::CalculateSizeThresholdFromStepValue(max_value, rmv::kSizeSliderRange - 1);
        range_value_label_->setText(rmv::string_util::GetMemoryRangeString(lower_range, upper_range));
    }
}
