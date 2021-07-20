//=============================================================================
// Copyright (c) 2018-2021 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Header for the Snapshot Delta pane.
//=============================================================================

#ifndef RMV_VIEWS_COMPARE_SNAPSHOT_DELTA_PANE_H_
#define RMV_VIEWS_COMPARE_SNAPSHOT_DELTA_PANE_H_

#include "ui_snapshot_delta_pane.h"

#include "qt_common/custom_widgets/colored_legend_scene.h"

#include "models/compare/snapshot_delta_model.h"
#include "views/base_pane.h"
#include "views/compare_pane.h"
#include "views/custom_widgets/rmv_carousel.h"
#include "views/custom_widgets/rmv_delta_display.h"
#include "util/definitions.h"

/// @brief Enum containing indices for the snapshot delta information.
enum SnapshotDeltaDataType
{
    kSnapshotDeltaTypeAvailableSize,
    kSnapshotDeltaTypeAllocatedAndBound,
    kSnapshotDeltaTypeAllocatedAndUnbound,
    kSnapshotDeltaTypeAllocationCount,
    kSnapshotDeltaTypeResourceCount,

    kSnapshotDeltaTypeCount,
};

/// @brief Pairs a delta display row with a simple line.
struct DeltaDisplayLinePair
{
    RMVDeltaDisplay* display;  ///< The delta items.
    QFrame*          line;     ///< The separator line.
};

/// @brief Class declaration.
class SnapshotDeltaPane : public ComparePane
{
    Q_OBJECT

public:
    /// @brief Constructor.
    ///
    /// @param [in] parent The parent widget.
    explicit SnapshotDeltaPane(QWidget* parent = nullptr);

    /// @brief Destructor.
    virtual ~SnapshotDeltaPane();

    /// @brief Overridden show event. Fired when this pane is opened.
    ///
    /// @param [in] event the show event object.
    virtual void showEvent(QShowEvent* event) Q_DECL_OVERRIDE;

    /// @brief Overridden window resize event.
    ///
    /// @param [in] event The resize event object.
    virtual void resizeEvent(QResizeEvent* event) Q_DECL_OVERRIDE;

    /// @brief Reset UI state.
    virtual void Reset() Q_DECL_OVERRIDE;

    /// @brief Update UI coloring.
    virtual void ChangeColoring() Q_DECL_OVERRIDE;

    /// @brief Update the UI when a new comparison is done.
    virtual void Refresh() Q_DECL_OVERRIDE;

private slots:
    /// @brief Switch snapshots.
    void SwitchSnapshots();

    /// @brief Resize widgets as needed according to the DPI Scale.
    void OnScaleFactorChanged();

private:
    /// @brief Update the UI.
    void UpdateUI();

    /// @brief Resize relevant items.
    void ResizeItems();

    /// @brief Add the memory delta legends to the required scene.
    void AddMemoryDeltaLegends();

    Ui::SnapshotDeltaPane* ui_;  ///< Pointer to the Qt UI design.

    rmv::SnapshotDeltaModel* model_;                                ///< Container class for the widget models.
    RMVCarousel*             carousel_;                             ///< Pointer to the carousel object.
    ColoredLegendScene*      legends_;                              ///< Pointer to the legends scene.
    QVector<DeltaItem>       delta_items_;                          ///< Array of delta items.
    DeltaDisplayLinePair     delta_line_pairs_[kRmtHeapTypeCount];  ///< Array of delta pairs.
};

#endif  // RMV_VIEWS_COMPARE_SNAPSHOT_DELTA_PANE_H_
