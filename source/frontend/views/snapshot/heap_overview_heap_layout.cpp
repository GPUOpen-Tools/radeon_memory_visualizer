//=============================================================================
// Copyright (c) 2019-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Implementation for a single heap in the heap overview pane.
//=============================================================================

#include "views/snapshot/heap_overview_heap_layout.h"

#include "qt_common/utils/qt_util.h"

#include "rmt_print.h"

#include "managers/message_manager.h"
#include "models/colorizer.h"
#include "settings/rmv_settings.h"
#include "util/string_util.h"
#include "util/widget_util.h"

// A string used to describe the data presented by the resource usage donut.
static const QString kResourceDonutDescription = "Committed %1 memory\nGrouped by Resource usage";

HeapOverviewHeapLayout::HeapOverviewHeapLayout(QWidget* parent)
    : QWidget(parent)
    , ui_(new Ui::HeapOverviewHeapLayout)
    , model_(nullptr)
    , resource_legends_views_{}
    , resource_legends_scenes_{}
{
    ui_->setupUi(this);

    // Set up the resource legends.
    resource_legends_views_[0] = ui_->legends_resource_1_;
    resource_legends_views_[1] = ui_->legends_resource_2_;
    resource_legends_views_[2] = ui_->legends_resource_3_;
    resource_legends_views_[3] = ui_->legends_resource_4_;
    resource_legends_views_[4] = ui_->legends_resource_5_;
    resource_legends_views_[5] = ui_->legends_resource_6_;

    if (QtCommon::QtUtils::ColorTheme::Get().GetColorTheme() == ColorThemeType::kColorThemeTypeDark)
    {
        ui_->warning_icon_->setPixmap(QPixmap(QString::fromUtf8(":/Resources/assets/third_party/ionicons/warning_white.svg")));
    }

    connect(&QtCommon::QtUtils::ColorTheme::Get(), &QtCommon::QtUtils::ColorTheme::ColorThemeUpdated, this, &HeapOverviewHeapLayout::OnColorThemeUpdated);

    for (int i = 0; i < kNumResourceLegends; i++)
    {
        rmv::widget_util::InitGraphicsView(resource_legends_views_[i], rmv::kColoredLegendsHeight);
        resource_legends_scenes_[i] = rmv::widget_util::InitColorLegend(resource_legends_views_[i]);
    }
}

HeapOverviewHeapLayout::~HeapOverviewHeapLayout()
{
    delete model_;
    for (int i = 0; i < kNumResourceLegends; i++)
    {
        delete resource_legends_scenes_[i];
    }
}

void HeapOverviewHeapLayout::Initialize(RmtHeapType heap)
{
    Q_ASSERT(model_ == nullptr);
    model_ = new rmv::HeapOverviewHeapModel(heap);

    model_->InitializeModel(ui_->title_label_, rmv::kHeapOverviewTitle, "text");
    model_->InitializeModel(ui_->title_description_, rmv::kHeapOverviewDescription, "text");
    model_->InitializeModel(ui_->heap_status_label_, rmv::kHeapOverviewHeapStatus, "text");

    model_->InitializeModel(ui_->warning_message_, rmv::kHeapOverviewWarningText, "text");

    model_->InitializeModel(ui_->content_location_, rmv::kHeapOverviewLocation, "text");
    model_->InitializeModel(ui_->content_cpu_cached_, rmv::kHeapOverviewCpuCached, "text");
    model_->InitializeModel(ui_->content_cpu_visible_, rmv::kHeapOverviewCpuVisible, "text");
    model_->InitializeModel(ui_->content_gpu_cached_, rmv::kHeapOverviewGpuCached, "text");
    model_->InitializeModel(ui_->content_gpu_visible_, rmv::kHeapOverviewGpuVisible, "text");
    model_->InitializeModel(ui_->content_committed_, rmv::kHeapOverviewCommitted, "text");
    model_->InitializeModel(ui_->content_allocation_count_, rmv::kHeapOverviewAllocationCount, "text");
    model_->InitializeModel(ui_->content_smallest_allocation_, rmv::kHeapOverviewSmallestAllocation, "text");
    model_->InitializeModel(ui_->content_largest_allocation_, rmv::kHeapOverviewLargestAllocation, "text");
    model_->InitializeModel(ui_->content_mean_allocation_, rmv::kHeapOverviewMeanAllocation, "text");
    model_->InitializeModel(ui_->content_resource_count_, rmv::kHeapOverviewResourceCount, "text");

    ui_->label_resource_donut_description_->setText(kResourceDonutDescription.arg(RmtGetHeapTypeNameFromHeapType(heap)));

    ui_->content_committed_->hide();
    ui_->label_committed_->hide();
}

int HeapOverviewHeapLayout::GetDonutSectionWidth() const
{
    int width = ui_->donut_widget_->sizeHint().width();
    return width;
}

void HeapOverviewHeapLayout::SetDonutSectionWidth(const int width)
{
    ui_->donut_widget_->setMinimumWidth(width);
}

void HeapOverviewHeapLayout::resizeEvent(QResizeEvent* event)
{
    // Set the bar graph width to 1/3rd of the screen.
    int bar_graph_width = this->width() / 3;
    ui_->bar_graph_widget_->setFixedWidth(bar_graph_width);

    QWidget::resizeEvent(event);
}

