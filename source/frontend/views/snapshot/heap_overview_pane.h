//=============================================================================
// Copyright (c) 2018-2023 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Header for the Heap Overview pane.
//=============================================================================

#ifndef RMV_VIEWS_SNAPSHOT_HEAP_OVERVIEW_PANE_H_
#define RMV_VIEWS_SNAPSHOT_HEAP_OVERVIEW_PANE_H_

#include "ui_heap_overview_pane.h"

#include "views/base_pane.h"

/// @brief Class declaration.
class HeapOverviewPane : public BasePane
{
    Q_OBJECT

public:
    /// @brief Constructor.
    ///
    /// @param [in] parent The parent widget.
    explicit HeapOverviewPane(QWidget* parent = nullptr);

    /// @brief Destructor.
    virtual ~HeapOverviewPane();

    /// @brief Open a snapshot.
    ///
    /// @param [in] snapshot The snapshot to open.
    virtual void OpenSnapshot(RmtDataSnapshot* snapshot) Q_DECL_OVERRIDE;

protected:
    /// @brief Overridden show event. Fired when this pane is opened.
    ///
    /// @param [in] event the show event object.
    virtual void showEvent(QShowEvent* event) Q_DECL_OVERRIDE;

    /// @brief Overridden window resize event.
    ///
    /// @param [in] event the resize event object.
    virtual void resizeEvent(QResizeEvent* event) Q_DECL_OVERRIDE;

private:
    /// @brief Refresh what's visible on the UI.
    void Refresh();

    /// @brief Resize all relevant UI items.
    void ResizeItems();

    Ui::HeapOverviewPane* ui_;  ///< Pointer to the Qt UI design.
};

#endif  // RMV_VIEWS_SNAPSHOT_HEAP_OVERVIEW_PANE_H_
