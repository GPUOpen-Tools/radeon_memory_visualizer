//=============================================================================
/// Copyright (c) 2018-2020 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Header for the Keyboard Shortcuts pane.
//=============================================================================

#ifndef RMV_VIEWS_SETTINGS_KEYBOARD_SHORTCUTS_PANE_H_
#define RMV_VIEWS_SETTINGS_KEYBOARD_SHORTCUTS_PANE_H_

#include "ui_keyboard_shortcuts_pane.h"

#include "views/base_pane.h"

/// Class declaration.
class KeyboardShortcutsPane : public BasePane
{
    Q_OBJECT

public:
    /// Constructor.
    /// \param parent The widget's parent.
    explicit KeyboardShortcutsPane(QWidget* parent = nullptr);

    /// Destructor.
    virtual ~KeyboardShortcutsPane();

private:
    Ui::KeyboardShortcutsPane* ui_;  ///< Pointer to the Qt UI design.
};

#endif  // RMV_VIEWS_SETTINGS_KEYBOARD_SHORTCUTS_PANE_H_
