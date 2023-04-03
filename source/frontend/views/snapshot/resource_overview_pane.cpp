//=============================================================================
// Copyright (c) 2018-2023 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Implementation of the Resource Overview pane.
//=============================================================================

#include "views/snapshot/resource_overview_pane.h"

#include <QCheckBox>

#include "qt_common/custom_widgets/colored_legend_scene.h"
#include "qt_common/utils/scaling_manager.h"

#include "rmt_data_snapshot.h"

#include "managers/trace_manager.h"
#include "managers/message_manager.h"
#include "managers/pane_manager.h"
#include "managers/snapshot_manager.h"
#include "models/heap_combo_box_model.h"
#include "models/resource_usage_combo_box_model.h"
#include "models/snapshot/resource_overview_model.h"
#include "settings/rmv_settings.h"
#include "util/widget_util.h"
#include "views/custom_widgets/rmv_colored_checkbox.h"

ResourceOverviewPane::ResourceOverviewPane(QWidget* parent)
    : BasePane(parent)
    , ui_(new Ui::ResourceOverviewPane)
    , selected_resource_(nullptr)
    , resource_details_(nullptr)
    , allocation_details_scene_(nullptr)
    , slice_mode_map_{}
{
    ui_->setupUi(this);
    ui_->empty_page_->SetEmptyTitleText();

    rmv::widget_util::ApplyStandardPaneStyle(this, ui_->main_content_, ui_->main_scroll_area_);

    model_ = new rmv::ResourceOverviewModel();

    model_->InitializeModel(ui_->total_available_size_value_, rmv::kResourceOverviewTotalAvailableSize, "text");
    model_->InitializeModel(ui_->total_allocated_and_used_value_, rmv::kResourceOverviewTotalAllocatedAndUsed, "text");
    model_->InitializeModel(ui_->total_allocated_and_unused_value_, rmv::kResourceOverviewTotalAllocatedAndUnused, "text");
    model_->InitializeModel(ui_->allocations_value_, rmv::kResourceOverviewAllocationCount, "text");
    model_->InitializeModel(ui_->resources_value_, rmv::kResourceOverviewResourceCount, "text");

    rmv::widget_util::InitMultiSelectComboBox(this, ui_->preferred_heap_combo_box_, rmv::text::kPreferredHeap);
    rmv::widget_util::InitMultiSelectComboBox(this, ui_->actual_heap_combo_box_, rmv::text::kActualHeap);
    rmv::widget_util::InitMultiSelectComboBox(this, ui_->resource_usage_combo_box_, rmv::text::kResourceUsage);
    rmv::widget_util::InitSingleSelectComboBox(this, ui_->slicing_button_one_, "", false, "Level 1: ");
    rmv::widget_util::InitSingleSelectComboBox(this, ui_->slicing_button_two_, "", false, "Level 2: ");
    rmv::widget_util::InitSingleSelectComboBox(this, ui_->slicing_button_three_, "", false, "Level 3: ");

    // Hide actual heap as it's not that useful currently.
    ui_->actual_heap_combo_box_->hide();

    tree_map_models_.preferred_heap_model = new rmv::HeapComboBoxModel();
    tree_map_models_.actual_heap_model    = new rmv::HeapComboBoxModel();
    tree_map_models_.resource_usage_model = new rmv::ResourceUsageComboBoxModel();
    colorizer_                            = new rmv::Colorizer();

    tree_map_models_.preferred_heap_model->SetupHeapComboBox(ui_->preferred_heap_combo_box_);
    connect(tree_map_models_.preferred_heap_model, &rmv::HeapComboBoxModel::FilterChanged, this, &ResourceOverviewPane::ComboFiltersChanged);

    tree_map_models_.actual_heap_model->SetupHeapComboBox(ui_->actual_heap_combo_box_);
    connect(tree_map_models_.actual_heap_model, &rmv::HeapComboBoxModel::FilterChanged, this, &ResourceOverviewPane::ComboFiltersChanged);

    tree_map_models_.resource_usage_model->SetupResourceComboBox(ui_->resource_usage_combo_box_);
    connect(tree_map_models_.resource_usage_model, &rmv::ResourceUsageComboBoxModel::FilterChanged, this, &ResourceOverviewPane::ComboFiltersChanged);

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

    ui_->tree_map_view_->SetModels(model_, &tree_map_models_, colorizer_);

    struct SliceMapping
    {
        int     slice_id;
        QString slice_text;
    };

    static const std::vector<SliceMapping> slice_map = {
        {RMVTreeMapBlocks::kSliceTypeNone, "no slicing"},
        {RMVTreeMapBlocks::kSliceTypeResourceUsageType, "slice by resource usage"},
        {RMVTreeMapBlocks::kSliceTypePreferredHeap, "slice by preferred heap"},
        {RMVTreeMapBlocks::kSliceTypeAllocationAge, "slice by allocation age"},
        {RMVTreeMapBlocks::kSliceTypeResourceCreateAge, "slice by resource create time"},
        {RMVTreeMapBlocks::kSliceTypeResourceBindAge, "slice by resource bind time"},
        {RMVTreeMapBlocks::kSliceTypeVirtualAllocation, "slice by virtual allocation"},
        //{RMVTreeMapBlocks::kSliceTypeActualHeap, "slice by actual heap"},
        {RMVTreeMapBlocks::kSliceTypeCpuMapped, "slice by CPU mapped"},
        //{RMVTreeMapBlocks::kSliceTypeResourceOwner, "slice by resource owner"},
        {RMVTreeMapBlocks::kSliceTypeInPreferredHeap, "slice by not all in preferred heap"}
        //{RMVTreeMapBlocks::kSliceTypeResourceCommitType, "slice by commit type"}
    };

    int index = 0;
    for (auto it = slice_map.begin(); it != slice_map.end(); ++it)
    {
        const QString& slice_string = (*it).slice_text;
        ui_->slicing_button_one_->AddItem(slice_string);
        ui_->slicing_button_two_->AddItem(slice_string);
        ui_->slicing_button_three_->AddItem(slice_string);

        // Add the indices to a map of combo box index to slice type.
        slice_mode_map_[index] = (*it).slice_id;
        index++;
    }

    ui_->resource_details_view_->setFrameStyle(QFrame::NoFrame);
    ui_->resource_details_view_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui_->resource_details_view_->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui_->resource_details_view_->setFixedHeight(ScalingManager::Get().Scaled(kResourceDetailsHeight));

    allocation_details_scene_ = new QGraphicsScene();
    ui_->resource_details_view_->setScene(allocation_details_scene_);

    RMVResourceDetailsConfig config = {};
    config.width                    = ui_->resource_details_view_->width();
    config.height                   = ui_->resource_details_view_->height();
    config.resource_valid           = false;
    config.allocation_thumbnail     = true;
    config.colorizer                = colorizer_;

    resource_details_ = new RMVResourceDetails(config);
    allocation_details_scene_->addItem(resource_details_);

    ui_->resource_details_checkbox_->Initialize(true, rmv::kCheckboxEnableColor, Qt::black);

    rmv::widget_util::InitDoubleSlider(ui_->size_slider_);

    connect(ui_->size_slider_, &DoubleSliderWidget::SpanChanged, this, &ResourceOverviewPane::FilterBySizeSliderChanged);

    connect(ui_->resource_details_checkbox_, &RMVColoredCheckbox::Clicked, this, &ResourceOverviewPane::ToggleResourceDetails);
    connect(ui_->slicing_button_one_, &ArrowIconComboBox::SelectionChanged, this, &ResourceOverviewPane::SlicingLevelChanged);
    connect(ui_->slicing_button_two_, &ArrowIconComboBox::SelectionChanged, this, &ResourceOverviewPane::SlicingLevelChanged);
    connect(ui_->slicing_button_three_, &ArrowIconComboBox::SelectionChanged, this, &ResourceOverviewPane::SlicingLevelChanged);
    connect(ui_->tree_map_view_->BlocksWidget(), &RMVTreeMapBlocks::ResourceSelected, this, &ResourceOverviewPane::SelectedResource);
    connect(ui_->tree_map_view_->BlocksWidget(), &RMVTreeMapBlocks::UnboundResourceSelected, this, &ResourceOverviewPane::SelectedUnboundResource);
    connect(ui_->color_combo_box_, &ArrowIconComboBox::SelectionChanged, this, &ResourceOverviewPane::ColorModeChanged);
    connect(&rmv::MessageManager::Get(), &rmv::MessageManager::ResourceSelected, this, &ResourceOverviewPane::SelectResource);
    connect(&ScalingManager::Get(), &ScalingManager::ScaleFactorChanged, this, &ResourceOverviewPane::OnScaleFactorChanged);
}

