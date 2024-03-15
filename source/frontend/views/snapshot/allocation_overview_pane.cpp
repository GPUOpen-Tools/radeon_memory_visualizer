//=============================================================================
// Copyright (c) 2018-2024 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Implementation of the allocation overview pane.
//=============================================================================

#include "views/snapshot/allocation_overview_pane.h"

#include <QGraphicsView>
#include <QScrollBar>
#include <QCheckBox>
#include <QStyle>
#include <vector>

#include "qt_common/utils/scaling_manager.h"

#include "rmt_assert.h"
#include "rmt_data_snapshot.h"

#include "managers/pane_manager.h"
#include "managers/message_manager.h"
#include "managers/snapshot_manager.h"
#include "models/colorizer.h"
#include "models/heap_combo_box_model.h"
#include "models/snapshot/allocation_overview_model.h"
#include "settings/rmv_settings.h"
#include "util/widget_util.h"
#include "views/custom_widgets/rmv_allocation_bar.h"

// The number of graphic objects in the scene to show allocations. It's inefficient to
// have one graphics object per allocation, particularly when there are thousands of
// allocations. Instead, the graphic objects are positioned in the currently visible
// area of the scene. Rather than each graphic object having a fixed allocation index,
// an offset is added to each allocation depending on where the visible region of the
// scene is.
static const int32_t kMaxAllocationObjects = 100;

// Enum for the number of allocation models needed. For this pane, one model is needed for all
// allocations shown in the table.
enum
{
    kAllocationModelIndex,

    kNumAllocationModels,
};

// Map between sort type ID and its text representation.
// These are the items that will be added to the combo box.
static const std::map<int, QString> kSortTextMap = {{rmv::AllocationOverviewModel::kSortModeAllocationID, rmv::text::kSortByAllocationId},
                                                    {rmv::AllocationOverviewModel::kSortModeAllocationSize, rmv::text::kSortByAllocationSize},
                                                    {rmv::AllocationOverviewModel::kSortModeAllocationAge, rmv::text::kSortByAllocationAge},
                                                    {rmv::AllocationOverviewModel::kSortModeResourceCount, rmv::text::kSortByResourceCount},
                                                    {rmv::AllocationOverviewModel::kSortModeFragmentationScore, rmv::text::kSortByFragmentationScore}};

// Map between sort direction ID and its text representation.
// These are the items that will be added to the combo box.
static const std::map<int, QString> kDirectionTextMap = {
    {rmv::AllocationOverviewModel::kSortDirectionAscending, rmv::text::kSortAscending},
    {rmv::AllocationOverviewModel::kSortDirectionDescending, rmv::text::kSortDescending},
};

