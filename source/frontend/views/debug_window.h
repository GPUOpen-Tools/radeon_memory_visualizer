//=============================================================================
/// Copyright (c) 2018-2020 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Header for RMV debug window.
//=============================================================================

#ifndef RMV_VIEWS_DEBUG_WINDOW_H_
#define RMV_VIEWS_DEBUG_WINDOW_H_

#include "ui_debug_window.h"

#include <QDialog>

/// Support for RMV debug window.
class DebugWindow : public QDialog
{
    Q_OBJECT

public:
    /// Constructor.
    explicit DebugWindow();

    /// Destructor.
    ~DebugWindow();

    /// Send a message to the debug window. Supports multiple arguments.
    /// \param format The string containing format for each argument.
    static void DbgMsg(const char* format, ...);

signals:
    /// Signal that gets emitted when the debug window has new text to add.
    /// This will be picked up by the slot below.
    /// \param string The new line of text to add.
    void EmitSetText(const QString& string);

private slots:
    /// Add a new line of text to the debug window.
    /// \param string The new line of text to add.
    void SetText(const QString& string);

private:
    /// Helper function which to automatically scroll to the bottom on new line.
    void ScrollToBottom();

    /// Register the Debug Window such that it is accessible.
    /// This is only to be called once, when initializing MainWindow.
    void RegisterDbgWindow();

    Ui::DebugWindow* ui_;  ///< Pointer to the Qt UI design.
};

#endif  // RMV_VIEWS_DEBUG_WINDOW_H_
