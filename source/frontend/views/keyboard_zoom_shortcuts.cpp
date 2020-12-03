//=============================================================================
/// Copyright (c) 2018-2020 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief Implementation for the global keyboard shortcuts class.
//=============================================================================

#include "views/keyboard_zoom_shortcuts.h"

bool KeyboardZoomShortcuts::enable_shortcuts_ = false;

KeyboardZoomShortcuts::KeyboardZoomShortcuts(QScrollBar* scroll_bar, QGraphicsView* zoom_view)
    : scroll_bar_(scroll_bar)
    , zoom_view_(zoom_view)
{
    SetupNavigationControl();
}

KeyboardZoomShortcuts::~KeyboardZoomShortcuts()
{
}

void KeyboardZoomShortcuts::OnScrollViewSingleStepSub(bool checked)
{
    Q_UNUSED(checked);

    scroll_bar_->triggerAction(QAbstractSlider::SliderSingleStepSub);
}

void KeyboardZoomShortcuts::OnScrollViewPageStepSub(bool checked)
{
    Q_UNUSED(checked);

    scroll_bar_->triggerAction(QAbstractSlider::SliderPageStepSub);
}

void KeyboardZoomShortcuts::OnScrollViewSingleStepAdd(bool checked)
{
    Q_UNUSED(checked);

    scroll_bar_->triggerAction(QAbstractSlider::SliderSingleStepAdd);
}

void KeyboardZoomShortcuts::OnScrollViewPageStepAdd(bool checked)
{
    Q_UNUSED(checked);

    scroll_bar_->triggerAction(QAbstractSlider::SliderPageStepAdd);
}

void KeyboardZoomShortcuts::EnableShortcuts(bool enable)
{
    enable_shortcuts_ = enable;
}

bool KeyboardZoomShortcuts::IsShortcutsEnabled()
{
    return enable_shortcuts_;
}

void KeyboardZoomShortcuts::SetupNavigationControl()
{
    navigation_control_[Qt::CTRL | Qt::Key_Left]  = &KeyboardZoomShortcuts::OnScrollViewPageStepSub;
    navigation_control_[Qt::Key_Left]             = &KeyboardZoomShortcuts::OnScrollViewSingleStepSub;
    navigation_control_[Qt::CTRL | Qt::Key_Right] = &KeyboardZoomShortcuts::OnScrollViewPageStepAdd;
    navigation_control_[Qt::Key_Right]            = &KeyboardZoomShortcuts::OnScrollViewSingleStepAdd;
    navigation_control_[Qt::Key_A]                = &KeyboardZoomShortcuts::OnZoomInShortCut;
    navigation_control_[Qt::Key_Z]                = &KeyboardZoomShortcuts::OnZoomOutShortCut;
    navigation_control_[Qt::CTRL | Qt::Key_Z]     = &KeyboardZoomShortcuts::OnZoomInSelection;
    navigation_control_[Qt::Key_H]                = &KeyboardZoomShortcuts::OnResetView;
    navigation_control_[Qt::Key_S]                = &KeyboardZoomShortcuts::OnZoomInMoreShortCut;
    navigation_control_[Qt::Key_X]                = &KeyboardZoomShortcuts::OnZoomOutMoreShortCut;
}