AllocationOverviewPane::AllocationOverviewPane(QWidget* parent)
    : BasePane(parent)
    , ui_(new Ui::allocation_overview_pane)
{
    ui_->setupUi(this);
    ui_->empty_page_->SetEmptyTitleText();

    rmv::widget_util::ApplyStandardPaneStyle(this, ui_->main_content_, ui_->main_scroll_area_);

    model_ = new rmv::AllocationOverviewModel(kNumAllocationModels);

    ui_->search_box_->setFixedWidth(rmv::kSearchBoxWidth);
    ui_->normalize_allocations_checkbox_->Initialize(false, rmv::kCheckboxEnableColor, Qt::black);
    ToggleNormalizeAllocations();
    connect(ui_->normalize_allocations_checkbox_, &RMVColoredCheckbox::Clicked, this, &AllocationOverviewPane::ToggleNormalizeAllocations);

    ui_->aliased_resource_checkbox_->Initialize(false, rmv::kCheckboxEnableColor, Qt::black);
    connect(ui_->aliased_resource_checkbox_, &RMVColoredCheckbox::Clicked, this, &AllocationOverviewPane::ToggleAliasedResources);

    rmv::widget_util::InitMultiSelectComboBox(this, ui_->preferred_heap_combo_box_, rmv::text::kPreferredHeap);
    rmv::widget_util::InitSingleSelectComboBox(this, ui_->sort_combo_box_, rmv::text::kSortByAllocationId, false);
    rmv::widget_util::InitSingleSelectComboBox(this, ui_->sort_direction_combo_box_, rmv::text::kSortAscending, false);

    connect(ui_->search_box_, &QLineEdit::textChanged, this, &AllocationOverviewPane::ApplyFilters);
    connect(ui_->preferred_heap_combo_box_, &ArrowIconComboBox::SelectionChanged, this, &AllocationOverviewPane::ApplyFilters);

    preferred_heap_combo_box_model_ = new rmv::HeapComboBoxModel();
    preferred_heap_combo_box_model_->SetupHeapComboBox(ui_->preferred_heap_combo_box_);
    connect(preferred_heap_combo_box_model_, &rmv::HeapComboBoxModel::FilterChanged, this, &AllocationOverviewPane::HeapChanged);

    // Add text strings to the sort combo box.
    ui_->sort_combo_box_->ClearItems();
    for (int loop = 0; loop < rmv::AllocationOverviewModel::kSortModeCount; loop++)
    {
        std::map<int, QString>::const_iterator it = kSortTextMap.find(loop);
        if (it != kSortTextMap.end())
        {
            ui_->sort_combo_box_->AddItem((*it).second);
        }
        else
        {
            RMT_ASSERT_MESSAGE(false, "Sort mode not found in list");
            ui_->sort_combo_box_->AddItem("Unknown");
        }
    }

    // Add text strings to the sort direction combo box.
    ui_->sort_direction_combo_box_->ClearItems();
    for (int loop = 0; loop < rmv::AllocationOverviewModel::kSortDirectionCount; loop++)
    {
        std::map<int, QString>::const_iterator it = kDirectionTextMap.find(loop);
        if (it != kDirectionTextMap.end())
        {
            ui_->sort_direction_combo_box_->AddItem((*it).second);
        }
        else
        {
            RMT_ASSERT_MESSAGE(false, "Sort direction not found in list");
            ui_->sort_direction_combo_box_->AddItem("Unknown");
        }
    }

    // Set up scrollbar parameters for the memory map graphics view.
    ui_->allocation_list_view_->setMouseTracking(true);
    ui_->allocation_list_view_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui_->allocation_list_view_->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    ui_->allocation_list_view_->horizontalScrollBar()->blockSignals(true);
    allocation_list_scene_ = new QGraphicsScene();
    ui_->allocation_list_view_->setScene(allocation_list_scene_);

    colorizer_ = new rmv::Colorizer();

    // Set up a list of required coloring modes, in order.
    // The list is terminated with kColorModeCount.
    static const rmv::Colorizer::ColorMode mode_list[] = {
        rmv::Colorizer::kColorModeResourceUsageType,
        rmv::Colorizer::kColorModePreferredHeap,
        rmv::Colorizer::kColorModeAllocationAge,
        rmv::Colorizer::kColorModeResourceCreateAge,
        rmv::Colorizer::kColorModeResourceBindAge,
        rmv::Colorizer::kColorModeResourceGUID,
        rmv::Colorizer::kColorModeResourceCPUMapped,
        rmv::Colorizer::kColorModeNotAllPreferred,
        rmv::Colorizer::kColorModeAliasing,
        rmv::Colorizer::kColorModeCommitType,
        rmv::Colorizer::kColorModeCount,
    };

    // Initialize the "color by" UI elements. Set up the combo box, legends and signals etc.
    colorizer_->Initialize(parent, ui_->color_combo_box_, ui_->legends_view_, mode_list);

    // Set up what happens when the user selects an item from the sort combo box.
    connect(ui_->sort_combo_box_, &ArrowIconComboBox::SelectionChanged, this, &AllocationOverviewPane::ApplySort);
    connect(ui_->sort_direction_combo_box_, &ArrowIconComboBox::SelectionChanged, this, &AllocationOverviewPane::ApplySort);
    connect(ui_->color_combo_box_, &ArrowIconComboBox::SelectionChanged, this, &AllocationOverviewPane::ColorModeChanged);
    connect(&rmv::MessageManager::Get(), &rmv::MessageManager::ResourceSelected, this, &AllocationOverviewPane::SelectResource);
    connect(&ScalingManager::Get(), &ScalingManager::ScaleFactorChanged, this, &AllocationOverviewPane::OnScaleFactorChanged);

    connect(ui_->allocation_height_slider_, &QSlider::valueChanged, this, [=]() { AllocationHeightChanged(); });
    connect(ui_->allocation_list_view_->verticalScrollBar(), &QScrollBar::valueChanged, this, [=]() { ResizeItems(); });

    ui_->allocation_height_slider_->setCursor(Qt::PointingHandCursor);

    allocation_height_ = ui_->allocation_height_slider_->value();
}

