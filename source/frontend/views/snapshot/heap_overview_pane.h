//=============================================================================
/// Copyright (c) 2018-2020 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Header for the Heap Overview pane.
//=============================================================================

#ifndef RMV_VIEWS_SNAPSHOT_HEAP_OVERVIEW_PANE_H_
#define RMV_VIEWS_SNAPSHOT_HEAP_OVERVIEW_PANE_H_

#include "ui_heap_overview_pane.h"

#include "views/base_pane.h"

/// Class declaration
class HeapOverviewPane : public BasePane
{
    Q_OBJECT

public:
    /// Constructor.
    /// \param parent The parent widget.
    explicit HeapOverviewPane(QWidget* parent = nullptr);

    /// Destructor.
    virtual ~HeapOverviewPane();

    /// Open a snapshot.
    /// \param snapshot The snapshot to open.
    virtual void OpenSnapshot(RmtDataSnapshot* snapshot) Q_DECL_OVERRIDE;

protected:
    /// Overridden show event. Fired when this pane is opened.
    /// \param event the show event object.
    virtual void showEvent(QShowEvent* event) Q_DECL_OVERRIDE;

    /// Overridden window resize event.
    /// \param event the resize event object.
    virtual void resizeEvent(QResizeEvent* event) Q_DECL_OVERRIDE;

private:
    /// Refresh what's visible on the UI.
    void Refresh();

    /// Resize all relevant UI items.
    void ResizeItems();

    Ui::HeapOverviewPane* ui_;  ///< Pointer to the Qt UI design.
};

#endif  // RMV_VIEWS_SNAPSHOT_HEAP_OVERVIEW_PANE_H_