void HeapOverviewHeapLayout::Update()
{
    Q_ASSERT(model_ != nullptr);
    if (model_ != nullptr)
    {
        model_->Update();
    }

    // Hide the invisible heap if SAM is enabled.
    if ((model_->GetHeapType() == kRmtHeapTypeInvisible) && (model_->IsSAMSupported() || model_->IsCPUHostApertureSupported()))
    {
        setVisible(false);
    }
    else
    {
        setVisible(true);
    }

    bool show_warning_message = model_->ShowSubscriptionWarning();
    ui_->warning_widget_->setVisible(show_warning_message);

    ui_->resource_donut_->SetArcWidth(20);

    uint64_t resource_data[kRmtResourceUsageTypeCount * 2] = {0};
    int      num_other                                     = 0;
    int      num_resources                                 = model_->GetResourceData(kNumResourceLegends - 1, &resource_data[0], &num_other);

    if (num_resources > 0)
    {
        int num_segments = num_resources + ((num_other > 0) ? 1 : 0);
        ui_->resource_donut_->SetNumSegments(num_segments);

        for (int i = 0; i < kNumResourceLegends; i++)
        {
            resource_legends_scenes_[i]->Clear();
        }

        for (int i = 0; i < num_resources; i++)
        {
            RmtResourceUsageType type  = (RmtResourceUsageType)resource_data[i * 2];
            uint64_t             value = resource_data[(i * 2) + 1];

            ui_->resource_donut_->SetIndexValue(i, value);
            const QColor& color = rmv::Colorizer::GetResourceUsageColor(type);
            ui_->resource_donut_->SetIndexColor(i, color);
            resource_legends_scenes_[i]->AddColorLegendItem(color, RmtGetResourceUsageTypeNameFromResourceUsageType(type));
        }

        if (num_other > 0)
        {
            ui_->resource_donut_->SetIndexValue(num_segments - 1, num_other);
            ui_->resource_donut_->SetIndexColor(num_segments - 1, rmv::RMVSettings::Get().GetColorResourceFreeSpace());
            resource_legends_scenes_[num_segments - 1]->AddColorLegendItem(rmv::RMVSettings::Get().GetColorResourceFreeSpace(), "Other");
        }
    }
    else
    {
        ui_->resource_donut_->SetNumSegments(1);
        ui_->resource_donut_->SetIndexValue(0, 1);
        ui_->resource_donut_->SetIndexColor(0, rmv::RMVSettings::Get().GetColorResourceFreeSpace());
    }

    // Set the view size to match the scene size so the legends appear left-justified.
    for (int i = 0; i < kNumResourceLegends; i++)
    {
        resource_legends_views_[i]->setFixedSize(resource_legends_scenes_[i]->itemsBoundingRect().size().toSize());
    }

    // Get memory parameters from the model.
    uint64_t                     total_physical_size                      = 0;
    uint64_t                     total_virtual_memory_requested           = 0;
    uint64_t                     total_bound_virtual_memory               = 0;
    uint64_t                     total_physical_mapped_by_process         = 0;
    uint64_t                     total_physical_mapped_by_other_processes = 0;
    RmtSegmentSubscriptionStatus subscription_status                      = kRmtSegmentSubscriptionStatusUnderLimit;

    model_->GetMemoryParameters(total_physical_size,
                                total_virtual_memory_requested,
                                total_bound_virtual_memory,
                                total_physical_mapped_by_process,
                                total_physical_mapped_by_other_processes,
                                subscription_status);

    // Calc max size.
    uint64_t max_size = total_physical_mapped_by_process + total_physical_mapped_by_other_processes;
    if (total_virtual_memory_requested > max_size)
    {
        max_size = total_virtual_memory_requested;
    }
    if (total_physical_size > max_size)
    {
        max_size = total_physical_size;
    }
    if (total_bound_virtual_memory > max_size)
    {
        max_size = total_bound_virtual_memory;
    }

    // Apply memory parameters to the memory bars.
    ui_->bar_requested_->SetParameters(total_virtual_memory_requested, 0, max_size, true, subscription_status, false);
    ui_->bar_bound_->SetParameters(total_bound_virtual_memory, 0, max_size, false, subscription_status, false);
    ui_->bar_total_size_->SetParameters(total_physical_size, 0, max_size, false, subscription_status, true);

    // Update the various UI elements.
    ui_->bar_requested_->update();
    ui_->bar_bound_->update();
    ui_->bar_total_size_->update();

    ui_->donut_widget_->update();
}

void HeapOverviewHeapLayout::OnColorThemeUpdated()
{
    if (QtCommon::QtUtils::ColorTheme::Get().GetColorTheme() == ColorThemeType::kColorThemeTypeDark)
    {
        ui_->warning_icon_->setPixmap(QPixmap(QString::fromUtf8(":/Resources/assets/third_party/ionicons/warning_white.svg")));
    }
    else
    {
        ui_->warning_icon_->setPixmap(QPixmap(QString::fromUtf8(":/Resources/assets/third_party/ionicons/warning.svg")));
    }
}