AllocationOverviewPane::~AllocationOverviewPane()
{
    disconnect(&ScalingManager::Get(), &ScalingManager::ScaleFactorChanged, this, &AllocationOverviewPane::OnScaleFactorChanged);

    int32_t current_size = static_cast<int32_t>(allocation_graphic_objects_.size());
    for (int32_t i = 0; i < current_size; i++)
    {
        RMVAllocationBar* allocation_item = allocation_graphic_objects_[i];
        allocation_list_scene_->removeItem(allocation_item);
        disconnect(allocation_item, &RMVAllocationBar::ResourceSelected, this, &AllocationOverviewPane::SelectedResource);
    }

    delete model_;
    delete preferred_heap_combo_box_model_;
    delete colorizer_;
}

void AllocationOverviewPane::OnTraceClose()
{
    model_->ResetModelValues();
    allocation_graphic_objects_.clear();
    allocation_list_scene_->clear();
    preferred_heap_combo_box_model_->ResetHeapComboBox(ui_->preferred_heap_combo_box_);
}

void AllocationOverviewPane::Reset()
{
    ui_->color_combo_box_->SetSelectedRow(0);
    colorizer_->ApplyColorMode();
    ui_->sort_combo_box_->SetSelectedRow(0);
    ui_->sort_direction_combo_box_->SetSelectedRow(0);
    ui_->normalize_allocations_checkbox_->setChecked(false);
    ToggleNormalizeAllocations();

    ui_->aliased_resource_checkbox_->setChecked(false);
    ToggleAliasedResources();

    ui_->allocation_height_slider_->setSliderPosition(0);

    ui_->search_box_->setText("");
}

void AllocationOverviewPane::ChangeColoring()
{
    colorizer_->UpdateLegends();
    ResizeItems();
}

void AllocationOverviewPane::ColorModeChanged()
{
    ChangeColoring();
    ui_->allocation_list_view_->viewport()->update();
}

void AllocationOverviewPane::OpenSnapshot(RmtDataSnapshot* snapshot)
{
    RMT_ASSERT(snapshot != nullptr);
    if (snapshot != nullptr && rmv::SnapshotManager::Get().LoadedSnapshotValid())
    {
        ui_->pane_stack_->setCurrentIndex(rmv::kSnapshotIndexPopulatedPane);
        ui_->sort_combo_box_->SetSelectedRow(0);
        ui_->sort_direction_combo_box_->SetSelectedRow(0);
        model_->ResetModelValues();

        if (snapshot->virtual_allocation_list.allocation_count > 0)
        {
            // Remove any old allocations from the last snapshot and disconnect any connections.
            size_t current_size = allocation_graphic_objects_.size();
            for (size_t i = 0; i < current_size; i++)
            {
                RMVAllocationBar* allocation_item = allocation_graphic_objects_[i];
                disconnect(allocation_item, &RMVAllocationBar::ResourceSelected, this, &AllocationOverviewPane::SelectedResource);
            }
            allocation_graphic_objects_.clear();
            allocation_list_scene_->clear();

            // Add the graphics items to the scene, one item per allocation.
            int32_t count = std::min<int>(kMaxAllocationObjects, snapshot->virtual_allocation_list.allocation_count);
            for (int i = 0; i < count; i++)
            {
                RMVAllocationBar* allocation_item = new RMVAllocationBar(model_->GetAllocationBarModel(), i, kAllocationModelIndex, colorizer_);
                allocation_list_scene_->addItem(allocation_item);
                allocation_graphic_objects_.push_back(allocation_item);
                connect(allocation_item, &RMVAllocationBar::ResourceSelected, this, &AllocationOverviewPane::SelectedResource);
            }

            // Apply filters and sorting to the newly added items.
            ApplyFilters();
            ApplySort();
        }

        ResizeItems();
    }
    else
    {
        ui_->pane_stack_->setCurrentIndex(rmv::kSnapshotIndexEmptyPane);
    }
}

