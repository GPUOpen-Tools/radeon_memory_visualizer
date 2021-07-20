//=============================================================================
// Copyright (c) 2020-2021 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Header for the Snapshot start pane.
//=============================================================================

#ifndef RMV_VIEWS_SNAPSHOT_SNAPSHOT_START_PANE_H_
#define RMV_VIEWS_SNAPSHOT_SNAPSHOT_START_PANE_H_

#include "ui_snapshot_start_pane.h"

#include <QGraphicsScene>

#include "views/base_pane.h"
#include "views/custom_widgets/rmv_camera_snapshot_widget.h"

/// @brief Class declaration.
class SnapshotStartPane : public BasePane
{
    Q_OBJECT

public:
    /// @brief constructor.
    ///
    /// @param [in] parent The parent widget.
    explicit SnapshotStartPane(QWidget* parent = nullptr);

    /// @brief Destructor.
    virtual ~SnapshotStartPane();

    /// @brief Overridden window resize event.
    ///
    /// @param [in] event the resize event object.
    virtual void resizeEvent(QResizeEvent* event) Q_DECL_OVERRIDE;

    /// @brief Reset UI state.
    virtual void Reset() Q_DECL_OVERRIDE;

    /// @brief Update UI coloring.
    virtual void ChangeColoring() Q_DECL_OVERRIDE;

    /// @brief Open a snapshot.
    ///
    /// @param [in] snapshot Pointer to the snapshot to open.
    virtual void OpenSnapshot(RmtDataSnapshot* snapshot) Q_DECL_OVERRIDE;

    /// @brief Set the text for the pane describing an empty snapshot.
    void SetEmptyTitleText();

private slots:
    /// @brief Callback for when the DPI Scale changes.
    void OnScaleFactorChanged();

private:
    /// @brief Resizes the GraphicsView to fit the scene.
    void ResizeGraphicsView();

    Ui::SnapshotStartPane*   ui_;               ///< Pointer to the Qt UI design.
    QGraphicsScene*          scene_;            ///< Qt scene for the camera drawing.
    RMVCameraSnapshotWidget* snapshot_widget_;  ///< Circle with camera.
};

#endif  // RMV_VIEWS_SNAPSHOT_SNAPSHOT_START_PANE_H_
