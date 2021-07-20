//=============================================================================
// Copyright (c) 2019-2021 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Header for a layout for a single heap in the heap overview pane.
//=============================================================================

#ifndef RMV_VIEWS_SNAPSHOT_HEAP_OVERVIEW_HEAP_LAYOUT_H_
#define RMV_VIEWS_SNAPSHOT_HEAP_OVERVIEW_HEAP_LAYOUT_H_

#include "ui_heap_overview_heap_layout.h"

#include <QWidget>

#include "qt_common/custom_widgets/colored_legend_scene.h"
#include "qt_common/custom_widgets/colored_legend_graphics_view.h"

#include "models/snapshot/heap_overview_heap_model.h"

/// @brief Class declaration.
class HeapOverviewHeapLayout : public QWidget
{
    Q_OBJECT

public:
    /// The number of resource legends set up in the UI.
    static const int kNumResourceLegends = 6;

    /// @brief Constructor.
    ///
    /// @param [in] parent Pointer to the parent widget.
    explicit HeapOverviewHeapLayout(QWidget* parent = nullptr);

    /// @brief Destructor.
    virtual ~HeapOverviewHeapLayout();

    /// @brief Initialize the widget.
    ///
    /// @param [in] heap The heap this widget represents.
    void Initialize(RmtHeapType heap);

    /// @brief Update the widget.
    void Update();

    /// @brief Gets the width of the section containing the donut, legend, and a horizontal spacer.
    ///
    /// @return width of the donut section in pixels.
    int GetDonutSectionWidth() const;

    /// @brief Sets the minimum width of the donut section.
    ///
    /// @param [in] width The minimum width in pixels to set the donut section.
    void SetDonutSectionWidth(const int width);

protected:
    /// @brief Overridden window resize event.
    ///
    /// @param [in] event the resize event object.
    virtual void resizeEvent(QResizeEvent* event) Q_DECL_OVERRIDE;

private:
    Ui::HeapOverviewHeapLayout* ui_;  ///< Pointer to the Qt UI design.

    rmv::HeapOverviewHeapModel* model_;                                         ///< Container class for the widget model.
    ColoredLegendGraphicsView*  resource_legends_views_[kNumResourceLegends];   ///< Duplicate of ui elements in array form for easy access.
    ColoredLegendScene*         resource_legends_scenes_[kNumResourceLegends];  ///< Pointer to the heap residency legends scene.
};

#endif  // RMV_VIEWS_SNAPSHOT_HEAP_OVERVIEW_HEAP_LAYOUT_H_