void AllocationOverviewPane::resizeEvent(QResizeEvent* event)
{
    ResizeItems();
    QWidget::resizeEvent(event);
}

void AllocationOverviewPane::showEvent(QShowEvent* event)
{
    ResizeItems();
    QWidget::showEvent(event);
}

void AllocationOverviewPane::hideEvent(QHideEvent* event)
{
    QWidget::hideEvent(event);
}

void AllocationOverviewPane::AllocationHeightChanged()
{
    QScrollBar* scroll_bar = ui_->allocation_list_view_->verticalScrollBar();
    if (scroll_bar != nullptr)
    {
        const int32_t new_allocation_height = ui_->allocation_height_slider_->value();

        if (allocation_height_ != new_allocation_height)
        {
            // Reposition the slider so the same allocation as before is in view.
            int32_t scroll_bar_offset = scroll_bar->value();
            scroll_bar_offset *= new_allocation_height;
            scroll_bar_offset /= allocation_height_;
            allocation_height_ = new_allocation_height;

            scroll_bar->setValue(scroll_bar_offset);
            ResizeItems();
        }
    }
}

void AllocationOverviewPane::ResizeItems()
{
    int allocation_offset = 0;

    const qreal       scaled_allocation_height = ScalingManager::Get().Scaled(allocation_height_);
    const QScrollBar* scroll_bar               = ui_->allocation_list_view_->verticalScrollBar();
    if (scroll_bar != nullptr)
    {
        allocation_offset = scroll_bar->value() / scaled_allocation_height;
    }

    model_->GetAllocationBarModel()->SetAllocationOffset(allocation_offset);

    int       y_offset        = allocation_offset * scaled_allocation_height;
    const int scrollbar_width = qApp->style()->pixelMetric(QStyle::PM_ScrollBarExtent);
    const int view_width      = ui_->allocation_list_view_->width() - scrollbar_width - 2;

    const size_t num_objects = allocation_graphic_objects_.size();

    for (size_t current_allocation_graphic_object_index = 0; current_allocation_graphic_object_index < num_objects; current_allocation_graphic_object_index++)
    {
        allocation_graphic_objects_[current_allocation_graphic_object_index]->UpdateDimensions(view_width, scaled_allocation_height);
        allocation_graphic_objects_[current_allocation_graphic_object_index]->setPos(0, y_offset);

        // Move down based on the size of the item that was just added.
        y_offset += allocation_graphic_objects_[current_allocation_graphic_object_index]->boundingRect().height();
    }

    UpdateAllocationListSceneRect();
}

void AllocationOverviewPane::ApplyFilters()
{
    preferred_heap_combo_box_model_->SetupState(ui_->preferred_heap_combo_box_);

    bool heaps[kRmtHeapTypeCount];
    for (uint32_t i = 0; i < (uint32_t)kRmtHeapTypeCount; i++)
    {
        heaps[i] = preferred_heap_combo_box_model_->ItemInList(i);
    }

    // This does not show/hide items from the scene,
    // instead it changes the underlying objects that are referenced by the items in the scene.
    model_->ApplyFilters(ui_->search_box_->text(), &heaps[0]);

    UpdateAllocationListSceneRect();
}

