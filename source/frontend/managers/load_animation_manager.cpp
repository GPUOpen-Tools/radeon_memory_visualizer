//==============================================================================
// Copyright (c) 2020-2024 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Implementation of the file loading animation manager.
//==============================================================================

#include "managers/load_animation_manager.h"

#include "managers/message_manager.h"

#include <QApplication>

#include "qt_common/utils/scaling_manager.h"

namespace rmv
{
    /// The single instance of the file loading animation manager.
    static LoadAnimationManager load_animation_manager;

    LoadAnimationManager& LoadAnimationManager::Get()
    {
        return load_animation_manager;
    }

    LoadAnimationManager::LoadAnimationManager(QObject* parent)
        : QObject(parent)
        , tab_widget_(nullptr)
        , file_load_animation_(nullptr)
    {
    }

    LoadAnimationManager::~LoadAnimationManager()
    {
    }

    void LoadAnimationManager::Initialize(TabWidget* tab_widget)
    {
        tab_widget_ = tab_widget;
    }

    void LoadAnimationManager::ResizeAnimation()
    {
        // Update Animation widget on window resize.
        if (file_load_animation_ != nullptr && tab_widget_ != nullptr)
        {
            double   height_offset = tab_widget_->TabHeight();
            QWidget* parent        = file_load_animation_->parentWidget();

            Resize(parent, height_offset);
        }
    }

    void LoadAnimationManager::StartAnimation(QWidget* parent, int height_offset, const bool can_cancel)
    {
        if (file_load_animation_ == nullptr)
        {
            file_load_animation_ = new RmvCancellableLoadingWidget(parent, can_cancel);
            Resize(parent, height_offset);

            file_load_animation_->show();
            tab_widget_->setDisabled(true);
            emit rmv::MessageManager::Get().ChangeActionsRequested(false);

            qApp->setOverrideCursor(Qt::BusyCursor);

            if (can_cancel)
            {
                connect(file_load_animation_, &RmvCancellableLoadingWidget::CancelClicked, this, &LoadAnimationManager::AnimationCancelled);
            }
        }
    }

    void LoadAnimationManager::StartAnimation()
    {
        StartAnimation(tab_widget_, tab_widget_->TabHeight(), false);
    }

    void LoadAnimationManager::StopAnimation()
    {
        if (file_load_animation_ != nullptr)
        {
            delete file_load_animation_;
            file_load_animation_ = nullptr;

            tab_widget_->setEnabled(true);
            emit rmv::MessageManager::Get().ChangeActionsRequested(true);

            qApp->restoreOverrideCursor();
        }
    }

    void LoadAnimationManager::Resize(QWidget* parent, int height_offset)
    {
        if (parent != nullptr)
        {
            // Set overall size of the widget to cover the tab contents.
            int width  = parent->width();
            int height = parent->height() - height_offset;
            file_load_animation_->setGeometry(parent->x(), parent->y() + height_offset, width, height);

            // Set the contents margin so that the animated bars only cover a small area in the middle of the screen.
            const int desired_loading_dimension = ScalingManager::Get().Scaled(200);
            int       vertical_margin           = (height - desired_loading_dimension) / 2;
            int       horizontal_margin         = (width - desired_loading_dimension) / 2;
            file_load_animation_->setContentsMargins(horizontal_margin, vertical_margin, horizontal_margin, vertical_margin);
        }
    }

}  // namespace rmv
