//=============================================================================
/// Copyright (c) 2018-2020 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Implementation for a base pane class
//=============================================================================

#ifndef RMV_VIEWS_BASE_PANE_H_
#define RMV_VIEWS_BASE_PANE_H_

#include <QWidget>

typedef struct RmtDataSnapshot RmtDataSnapshot;

/// Base class for a pane in the UI.
class BasePane : public QWidget
{
    Q_OBJECT

public:
    /// Constructor.
    /// \param parent The parent widget.
    explicit BasePane(QWidget* parent = nullptr);

    /// Destructor.
    virtual ~BasePane();

    /// Open a snapshot.
    /// \param snapshot The snapshot to open.
    virtual void OpenSnapshot(RmtDataSnapshot* snapshot);

    /// Switch panes.
    virtual void PaneSwitched();

    /// Switch the time units.
    virtual void SwitchTimeUnits();

    /// Trace closed.
    virtual void OnTraceClose();

    /// Reset the UI.
    virtual void Reset();

    /// Change the UI coloring based on the settings.
    virtual void ChangeColoring();
};

#endif  // RMV_VIEWS_BASE_PANE_H_
