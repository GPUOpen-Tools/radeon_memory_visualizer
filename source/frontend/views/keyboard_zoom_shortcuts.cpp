//=============================================================================
// Copyright (c) 2018-2024 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Implementation for the global keyboard shortcuts class.
//=============================================================================

#include "views/keyboard_zoom_shortcuts.h"

namespace rmv
{
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
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
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
#else
        navigation_control_[QKeyCombination(Qt::CTRL | Qt::Key_Left).toCombined()]  = &KeyboardZoomShortcuts::OnScrollViewPageStepSub;
        navigation_control_[QKeyCombination(Qt::Key_Left).toCombined()]             = &KeyboardZoomShortcuts::OnScrollViewSingleStepSub;
        navigation_control_[QKeyCombination(Qt::CTRL | Qt::Key_Right).toCombined()] = &KeyboardZoomShortcuts::OnScrollViewPageStepAdd;
        navigation_control_[QKeyCombination(Qt::Key_Right).toCombined()]            = &KeyboardZoomShortcuts::OnScrollViewSingleStepAdd;
        navigation_control_[QKeyCombination(Qt::Key_A).toCombined()]                = &KeyboardZoomShortcuts::OnZoomInShortCut;
        navigation_control_[QKeyCombination(Qt::Key_Z).toCombined()]                = &KeyboardZoomShortcuts::OnZoomOutShortCut;
        navigation_control_[QKeyCombination(Qt::CTRL | Qt::Key_Z).toCombined()]     = &KeyboardZoomShortcuts::OnZoomInSelection;
        navigation_control_[QKeyCombination(Qt::Key_H).toCombined()]                = &KeyboardZoomShortcuts::OnResetView;
        navigation_control_[QKeyCombination(Qt::Key_S).toCombined()]                = &KeyboardZoomShortcuts::OnZoomInMoreShortCut;
        navigation_control_[QKeyCombination(Qt::Key_X).toCombined()]                = &KeyboardZoomShortcuts::OnZoomOutMoreShortCut;
#endif
    }
}  // namespace rmv