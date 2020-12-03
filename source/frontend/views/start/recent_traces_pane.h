//=============================================================================
/// Copyright (c) 2018-2020 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Header for the Recent traces pane.
//=============================================================================

#ifndef RMV_VIEWS_START_RECENT_TRACES_PANE_H_
#define RMV_VIEWS_START_RECENT_TRACES_PANE_H_

#include "ui_recent_traces_pane.h"

#include <QVector>
#include <QWidget>
#include <QLabel>

#include "qt_common/custom_widgets/recent_trace_widget.h"

#include "views/base_pane.h"

class MainWindow;

/// Class declaration.
class RecentTracesPane : public BasePane
{
    Q_OBJECT

public:
    /// Constructor.
    /// \param parent The widget's parent.
    explicit RecentTracesPane(MainWindow* parent = nullptr);

    /// Destructor.
    virtual ~RecentTracesPane();

signals:
    /// Something changed the file list (either a delete or a new file added).
    void FileListChanged();

public slots:
    /// Set up the list of recent traces in the UI.
    void SetupFileList();

    /// Slot to delete a trace from the Recent traces list. Only removes it from
    /// the list; doesn't actually delete the file.
    /// \param path The path to the trace file.
    void DeleteTrace(QString path);

private:
    Ui::RecentTracesPane* ui_;  ///< Pointer to the Qt UI design.

    MainWindow*                 main_window_;                  ///< Reference to the mainwindow (parent).
    QVector<RecentTraceWidget*> trace_widgets_;                ///< Array of trace widgets.
    QVBoxLayout*                vbox_layout_;                  ///< The vertical layout to handle custom widgets.
    QWidget*                    scroll_area_widget_contents_;  ///< The scroll area widget contents widget.
    QLabel*                     no_traces_label_;              ///< The no traces label.
};

#endif  // RMV_VIEWS_START_RECENT_TRACES_PANE_H_
