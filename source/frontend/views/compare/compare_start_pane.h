//=============================================================================
// Copyright (c) 2018-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Header for the Compare start pane.
//=============================================================================

#ifndef RMV_VIEWS_COMPARE_COMPARE_START_PANE_H_
#define RMV_VIEWS_COMPARE_COMPARE_START_PANE_H_

#include "ui_compare_start_pane.h"

#include <QGraphicsScene>

#include "views/base_pane.h"
#include "views/custom_widgets/rmv_camera_snapshot_widget.h"

/// @brief Class declaration.
class CompareStartPane : public BasePane
{
    Q_OBJECT

public:
    /// @brief Constructor.
    ///
    /// @param [in] parent The parent widget.
    explicit CompareStartPane(QWidget* parent = nullptr);

    /// @brief Destructor.
    virtual ~CompareStartPane();

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
    /// @param [in] snapshot The snapshot to open.
    virtual void OpenSnapshot(RmtDataSnapshot* snapshot) Q_DECL_OVERRIDE;

    /// @brief Set the text for the pane describing an empty comparison.
    void SetEmptyTitleText();

private:
    /// @brief Repositions the right circle and resizes the GraphicsView to fit the whole scene.
    void UpdateCirclePositions();

    Ui::CompareStartPane* ui_;  ///< Pointer to the Qt UI design.

    QGraphicsScene*          scene_;                  ///< Qt scene for the camera drawing.
    RMVCameraSnapshotWidget* snapshot_widget_left_;   ///< Left circle with camera.
    RMVCameraSnapshotWidget* snapshot_widget_right_;  ///< Right circle with camera.
};

#endif  // RMV_VIEWS_COMPARE_COMPARE_START_PANE_H_
