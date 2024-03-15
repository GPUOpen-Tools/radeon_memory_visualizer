//=============================================================================
// Copyright (c) 2017-2024 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Header for the timeline keyboard zoom shortcuts.
//=============================================================================

#ifndef RMV_VIEWS_TIMELINE_KEYBOARD_ZOOM_SHORTCUTS_TIMELINE_H_
#define RMV_VIEWS_TIMELINE_KEYBOARD_ZOOM_SHORTCUTS_TIMELINE_H_

#include <QApplication>
#include <QScrollBar>
#include <QGraphicsView>

#include "views/keyboard_zoom_shortcuts.h"

class TimelinePane;

namespace rmv
{
    /// @brief Class to handle keyboard zoom shortcut keys for the Timeline pane.
    class KeyboardZoomShortcutsTimeline : public KeyboardZoomShortcuts
    {
        Q_OBJECT

    public:
        /// @brief Constructor.
        ///
        /// @param [in] parent_pane The parent pane.
        /// @param [in] scroll_bar  The view where the zoom scrollbar is located.
        /// @param [in] zoom_view   The view where the zoom shortcuts are applied.
        explicit KeyboardZoomShortcutsTimeline(TimelinePane* parent_pane, QScrollBar* scroll_bar, QGraphicsView* zoom_view = nullptr);

        /// @brief Destructor.
        virtual ~KeyboardZoomShortcutsTimeline();

        /// @brief Handle a key press.
        ///
        /// @param [in] key_code       The key code of the key pressed.
        /// @param [in] is_auto_repeat Is the key autorepeat on.
        ///
        /// @return true if the key press has been processed, false otherwise.
        virtual bool KeyPressed(int key_code, bool is_auto_repeat);

        /// @brief Handle a key release.
        ///
        /// @param [in] key_code       The key code of the key released.
        /// @param [in] is_auto_repeat Is the key autorepeat on.
        ///
        /// @return true if the key press has been processed, false otherwise.
        virtual bool KeyReleased(int key_code, bool is_auto_repeat);

    signals:
        /// @brief Signal for zoom in selection.
        void ZoomInSelectionSignal();

        /// @brief Signal for zoom reset.
        void ResetViewSignal();

    private slots:
        /// @brief Action slot to zoom in.
        ///
        /// @param [in] checked Boolean to indicate if the item checked.
        virtual void OnZoomInShortCut(bool checked) Q_DECL_OVERRIDE;

        /// @brief Action slot to zoom out.
        ///
        /// @param [in] checked Boolean to indicate if the item checked.
        virtual void OnZoomOutShortCut(bool checked) Q_DECL_OVERRIDE;

        /// @brief Action slot to zoom in faster.
        ///
        /// @param [in] checked Boolean to indicate if the item checked.
        virtual void OnZoomInMoreShortCut(bool checked) Q_DECL_OVERRIDE;

        /// @brief Action slot to zoom out faster.
        ///
        /// @param [in] checked Boolean to indicate if the item checked.
        virtual void OnZoomOutMoreShortCut(bool checked) Q_DECL_OVERRIDE;

        /// @brief Action slot to zoom in selection.
        ///
        /// @param [in] checked Boolean to indicate if the item checked.
        virtual void OnZoomInSelection(bool checked) Q_DECL_OVERRIDE;

        /// @brief Action slot to reset the view.
        ///
        /// @param [in] checked Boolean to indicate if the item checked.
        virtual void OnResetView(bool checked) Q_DECL_OVERRIDE;

    private:
        /// @brief Setup keyboard shortcuts for zooming, etc.
        void SetupKeyboardZoomShortcuts();

        TimelinePane*  parent_pane_;  ///< The parent UI Pane.
        QScrollBar*    scroll_bar_;   ///< The scrollbar used for zooming.
        QGraphicsView* zoom_view_;    ///< The graphics view to zoom.
    };
}  // namespace rmv

#endif  // RMV_VIEWS_TIMELINE_KEYBOARD_ZOOM_SHORTCUTS_TIMELINE_H_
