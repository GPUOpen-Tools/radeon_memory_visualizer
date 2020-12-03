//=============================================================================
/// Copyright (c) 2018-2020 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Header for the Compare start pane.
//=============================================================================

#ifndef RMV_VIEWS_COMPARE_COMPARE_START_PANE_H_
#define RMV_VIEWS_COMPARE_COMPARE_START_PANE_H_

#include "ui_compare_start_pane.h"

#include <QGraphicsScene>

#include "views/base_pane.h"
#include "views/custom_widgets/rmv_camera_snapshot_widget.h"

/// Class declaration.
class CompareStartPane : public BasePane
{
    Q_OBJECT

public:
    /// Constructor.
    /// \param parent The widget's parent.
    explicit CompareStartPane(QWidget* parent = nullptr);

    /// Destructor.
    virtual ~CompareStartPane();

    /// Overridden window resize event.
    /// \param event the resize event object.
    virtual void resizeEvent(QResizeEvent* event) Q_DECL_OVERRIDE;

    /// Reset UI state.
    virtual void Reset() Q_DECL_OVERRIDE;

    /// Update UI coloring.
    virtual void ChangeColoring() Q_DECL_OVERRIDE;

    /// Open a snapshot.
    /// \param snapshot snapshot to open.
    virtual void OpenSnapshot(RmtDataSnapshot* snapshot) Q_DECL_OVERRIDE;

private slots:
    /// Callback for when the DPI Scale changes.
    void OnScaleFactorChanged();

private:
    /// Repositions the right circle and then.
    /// resizes the GraphicsView to fit the whole scene.
    void UpdateCirclePositions();

    Ui::CompareStartPane* ui_;  ///< Pointer to the Qt UI design.

    QGraphicsScene*          scene_;                  ///< Qt scene for the camera drawing.
    RMVCameraSnapshotWidget* snapshot_widget_left_;   ///< Left circle with camera.
    RMVCameraSnapshotWidget* snapshot_widget_right_;  ///< Right circle with camera.
};

#endif  // RMV_VIEWS_COMPARE_COMPARE_START_PANE_H_