// Update the scene rect of the allocationListView.
//
// Background:
// This is needed because the allocationListView always has all the items in it based
// on the number of allocations that were made. When a filter is applied to the model,
// it is not being done through a proxy model and the items in the graphics view are not
// being hidden. Instead the actual model is being changed, causing the items in the
// view to reference different data that has been moved to the same index.
// For example, the item at the top of the view is always index 0 and is always visible,
// but if a heap filter or search term happens to remove all the allocations, then all
// the allocations will skip their own painting, but the scene thinks they are still
// being displayed. As such, using a call like view->scene()->itemsBoundingRect() to get
// the height of visible allocations does not work properly - it always returns a size
// that is big enough to display all the allocations, even if they have been filtered out.
//
// About this function:
// This function will instead only update the scene rect if there is at least one item
// that is not filtered out and will retrieve the size of the first item and assume that
// all items are the same height. For this view, it is currently a safe assumption.
// An alternative approach, but more expensive, would be to iterate through every item
// and accumulate their heights if the model has an allocation for that particular index.
void AllocationOverviewPane::UpdateAllocationListSceneRect()
{
    const size_t num_allocations  = model_->GetViewableAllocationCount();
    const int    scroll_bar_width = qApp->style()->pixelMetric(QStyle::PM_ScrollBarExtent);
    const int    view_width       = ui_->allocation_list_view_->width() - scroll_bar_width - 2;
    int          item_height      = 0;

    if (num_allocations > 0 && allocation_graphic_objects_.size() > 0)
    {
        // Since each item has the same height, get the height of the first item and
        // update the scene rect according to the number of items that will actually get painted.
        item_height = allocation_graphic_objects_[0]->boundingRect().height();
    }

    allocation_list_scene_->setSceneRect(0, 0, view_width, num_allocations * item_height);
    ui_->allocation_list_view_->viewport()->update();
}

void AllocationOverviewPane::ApplySort()
{
    int  sortMode  = ui_->sort_combo_box_->CurrentRow();
    bool ascending = (ui_->sort_direction_combo_box_->CurrentRow() == rmv::AllocationOverviewModel::kSortDirectionAscending);

    model_->Sort(sortMode, ascending);
    ui_->allocation_list_view_->viewport()->update();
}

void AllocationOverviewPane::SelectedResource(RmtResourceIdentifier resource_identifier, bool navigate_to_pane)
{
    // Broadcast the resource selection to any listening panes
    // or the user needs to navigate because they double clicked.
    emit rmv::MessageManager::Get().ResourceSelected(resource_identifier);

    if (navigate_to_pane == true)
    {
        emit rmv::MessageManager::Get().PaneSwitchRequested(rmv::kPaneIdSnapshotAllocationExplorer);
    }
}

void AllocationOverviewPane::SelectResource(RmtResourceIdentifier resource_identifier)
{
    size_t allocation_offset = model_->SelectResource(resource_identifier, kAllocationModelIndex);

    QScrollBar* scroll_bar = ui_->allocation_list_view_->verticalScrollBar();
    if (scroll_bar != nullptr && allocation_offset != UINT64_MAX)
    {
        const qreal   scaled_allocation_height = ScalingManager::Get().Scaled(allocation_height_);
        const int32_t view_height              = ui_->allocation_list_view_->height();
        const int32_t scroll_bar_offset        = scroll_bar->value();
        const int32_t top_allocation_index     = scroll_bar_offset / scaled_allocation_height;
        const int32_t bottom_allocation_index  = (scroll_bar_offset + view_height) / scaled_allocation_height;

        // If the allocation is outside the visible range, move the scrollbar so it's in range.
        // If the allocation is partially visible, move the scrollbar so it's all visible.
        if (static_cast<int32_t>(allocation_offset) <= top_allocation_index)
        {
            scroll_bar->setValue(static_cast<int32_t>(allocation_offset) * scaled_allocation_height);
        }
        else if (static_cast<int32_t>(allocation_offset) >= bottom_allocation_index)
        {
            int32_t offset = static_cast<int32_t>(allocation_offset) * scaled_allocation_height;
            offset -= view_height;
            offset += allocation_height_;
            scroll_bar->setValue(offset);
        }
    }

    ui_->allocation_list_view_->viewport()->update();
}

void AllocationOverviewPane::HeapChanged(bool checked)
{
    RMT_UNUSED(checked);
    ApplyFilters();
}

void AllocationOverviewPane::OnScaleFactorChanged()
{
    ResizeItems();
}

void AllocationOverviewPane::ToggleNormalizeAllocations()
{
    bool checked = ui_->normalize_allocations_checkbox_->isChecked();
    model_->SetNormalizeAllocations(checked);
    ui_->allocation_list_view_->viewport()->update();
}

void AllocationOverviewPane::ToggleAliasedResources() const
{
    bool checked = ui_->aliased_resource_checkbox_->isChecked();
    model_->GetAllocationBarModel()->ShowAliased(checked);
    ui_->allocation_list_view_->viewport()->update();
}
