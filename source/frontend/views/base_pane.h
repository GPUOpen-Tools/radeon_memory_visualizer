//=============================================================================
// Copyright (c) 2018-2021 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Header for the base pane class.
//=============================================================================

#ifndef RMV_VIEWS_BASE_PANE_H_
#define RMV_VIEWS_BASE_PANE_H_

#include <QWidget>

typedef struct RmtDataSnapshot RmtDataSnapshot;

/// @brief Base class for a pane in the UI.
class BasePane : public QWidget
{
    Q_OBJECT

public:
    /// @brief Constructor.
    ///
    /// @param [in] parent The parent widget.
    explicit BasePane(QWidget* parent = nullptr);

    /// @brief Destructor.
    virtual ~BasePane();

    /// Open a snapshot.
    ///
    /// @param [in] snapshot The snapshot to open.
    virtual void OpenSnapshot(RmtDataSnapshot* snapshot);

    /// @brief Switch the time units.
    virtual void SwitchTimeUnits();

    /// @brief Trace closed.
    virtual void OnTraceClose();

    /// @brief Reset the UI.
    virtual void Reset();

    /// @brief Change the UI coloring based on the settings.
    virtual void ChangeColoring();
};

#endif  // RMV_VIEWS_BASE_PANE_H_