ResourceOverviewPane::~ResourceOverviewPane()
{
    disconnect(&ScalingManager::Get(), &ScalingManager::ScaleFactorChanged, this, &ResourceOverviewPane::OnScaleFactorChanged);

    delete tree_map_models_.preferred_heap_model;
    delete tree_map_models_.actual_heap_model;
    delete tree_map_models_.resource_usage_model;
    delete colorizer_;
    delete model_;
}

void ResourceOverviewPane::Refresh()
{
    bool use_unbound = tree_map_models_.resource_usage_model->ItemInList(kRmtResourceUsageTypeFree);
    model_->Update(use_unbound);
}

void ResourceOverviewPane::showEvent(QShowEvent* event)
{
    ResizeItems();
    QWidget::showEvent(event);
}

void ResourceOverviewPane::resizeEvent(QResizeEvent* event)
{
    ResizeItems();
    QWidget::resizeEvent(event);
}

int ResourceOverviewPane::GetRowForSliceType(int slice_type)
{
    for (int i = 0; i < RMVTreeMapBlocks::SliceType::kSliceTypeCount; i++)
    {
        if (slice_mode_map_[i] == slice_type)
        {
            return i;
        }
    }
    return 0;
}

void ResourceOverviewPane::Reset()
{
    selected_resource_ = nullptr;
    resource_details_->UpdateResource(selected_resource_);

    ui_->slicing_button_one_->SetSelectedRow(GetRowForSliceType(RMVTreeMapBlocks::kSliceTypePreferredHeap));
    ui_->slicing_button_two_->SetSelectedRow(GetRowForSliceType(RMVTreeMapBlocks::kSliceTypeVirtualAllocation));
    ui_->slicing_button_three_->SetSelectedRow(GetRowForSliceType(RMVTreeMapBlocks::kSliceTypeResourceUsageType));

    tree_map_models_.preferred_heap_model->ResetHeapComboBox(ui_->preferred_heap_combo_box_);
    tree_map_models_.actual_heap_model->ResetHeapComboBox(ui_->actual_heap_combo_box_);
    tree_map_models_.resource_usage_model->ResetResourceComboBox(ui_->resource_usage_combo_box_);

    ui_->color_combo_box_->SetSelectedRow(0);
    colorizer_->ApplyColorMode();
    ui_->size_slider_->SetLowerValue(0);
    ui_->size_slider_->SetUpperValue(rmv::kSizeSliderRange);

    UpdateDetailsTitle();
}

