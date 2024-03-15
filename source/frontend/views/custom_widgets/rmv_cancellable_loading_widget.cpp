//=============================================================================
/// Copyright (c) 2023-2024 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Implementation of a loading widget with a cancel button.
//=============================================================================

#include "views/custom_widgets/rmv_cancellable_loading_widget.h"

#include <QApplication>
#include <QMainWindow>

#include "qt_common/utils/scaling_manager.h"

// The space between the bottom of the animation and the top of the cancel button.
static const int kCancelButtonVerticalSpace = 4;

RmvCancellableLoadingWidget::RmvCancellableLoadingWidget(QWidget* parent, const bool can_cancel)
    : FileLoadingWidget(parent)
    , cancel_button_(nullptr)
{
    if (can_cancel)
    {
        // Find the main window and make this the parent of the cancel button.
        // The rest of the UI will be disabled but the cancel button needs to be left enabled
        // so that it can be clicked by the user.
        for (QWidget* widget : QApplication::topLevelWidgets())
        {
            if (widget->inherits("QMainWindow"))
            {
                QMainWindow* main_window = qobject_cast<QMainWindow*>(widget);

                if (main_window != nullptr)
                {
                    cancel_button_ = new ScaledPushButton("Cancel", main_window);
                    cancel_button_->setObjectName("cancel_button");
                    connect(cancel_button_, &ScaledPushButton::clicked, this, &RmvCancellableLoadingWidget::HandleCancelClicked);
                    cancel_button_->show();

                    break;
                }
            }
        }
    }
}

RmvCancellableLoadingWidget::~RmvCancellableLoadingWidget()
{
    delete cancel_button_;
}

void RmvCancellableLoadingWidget::HandleCancelClicked(bool checked)
{
    Q_UNUSED(checked);
    emit CancelClicked();
}

void RmvCancellableLoadingWidget::resizeEvent(QResizeEvent* event)
{
    FileLoadingWidget::resizeEvent(event);

    if (cancel_button_ != nullptr)
    {
        cancel_button_->resize(cancel_button_->sizeHint().width(), cancel_button_->sizeHint().height());

        // The load animation is positioned by creating margins around the widget to squeeze it into the center.
        // Determine the position of the animation so that the cancel button can be positioned centered underneath.
        QMargins margins          = contentsMargins();
        int      animation_width  = geometry().width() - (margins.left() + margins.right());
        int      animation_height = geometry().height() - (margins.top() + margins.bottom());
        int      x_position       = geometry().x() + margins.left() + ((animation_width / 2) - (cancel_button_->geometry().width() / 2));
        int      y_position       = geometry().y() + margins.top() + animation_height + ScalingManager::Get().Scaled(kCancelButtonVerticalSpace);
        QPoint   button_position(x_position, y_position);
        button_position = mapToGlobal(button_position);
        button_position = cancel_button_->parentWidget()->mapFromGlobal(button_position);
        cancel_button_->setGeometry(button_position.x(), button_position.y(), cancel_button_->geometry().width(), cancel_button_->geometry().height());
    }
}
