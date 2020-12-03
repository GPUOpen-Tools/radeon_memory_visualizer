//=============================================================================
/// Copyright (c) 2017-2020 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief Header for the keyboard zoom shortcuts.
//=============================================================================

#ifndef RMV_VIEWS_TIMELINE_KEYBOARD_ZOOM_SHORTCUTS_TIMELINE_H_
#define RMV_VIEWS_TIMELINE_KEYBOARD_ZOOM_SHORTCUTS_TIMELINE_H_

#include <QApplication>
#include <QScrollBar>
#include <QGraphicsView>

#include "views/keyboard_zoom_shortcuts.h"

class TimelinePane;

/// Class to handle keyboard zoom shortcut keys for the Timeline pane.
class KeyboardZoomShortcutsTimeline : public KeyboardZoomShortcuts
{
    Q_OBJECT

public:
    /// Constructor.
    /// \param parent_pane The parent pane.
    /// \param scroll_bar The view where the zoom scrollbar is located.
    /// \param zoom_view The view where the zoom shortcuts are applied.
    explicit KeyboardZoomShortcutsTimeline(TimelinePane* parent_pane, QScrollBar* scroll_bar, QGraphicsView* zoom_view = nullptr);

    /// Destructor.
    virtual ~KeyboardZoomShortcutsTimeline();

    /// Handle a key press.
    /// \param key_code The key code of the key pressed.
    /// \param is_auto_repeat Is the key autorepeat on.
    /// \return true if the key press has been processed, false otherwise.
    virtual bool KeyPressed(int key_code, bool is_auto_repeat);

    /// Handle a key release.
    /// \param key_code The key code of the key released.
    /// \param is_auto_repeat Is the key autorepeat on.
    /// \return true if the key press has been processed, false otherwise.
    virtual bool KeyReleased(int key_code, bool is_auto_repeat);

signals:
    /// Signal for zoom in selection.
    void ZoomInSelectionSignal();

    /// Signal for zoom reset.
    void ResetViewSignal();

private slots:
    /// Action slot to zoom in.
    /// \param checked Boolean to indicate if the item checked.
    virtual void OnZoomInShortCut(bool checked) Q_DECL_OVERRIDE;

    /// Action slot to zoom out.
    /// \param checked Boolean to indicate if the item checked.
    virtual void OnZoomOutShortCut(bool checked) Q_DECL_OVERRIDE;

    /// Action slot to zoom in faster.
    /// \param checked Boolean to indicate if the item checked.
    virtual void OnZoomInMoreShortCut(bool checked) Q_DECL_OVERRIDE;

    /// Action slot to zoom out faster.
    /// \param checked Boolean to indicate if the item checked.
    virtual void OnZoomOutMoreShortCut(bool checked) Q_DECL_OVERRIDE;

    /// Action slot to zoom in selection.
    /// \param checked Boolean to indicate if the item checked.
    virtual void OnZoomInSelection(bool checked) Q_DECL_OVERRIDE;

    /// Action slot to reset the view.
    /// \param checked Boolean to indicate if the item checked.
    virtual void OnResetView(bool checked) Q_DECL_OVERRIDE;

private:
    /// Setup keyboard shortcuts for zooming, etc.
    void SetupKeyboardZoomShortcuts();

    TimelinePane*  parent_pane_;  ///< The parent UI Pane.
    QScrollBar*    scroll_bar_;   ///< The scrollbar used for zooming.
    QGraphicsView* zoom_view_;    ///< The graphics view to zoom.
};

#endif  // RMV_VIEWS_TIMELINE_KEYBOARD_ZOOM_SHORTCUTS_TIMELINE_H_