void ResourceOverviewPane::ChangeColoring()
{
    ui_->tree_map_view_->UpdateColorCache();
    resource_details_->UpdateResource(selected_resource_);
    colorizer_->UpdateLegends();
    Refresh();
}

void ResourceOverviewPane::OpenSnapshot(RmtDataSnapshot* snapshot)
{
    Q_UNUSED(snapshot);

    if (rmv::SnapshotManager::Get().LoadedSnapshotValid())
    {
        ui_->pane_stack_->setCurrentIndex(rmv::kSnapshotIndexPopulatedPane);
        selected_resource_ = nullptr;
        resource_details_->UpdateResource(selected_resource_);
        UpdateDetailsTitle();

        bool use_unbound = tree_map_models_.resource_usage_model->ItemInList(kRmtResourceUsageTypeFree);
        model_->Update(use_unbound);
        UpdateSlicingLevel();
        UpdateComboFilters();
        ui_->tree_map_view_->UpdateTreeMap();
    }
    else
    {
        ui_->pane_stack_->setCurrentIndex(rmv::kSnapshotIndexEmptyPane);
    }
}

void ResourceOverviewPane::ToggleResourceDetails()
{
    if (ui_->resource_details_checkbox_->isChecked())
    {
        ui_->resource_details_->show();
    }
    else
    {
        ui_->resource_details_->hide();
    }

    UpdateDetailsTitle();
}

void ResourceOverviewPane::UpdateComboFilters()
{
    tree_map_models_.preferred_heap_model->SetupState(ui_->preferred_heap_combo_box_);
    tree_map_models_.actual_heap_model->SetupState(ui_->actual_heap_combo_box_);
    tree_map_models_.resource_usage_model->SetupState(ui_->resource_usage_combo_box_);
}

void ResourceOverviewPane::ComboFiltersChanged(bool checked)
{
    RMT_UNUSED(checked);

    UpdateComboFilters();
    Refresh();

    selected_resource_ = nullptr;
    resource_details_->UpdateResource(selected_resource_);
    UpdateDetailsTitle();
    ui_->tree_map_view_->UpdateTreeMap();
}

void ResourceOverviewPane::UpdateSlicingLevel()
{
    QVector<RMVTreeMapBlocks::SliceType> slicingTypes;

    RMVTreeMapBlocks::SliceType typeLevelOne   = (RMVTreeMapBlocks::SliceType)slice_mode_map_[ui_->slicing_button_one_->CurrentRow()];
    RMVTreeMapBlocks::SliceType typeLevelTwo   = (RMVTreeMapBlocks::SliceType)slice_mode_map_[ui_->slicing_button_two_->CurrentRow()];
    RMVTreeMapBlocks::SliceType typeLevelThree = (RMVTreeMapBlocks::SliceType)slice_mode_map_[ui_->slicing_button_three_->CurrentRow()];

    if (typeLevelOne || typeLevelTwo || typeLevelThree)
    {
        if (typeLevelOne)
        {
            slicingTypes.push_back(typeLevelOne);
        }
        if (typeLevelTwo)
        {
            slicingTypes.push_back(typeLevelTwo);
        }
        if (typeLevelThree)
        {
            slicingTypes.push_back(typeLevelThree);
        }
    }

    ui_->tree_map_view_->UpdateSliceTypes(slicingTypes);
}

