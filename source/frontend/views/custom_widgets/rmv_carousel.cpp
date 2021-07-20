//=============================================================================
// Copyright (c) 2018-2021 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Implementation for the carousel.
//=============================================================================

#include "views/custom_widgets/rmv_carousel.h"

#include "qt_common/utils/scaling_manager.h"

#include "views/custom_widgets/rmv_carousel_memory_footprint.h"
#include "views/custom_widgets/rmv_carousel_allocation_sizes.h"
#include "views/custom_widgets/rmv_carousel_memory_types.h"
#include "views/custom_widgets/rmv_carousel_resource_types.h"

static const int kNavButtonWidth = 30;

RMVCarousel::RMVCarousel(const RMVCarouselConfig& config)
    : config_(config)
    , carousel_index_(1)
{
    scene_ = new QGraphicsScene();

    RMVCarouselConfig item_config = {};
    item_config.width             = 0;
    item_config.height            = config_.height;
    item_config.data_type         = config_.data_type;

    model_ = new rmv::CarouselModel();

    // Add items to the scene. The scene takes ownership of the items
    // with addItem() so no need to delete these objects.
    left_nav_button_ = new RMVCarouselNavButton(item_config.width, item_config.height, true);
    scene_->addItem(left_nav_button_);

    right_nav_button_ = new RMVCarouselNavButton(item_config.width, item_config.height, false);
    scene_->addItem(right_nav_button_);

    info_text_ = scene_->addText("");
    info_text_->setPos(0, item_config.height - 20.0);

    // Don't show the carousel counter (for now).
    info_text_->hide();

    // Add the carousel widgets.
    CreateCarouselItem<RMVCarouselMemoryFootprint>(item_config);
    CreateCarouselItem<RMVCarouselResourceTypes>(item_config);
    RMVCarouselMemoryTypes* virtual_memory  = CreateCarouselItem<RMVCarouselMemoryTypes>(item_config);
    RMVCarouselMemoryTypes* physical_memory = CreateCarouselItem<RMVCarouselMemoryTypes>(item_config);
    CreateCarouselItem<RMVCarouselAllocationSizes>(item_config);

    // Set the heap types required for the heap carousel items.
    virtual_memory->SetIsPhysicalHeap(false);
    physical_memory->SetIsPhysicalHeap(true);

    connect(left_nav_button_, &RMVCarouselNavButton::PressedButton, this, &RMVCarousel::MoveCarousel);
    connect(right_nav_button_, &RMVCarouselNavButton::PressedButton, this, &RMVCarousel::MoveCarousel);
}

RMVCarousel::~RMVCarousel()
{
    delete scene_;
    delete model_;
}

void RMVCarousel::MoveCarousel(bool left_direction)
{
    if (carousel_items_.empty() == false)
    {
        const int carousel_size = carousel_items_.size();

        if (left_direction)
        {
            RMVCarouselItem* temp = carousel_items_[carousel_size - 1];

            for (int i = carousel_size - 1; i > 0; i--)
            {
                carousel_items_[i] = carousel_items_[i - 1];
            }

            carousel_items_[0] = temp;

            // Decrease carousel index (1-based).
            carousel_index_--;
            if (carousel_index_ < 1)
            {
                carousel_index_ = carousel_size;
            }
        }
        else
        {
            RMVCarouselItem* temp = carousel_items_[0];

            for (int i = 0; i < carousel_size - 1; i++)
            {
                carousel_items_[i] = carousel_items_[i + 1];
            }

            carousel_items_[carousel_size - 1] = temp;

            // Increase carousel index (1-based).
            carousel_index_++;
            if (carousel_index_ > carousel_size)
            {
                carousel_index_ = 1;
            }
        }

        Update();
    }
}

void RMVCarousel::Update()
{
    const QRectF scene_rect = QRectF(0, 0, config_.width, config_.height);

    scene_->setSceneRect(scene_rect);

    int nav_button_height = config_.height / 2;
    int nav_button_width  = ScalingManager::Get().Scaled(kNavButtonWidth);

    left_nav_button_->UpdateDimensions(nav_button_width, nav_button_height);
    right_nav_button_->UpdateDimensions(nav_button_width, nav_button_height);

    int y_nav_pos = nav_button_height / 2;

    left_nav_button_->setPos(0, y_nav_pos);
    int right_nav_x_pos = config_.width - nav_button_width;
    right_nav_button_->setPos(right_nav_x_pos, y_nav_pos);

    int widget_start_pos = nav_button_width;
    int widget_end_pos   = config_.width - nav_button_width;
    int available_pixels = widget_end_pos - widget_start_pos;

    int potential_pixels = 0;
    int consumed_pixels  = 0;
    int widget_fit_count = 0;

    for (int i = 0; i < carousel_items_.size(); i++)
    {
        potential_pixels += carousel_items_[i]->boundingRect().width();

        if (potential_pixels < available_pixels - (ScalingManager::Get().Scaled(5)))
        {
            widget_fit_count++;
            consumed_pixels += carousel_items_[i]->boundingRect().width();

            carousel_items_[i]->show();
        }
        else
        {
            carousel_items_[i]->hide();
        }
    }

    int free_pixels            = available_pixels - consumed_pixels;
    int free_space_count       = widget_fit_count + 1;
    int free_space_pixel_width = free_pixels / free_space_count;

    int x_pos = widget_start_pos + free_space_pixel_width;
    for (int i = 0; i < carousel_items_.size(); i++)
    {
        carousel_items_[i]->setPos(x_pos, 0);
        x_pos += (carousel_items_[i]->boundingRect().width() + free_space_pixel_width);
    }

    // Update the carousel info.
    QString info_string = QString::number(carousel_index_) + QString("/") + QString::number(carousel_items_.size());
    info_text_->setPlainText(info_string);
}

QGraphicsScene* RMVCarousel::Scene() const
{
    return scene_;
}

void RMVCarousel::ResizeEvent(int width, int height)
{
    config_.width  = width;
    config_.height = height;

    Update();
}

void RMVCarousel::SetData(const rmv::RMVCarouselData& carousel_data)
{
    for (auto it : carousel_items_)
    {
        it->SetData(carousel_data);
    }

    update();
}

void RMVCarousel::ClearData()
{
    rmv::RMVCarouselData empty_data = {};
    SetData(empty_data);
}

void RMVCarousel::UpdateModel()
{
    rmv::RMVCarouselData carousel_data = {};
    model_->GetCarouselData(carousel_data);
    SetData(carousel_data);
}

void RMVCarousel::UpdateModel(RmtDataSnapshot* base_snapshot, RmtDataSnapshot* diff_snapshot)
{
    rmv::RMVCarouselData carousel_delta_data = {};
    model_->CalcGlobalCarouselData(base_snapshot, diff_snapshot, carousel_delta_data);
    SetData(carousel_delta_data);
}
