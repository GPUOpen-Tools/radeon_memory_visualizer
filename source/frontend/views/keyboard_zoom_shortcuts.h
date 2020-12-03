//=============================================================================
/// Copyright (c) 2018-2020 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief Header for the keyboard zoom shortcuts
//=============================================================================

#ifndef RMV_VIEWS_KEYBOARD_ZOOM_SHORTCUTS_H_
#define RMV_VIEWS_KEYBOARD_ZOOM_SHORTCUTS_H_

#include <QApplication>
#include <QScrollBar>
#include <QGraphicsView>
#include <QObject>

/// Class to handle keyboard zoom shortcut keys
class KeyboardZoomShortcuts : public QObject
{
    Q_OBJECT

public:
    /// Constructor.
    /// \param scroll_bar The view where the zoom scrollbar is located.
    /// \param zoom_view The view where the zoom shortcuts are applied.
    explicit KeyboardZoomShortcuts(QScrollBar* scroll_bar, QGraphicsView* zoom_view = nullptr);

    /// Destructor.
    virtual ~KeyboardZoomShortcuts();

    /// Enable keyboard shortcuts.
    /// \param enable Boolean to indicate if shortcuts are enabled.
    static void EnableShortcuts(bool enable);

    /// Are keyboard shortcuts enabled.
    /// \return true if enabled, false if not.
    static bool IsShortcutsEnabled();

public slots:
    /// Action slot to scroll using right arrow key.
    /// \param checked Boolean to indicate if the item checked.
    virtual void OnScrollViewSingleStepAdd(bool checked);

    /// Action slot to scroll using left arrow key.
    /// \param checked Boolean to indicate if the item checked.
    virtual void OnScrollViewSingleStepSub(bool checked);

    /// Action slot to scroll using ctrl + right arrow key.
    /// \param checked Boolean to indicate if the item checked.
    void OnScrollViewPageStepAdd(bool checked);

    /// Action slot to scroll using ctrl + left arrow key.
    /// \param checked Boolean to indicate if the item checked.
    void OnScrollViewPageStepSub(bool checked);

    // Pure virtual methods to be implemented in the derived classes

    /// Action slot to zoom in.
    /// \param checked Boolean to indicate if the item checked.
    virtual void OnZoomInShortCut(bool checked) = 0;

    /// Action slot to zoom out.
    /// \param checked Boolean to indicate if the item checked.
    virtual void OnZoomOutShortCut(bool checked) = 0;

    /// Action slot to zoom in faster.
    /// \param checked Boolean to indicate if the item checked.
    virtual void OnZoomInMoreShortCut(bool checked) = 0;

    /// Action slot to zoom out faster.
    /// \param checked Boolean to indicate if the item checked.
    virtual void OnZoomOutMoreShortCut(bool checked) = 0;

    /// Action slot to zoom in selection.
    /// \param checked Boolean to indicate if the item checked.
    virtual void OnZoomInSelection(bool checked) = 0;

    /// Action slot to reset the view.
    /// \param checked Boolean to indicate if the item checked.
    virtual void OnResetView(bool checked) = 0;

protected:
    typedef void (KeyboardZoomShortcuts::*ShortcutSlot)(bool flag);
    typedef std::map<int, ShortcutSlot> NavigationControl;

    NavigationControl navigation_control_;  ///< The navigation control information for each key.

private:
    /// Setup navigation controls.
    void SetupNavigationControl();

    static bool    enable_shortcuts_;  ///< Boolean indicate if shortcuts enabled for all panes.
    QScrollBar*    scroll_bar_;        ///< The scrollbar used for zooming.
    QGraphicsView* zoom_view_;         ///< The graphics view to zoom.
};

#endif  // RMV_VIEWS_KEYBOARD_ZOOM_SHORTCUTS_H_