void ResourceOverviewPane::SlicingLevelChanged()
{
    UpdateSlicingLevel();
    ui_->tree_map_view_->UpdateTreeMap();
}

void ResourceOverviewPane::ColorModeChanged()
{
    ChangeColoring();
}

void ResourceOverviewPane::SelectedResource(RmtResourceIdentifier resource_identifier, bool broadcast, bool navigate_to_pane)
{
    if (broadcast == true)
    {
        emit rmv::MessageManager::Get().ResourceSelected(resource_identifier);
    }
    else
    {
        SelectResource(resource_identifier);
    }

    if (navigate_to_pane == true)
    {
        emit rmv::MessageManager::Get().PaneSwitchRequested(rmv::kPaneIdSnapshotResourceDetails);
    }
}

void ResourceOverviewPane::SelectedUnboundResource(const RmtResource* unbound_resource, bool broadcast, bool navigate_to_pane)
{
    if (broadcast == true)
    {
        if (unbound_resource != nullptr)
        {
            emit rmv::MessageManager::Get().UnboundResourceSelected(unbound_resource->bound_allocation);
        }
    }
    SelectUnboundResource(unbound_resource);
    emit rmv::MessageManager::Get().ResourceSelected(0);

    if (navigate_to_pane == true)
    {
        emit rmv::MessageManager::Get().PaneSwitchRequested(rmv::kPaneIdSnapshotAllocationExplorer);
    }
}

void ResourceOverviewPane::SelectResource(RmtResourceIdentifier resource_identifier)
{
    if (resource_identifier == 0)
    {
        return;
    }

    ui_->tree_map_view_->SelectResource(resource_identifier);

    RmtDataSnapshot* open_snapshot = rmv::SnapshotManager::Get().GetOpenSnapshot();

    if (rmv::TraceManager::Get().DataSetValid() && open_snapshot != nullptr)
    {
        selected_resource_ = nullptr;

        const RmtErrorCode error_code = RmtResourceListGetResourceByResourceId(&open_snapshot->resource_list, resource_identifier, &selected_resource_);
        if (error_code == kRmtOk)
        {
            resource_details_->UpdateResource(selected_resource_);
        }

        UpdateDetailsTitle();
    }
}

void ResourceOverviewPane::SelectUnboundResource(const RmtResource* unbound_resource)
{
    if (unbound_resource == nullptr)
    {
        return;
    }

    resource_details_->UpdateResource(unbound_resource);
    UpdateDetailsTitle();
}

void ResourceOverviewPane::FilterBySizeSliderChanged(int min_value, int max_value)
{
    bool use_unbound = tree_map_models_.resource_usage_model->ItemInList(kRmtResourceUsageTypeFree);
    model_->FilterBySizeChanged(min_value, max_value, use_unbound);
    ui_->tree_map_view_->UpdateTreeMap();
}

void ResourceOverviewPane::UpdateDetailsTitle()
{
    if (selected_resource_ == nullptr)
    {
        ui_->resource_name_label_->setText("Select a resource");

        if (ui_->resource_details_checkbox_->isChecked())
        {
            ui_->resource_name_label_minimized_->setText("");
        }
        else
        {
            ui_->resource_name_label_minimized_->setText("Select a resource");
        }
    }
    else
    {
        ui_->resource_name_label_->setText(selected_resource_->name);

        if (ui_->resource_details_checkbox_->isChecked())
        {
            ui_->resource_name_label_minimized_->setText("");
        }
        else
        {
            ui_->resource_name_label_minimized_->setText(selected_resource_->name);
        }
    }
}

void ResourceOverviewPane::ResizeItems()
{
    if (allocation_details_scene_ != nullptr)
    {
        const QRectF scene_rect = QRectF(0, 0, ui_->resource_details_view_->width(), ui_->resource_details_view_->height());

        allocation_details_scene_->setSceneRect(scene_rect);

        resource_details_->UpdateDimensions(ui_->resource_details_view_->width(), ui_->resource_details_view_->height());
    }
}

void ResourceOverviewPane::OnScaleFactorChanged()
{
    ui_->resource_details_view_->setFixedHeight(ScalingManager::Get().Scaled(kResourceDetailsHeight));
}
