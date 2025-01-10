//=============================================================================
// Copyright (c) 2017-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Implementation for the timeline keyboard shortcuts class.
//=============================================================================

#include "views/timeline/keyboard_zoom_shortcuts_timeline.h"

#include <QMap>
#include <QAction>
#include <QDebug>

#include "views/timeline/timeline_pane.h"

namespace rmv
{
    KeyboardZoomShortcutsTimeline::KeyboardZoomShortcutsTimeline(TimelinePane* parent_pane, QScrollBar* scroll_bar, QGraphicsView* zoom_view)
        : KeyboardZoomShortcuts(scroll_bar, zoom_view)
        , parent_pane_(parent_pane)
        , scroll_bar_(scroll_bar)
        , zoom_view_(zoom_view)
    {
        SetupKeyboardZoomShortcuts();
    }

    KeyboardZoomShortcutsTimeline::~KeyboardZoomShortcutsTimeline()
    {
    }

    void KeyboardZoomShortcutsTimeline::OnZoomInShortCut(bool checked)
    {
        Q_UNUSED(checked);

        parent_pane_->ZoomInCustom(2, true);
    }

    void KeyboardZoomShortcutsTimeline::OnZoomOutShortCut(bool checked)
    {
        Q_UNUSED(checked);

        parent_pane_->ZoomOutCustom(2, true);
    }

    void KeyboardZoomShortcutsTimeline::OnZoomInMoreShortCut(bool checked)
    {
        Q_UNUSED(checked);

        parent_pane_->ZoomInCustom(10, true);
    }

    void KeyboardZoomShortcutsTimeline::OnZoomOutMoreShortCut(bool checked)
    {
        Q_UNUSED(checked);

        parent_pane_->ZoomOutCustom(10, true);
    }

    void KeyboardZoomShortcutsTimeline::OnZoomInSelection(bool checked)
    {
        Q_UNUSED(checked);

        emit ZoomInSelectionSignal();
    }

    void KeyboardZoomShortcutsTimeline::OnResetView(bool checked)
    {
        Q_UNUSED(checked);

        emit ResetViewSignal();
    }

    void KeyboardZoomShortcutsTimeline::SetupKeyboardZoomShortcuts()
    {
        // Setup actions for various keyboard shortcuts.
        for (auto const& item : navigation_control_)
        {
            // NOTE: Actions here pass in the parent widget when constructed. It is the responsibility of the parent widget
            // to make sure they are deleted.
            QAction* action = new QAction(parent_pane_);
            action->setShortcut(item.first);

            connect(action, &QAction::triggered, this, item.second);
            parent_pane_->addAction(action);
        }
    }

    bool KeyboardZoomShortcutsTimeline::KeyPressed(int key_code, bool is_auto_repeat)
    {
        bool processed = true;
        switch (key_code)
        {
        case Qt::Key_Space:
        {
            // Only enable drag if not currently holding a mouse button down and the mouse
            // is over the graphics view to be dragged.
            Qt::MouseButtons buttons = QApplication::mouseButtons();
            if (is_auto_repeat == false && buttons == Qt::NoButton)
            {
                if ((zoom_view_ && zoom_view_->underMouse()))
                {
                    // Set graphics view to scroll mode.
                    zoom_view_->setDragMode(QGraphicsView::ScrollHandDrag);
                }
                else if (zoom_view_ == nullptr)
                {
                    // Set cursor for event timings treeview.
                    parent_pane_->setCursor(QCursor(Qt::OpenHandCursor));
                }
            }
        }
        break;

        default:
            processed = false;
            break;
        }
        return processed;
    }

    bool KeyboardZoomShortcutsTimeline::KeyReleased(int key_code, bool is_auto_repeat)
    {
        bool processed = true;
        switch (key_code)
        {
        case Qt::Key_Space:
            if (!is_auto_repeat)
            {
                if (zoom_view_ != nullptr)
                {
                    zoom_view_->setDragMode(QGraphicsView::NoDrag);
                }
                else
                {
                    parent_pane_->setCursor(Qt::ArrowCursor);
                }
            }
            break;

        default:
            processed = false;
            break;
        }
        return processed;
    }
}  // namespace rmv
