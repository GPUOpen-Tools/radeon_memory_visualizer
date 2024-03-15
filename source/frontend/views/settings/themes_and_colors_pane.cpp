//=============================================================================
// Copyright (c) 2019-2024 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Implementation of Colors and Themes pane.
//=============================================================================

#include "views/settings/themes_and_colors_pane.h"

#include "settings/rmv_settings.h"
#include "util/rmv_util.h"
#include "util/widget_util.h"
#include "util/string_util.h"
#include "views/debug_window.h"

const static int kPickerRows    = 4;
const static int kPickerColumns = 8;

ThemesAndColorsPane::ThemesAndColorsPane(QWidget* parent)
    : BasePane(parent)
    , ui_(new Ui::ThemesAndColorsPane)
{
    ui_->setupUi(this);

    rmv::widget_util::ApplyStandardPaneStyle(this, ui_->main_content_, ui_->main_scroll_area_);

    // Set up buttons using RMVSettingID's as buttongroup id's.
    button_group_.addButton(ui_->button_snapshots_viewed_, kSettingThemesAndColorsSnapshotViewed);
    button_group_.addButton(ui_->button_snapshots_compared_, kSettingThemesAndColorsSnapshotCompared);
    button_group_.addButton(ui_->button_snapshots_live_, kSettingThemesAndColorsSnapshotLive);
    button_group_.addButton(ui_->button_snapshots_generated_, kSettingThemesAndColorsSnapshotGenerated);
    button_group_.addButton(ui_->button_snapshots_vma_, kSettingThemesAndColorsSnapshotVma);

    button_group_.addButton(ui_->button_resource_depth_stencil_buffer_, kSettingThemesAndColorsResourceDsBuffer);
    button_group_.addButton(ui_->button_resource_render_target_, kSettingThemesAndColorsResourceRenderTarget);
    button_group_.addButton(ui_->button_resource_texture_, kSettingThemesAndColorsResourceTexture);
    button_group_.addButton(ui_->button_resource_ray_tracing_buffer_, kSettingThemesAndColorsResourceRayTracingBuffer);
    button_group_.addButton(ui_->button_resource_shader_pipeline_, kSettingThemesAndColorsResourceShaderPipeline);
    button_group_.addButton(ui_->button_resource_command_buffer_, kSettingThemesAndColorsResourceCommandBuffer);
    button_group_.addButton(ui_->button_resource_heap_, kSettingThemesAndColorsResourceHeap);
    button_group_.addButton(ui_->button_resource_descriptors_, kSettingThemesAndColorsResourceDescriptors);
    button_group_.addButton(ui_->button_resource_buffer_, kSettingThemesAndColorsResourceBuffer);
    button_group_.addButton(ui_->button_resource_gpu_event_, kSettingThemesAndColorsResourceGpuEvent);
    button_group_.addButton(ui_->button_resource_free_space_, kSettingThemesAndColorsResourceFreeSpace);
    button_group_.addButton(ui_->button_resource_internal_, kSettingThemesAndColorsResourceInternal);

    button_group_.addButton(ui_->button_delta_increase_, kSettingThemesAndColorsDeltaIncrease);
    button_group_.addButton(ui_->button_delta_decrease_, kSettingThemesAndColorsDeltaDecrease);
    button_group_.addButton(ui_->button_delta_no_change_, kSettingThemesAndColorsDeltaNoChange);

    button_group_.addButton(ui_->button_heap_local_, kSettingThemesAndColorsHeapLocal);
    button_group_.addButton(ui_->button_heap_invisible_, kSettingThemesAndColorsHeapInvisible);
    button_group_.addButton(ui_->button_heap_system_, kSettingThemesAndColorsHeapSystem);
    button_group_.addButton(ui_->button_heap_unspecified_, kSettingThemesAndColorsHeapUnspecified);

    button_group_.addButton(ui_->button_cpu_mapped_, kSettingThemesAndColorsCpuMapped);
    button_group_.addButton(ui_->button_not_cpu_mapped_, kSettingThemesAndColorsNotCpuMapped);

    button_group_.addButton(ui_->button_in_preferred_heap_, kSettingThemesAndColorsInPreferredHeap);
    button_group_.addButton(ui_->button_not_in_preferred_heap_, kSettingThemesAndColorsNotInPreferredHeap);

    button_group_.addButton(ui_->button_aliased_, kSettingThemesAndColorsAliased);
    button_group_.addButton(ui_->button_not_aliased_, kSettingThemesAndColorsNotAliased);

    button_group_.addButton(ui_->button_resource_history_resource_event_, kSettingThemesAndColorsResourceHistoryResourceEvent);
    button_group_.addButton(ui_->button_resource_history_cpu_mapping_, kSettingThemesAndColorsResourceHistoryCpuMapUnmap);
    button_group_.addButton(ui_->button_resource_history_residency_, kSettingThemesAndColorsResourceHistoryResidencyUpdate);
    button_group_.addButton(ui_->button_resource_history_page_table_, kSettingThemesAndColorsResourceHistoryPageTableUpdate);
    button_group_.addButton(ui_->button_resource_history_highlight_, kSettingThemesAndColorsResourceHistoryHighlight);
    button_group_.addButton(ui_->button_resource_history_snapshot_, kSettingThemesAndColorsResourceHistorySnapshot);

    button_group_.addButton(ui_->button_commit_type_committed_, kSettingThemesAndColorsCommitTypeCommitted);
    button_group_.addButton(ui_->button_commit_type_placed_, kSettingThemesAndColorsCommitTypePlaced);
    button_group_.addButton(ui_->button_commit_type_virtual_, kSettingThemesAndColorsCommitTypeVirtual);

    // Slot/signal connection for various widgets.
    connect(ui_->color_widget_, &RMVColorPickerWidget::ColorSelected, this, &ThemesAndColorsPane::PickerColorSelected);
    connect(&button_group_, &QButtonGroup::idClicked, this, &ThemesAndColorsPane::ItemButtonClicked);
    connect(ui_->default_settings_button_, SIGNAL(clicked(bool)), this, SLOT(DefaultSettingsButtonClicked()));
    connect(ui_->default_palette_button_, SIGNAL(clicked(bool)), this, SLOT(DefaultPaletteButtonClicked()));
    connect(ui_->spin_box_color_red_, SIGNAL(valueChanged(int)), this, SLOT(RgbValuesChanged()));
    connect(ui_->spin_box_color_green_, SIGNAL(valueChanged(int)), this, SLOT(RgbValuesChanged()));
    connect(ui_->spin_box_color_blue_, SIGNAL(valueChanged(int)), this, SLOT(RgbValuesChanged()));
    connect(ui_->spin_box_color_red_, SIGNAL(valueChanged(int)), ui_->slider_color_red_, SLOT(setValue(int)));
    connect(ui_->spin_box_color_green_, SIGNAL(valueChanged(int)), ui_->slider_color_green_, SLOT(setValue(int)));
    connect(ui_->spin_box_color_blue_, SIGNAL(valueChanged(int)), ui_->slider_color_blue_, SLOT(setValue(int)));
    connect(ui_->slider_color_red_, SIGNAL(valueChanged(int)), ui_->spin_box_color_red_, SLOT(setValue(int)));
    connect(ui_->slider_color_green_, SIGNAL(valueChanged(int)), ui_->spin_box_color_green_, SLOT(setValue(int)));
    connect(ui_->slider_color_blue_, SIGNAL(valueChanged(int)), ui_->spin_box_color_blue_, SLOT(setValue(int)));

    // Set up color picker.
    ui_->color_widget_->SetRowAndColumnCount(kPickerRows, kPickerColumns);
    ui_->color_widget_->SetPalette(rmv::RMVSettings::Get().GetColorPalette());

    // Initial checked item.
    ui_->button_snapshots_viewed_->setChecked(true);

    // Add margins around the color picker label.
    ui_->selected_color_label_->setContentsMargins(10, 5, 10, 5);

    // Initial refresh.
    Refresh();

    // Safety measure to guarantee settings values are in range (should prevent crashing
    // from an invalid settings file).
    for (QAbstractButton* button : button_group_.buttons())
    {
        int button_id  = button_group_.id(button);
        int palette_id = GetSettingsPaletteId(button_id);

        if (palette_id < 0 || palette_id >= (kPickerRows * kPickerColumns))
        {
            SetSettingsPaletteId(button_id, 0);
        }
        else
        {
            // Invalid settings strings which still produce an integer value in range.
            // Should be overwritten with that integer value.
            SetSettingsPaletteId(button_id, palette_id);
        }
    }

    // Set the cursor to pointing hand cursor for the sliders.
    ui_->slider_color_red_->setCursor(Qt::PointingHandCursor);
    ui_->slider_color_green_->setCursor(Qt::PointingHandCursor);
    ui_->slider_color_blue_->setCursor(Qt::PointingHandCursor);
}

ThemesAndColorsPane::~ThemesAndColorsPane()
{
}

void ThemesAndColorsPane::PickerColorSelected(int palette_id, const QColor& color)
{
    Q_UNUSED(color)

    int button_id = button_group_.checkedId();

    // Set palette id in the settings.
    SetSettingsPaletteId(button_id, palette_id);

    Refresh();
}

void ThemesAndColorsPane::ItemButtonClicked(int button_id)
{
    Q_UNUSED(button_id)

    Refresh();
}

void ThemesAndColorsPane::DefaultSettingsButtonClicked()
{
    // Restore default palette ids.
    rmv::RMVSettings::Get().RestoreDefaultColors();

    Refresh();
}

void ThemesAndColorsPane::DefaultPaletteButtonClicked()
{
    // Restore default palette settings.
    rmv::RMVSettings::Get().RestoreDefaultPalette();

    Refresh();
}

void ThemesAndColorsPane::RgbValuesChanged()
{
    ColorPalette palette    = ui_->color_widget_->GetPalette();
    int          palette_id = ui_->color_widget_->GetSelectedPaletteId();

    // Get color from spinbox values.
    QColor color(ui_->spin_box_color_red_->value(), ui_->spin_box_color_green_->value(), ui_->spin_box_color_blue_->value());

    // Set the new color in the palette.
    palette.SetColor(palette_id, color);
    rmv::RMVSettings::Get().SetColorPalette(palette);

    Refresh();
}

void ThemesAndColorsPane::Refresh()
{
    QColor color;

    // Set button color values from corresponding settings.
    for (QAbstractButton* button : button_group_.buttons())
    {
        int button_id = button_group_.id(button);

        if (button->isChecked())
        {
            // Select the picker color that matches this buttons color (default to first color).
            int palette_id = GetSettingsPaletteId(button_id);
            ui_->color_widget_->Select(palette_id);
        }

        // Get colors.
        color = GetSettingsColor(button_id);

        static_cast<ThemesAndColorsItemButton*>(button)->SetColor(color);
    }

    // Set color picker palette.
    ui_->color_widget_->SetPalette(rmv::RMVSettings::Get().GetColorPalette());

    // Set RGB spinbox/slider values.
    color = ui_->color_widget_->GetSelectedColor();
    ui_->spin_box_color_red_->setValue(color.red());
    ui_->spin_box_color_green_->setValue(color.green());
    ui_->spin_box_color_blue_->setValue(color.blue());

    // Set selected color hex label.
    QString color_string      = rmv::string_util::ToUpperCase(QString("#") + QString::number(color.rgb(), 16));
    QString font_color_string = QString("#") + QString::number(rmv_util::GetTextColorForBackground(color_string).rgb(), 16);
    ui_->selected_color_label_->setText("");
    ui_->selected_color_label_->setStyleSheet(QString("background-color:%1;color:%2;").arg(color_string).arg(font_color_string));

    // Indicate the colors may have changed.
    emit RefreshedColors();
}

QColor ThemesAndColorsPane::GetSettingsColor(int button_id) const
{
    return rmv::RMVSettings::Get().GetColorPalette().GetColor(GetSettingsPaletteId(button_id));
}

void ThemesAndColorsPane::SetSettingsPaletteId(int button_id, int palette_id)
{
    switch (button_id)
    {
    case kSettingThemesAndColorsSnapshotViewed:
    case kSettingThemesAndColorsSnapshotCompared:
    case kSettingThemesAndColorsSnapshotLive:
    case kSettingThemesAndColorsSnapshotGenerated:
    case kSettingThemesAndColorsSnapshotVma:
    case kSettingThemesAndColorsResourceDsBuffer:
    case kSettingThemesAndColorsResourceRenderTarget:
    case kSettingThemesAndColorsResourceTexture:
    case kSettingThemesAndColorsResourceVertexBuffer:
    case kSettingThemesAndColorsResourceIndexBuffer:
    case kSettingThemesAndColorsResourceRayTracingBuffer:
    case kSettingThemesAndColorsResourceUav:
    case kSettingThemesAndColorsResourceShaderPipeline:
    case kSettingThemesAndColorsResourceCommandBuffer:
    case kSettingThemesAndColorsResourceHeap:
    case kSettingThemesAndColorsResourceDescriptors:
    case kSettingThemesAndColorsResourceBuffer:
    case kSettingThemesAndColorsResourceGpuEvent:
    case kSettingThemesAndColorsResourceFreeSpace:
    case kSettingThemesAndColorsResourceInternal:
    case kSettingThemesAndColorsDeltaIncrease:
    case kSettingThemesAndColorsDeltaDecrease:
    case kSettingThemesAndColorsDeltaNoChange:
    case kSettingThemesAndColorsHeapLocal:
    case kSettingThemesAndColorsHeapInvisible:
    case kSettingThemesAndColorsHeapSystem:
    case kSettingThemesAndColorsHeapUnspecified:
    case kSettingThemesAndColorsCpuMapped:
    case kSettingThemesAndColorsNotCpuMapped:
    case kSettingThemesAndColorsInPreferredHeap:
    case kSettingThemesAndColorsNotInPreferredHeap:
    case kSettingThemesAndColorsAliased:
    case kSettingThemesAndColorsNotAliased:
    case kSettingThemesAndColorsResourceHistoryResourceEvent:
    case kSettingThemesAndColorsResourceHistoryCpuMapUnmap:
    case kSettingThemesAndColorsResourceHistoryResidencyUpdate:
    case kSettingThemesAndColorsResourceHistoryPageTableUpdate:
    case kSettingThemesAndColorsResourceHistoryHighlight:
    case kSettingThemesAndColorsResourceHistorySnapshot:
    case kSettingThemesAndColorsCommitTypeCommitted:
    case kSettingThemesAndColorsCommitTypePlaced:
    case kSettingThemesAndColorsCommitTypeVirtual:
        rmv::RMVSettings::Get().SetPaletteId(static_cast<RMVSettingID>(button_id), palette_id);
        break;

    default:
        DebugWindow::DbgMsg("Warning: Hit unused default switch case.");
        break;
    }
}

int ThemesAndColorsPane::GetSettingsPaletteId(int button_id) const
{
    switch (button_id)
    {
    case kSettingThemesAndColorsSnapshotViewed:
    case kSettingThemesAndColorsSnapshotCompared:
    case kSettingThemesAndColorsSnapshotLive:
    case kSettingThemesAndColorsSnapshotGenerated:
    case kSettingThemesAndColorsSnapshotVma:

    case kSettingThemesAndColorsResourceDsBuffer:
    case kSettingThemesAndColorsResourceRenderTarget:
    case kSettingThemesAndColorsResourceTexture:
    case kSettingThemesAndColorsResourceVertexBuffer:
    case kSettingThemesAndColorsResourceIndexBuffer:
    case kSettingThemesAndColorsResourceRayTracingBuffer:
    case kSettingThemesAndColorsResourceUav:
    case kSettingThemesAndColorsResourceShaderPipeline:
    case kSettingThemesAndColorsResourceCommandBuffer:
    case kSettingThemesAndColorsResourceHeap:
    case kSettingThemesAndColorsResourceDescriptors:
    case kSettingThemesAndColorsResourceBuffer:
    case kSettingThemesAndColorsResourceGpuEvent:
    case kSettingThemesAndColorsResourceFreeSpace:
    case kSettingThemesAndColorsResourceInternal:

    case kSettingThemesAndColorsDeltaIncrease:
    case kSettingThemesAndColorsDeltaDecrease:
    case kSettingThemesAndColorsDeltaNoChange:

    case kSettingThemesAndColorsHeapLocal:
    case kSettingThemesAndColorsHeapInvisible:
    case kSettingThemesAndColorsHeapSystem:
    case kSettingThemesAndColorsHeapUnspecified:

    case kSettingThemesAndColorsCpuMapped:
    case kSettingThemesAndColorsNotCpuMapped:

    case kSettingThemesAndColorsInPreferredHeap:
    case kSettingThemesAndColorsNotInPreferredHeap:

    case kSettingThemesAndColorsAliased:
    case kSettingThemesAndColorsNotAliased:

    case kSettingThemesAndColorsResourceHistoryResourceEvent:
    case kSettingThemesAndColorsResourceHistoryCpuMapUnmap:
    case kSettingThemesAndColorsResourceHistoryResidencyUpdate:
    case kSettingThemesAndColorsResourceHistoryPageTableUpdate:
    case kSettingThemesAndColorsResourceHistoryHighlight:
    case kSettingThemesAndColorsResourceHistorySnapshot:

    case kSettingThemesAndColorsCommitTypeCommitted:
    case kSettingThemesAndColorsCommitTypePlaced:
    case kSettingThemesAndColorsCommitTypeVirtual:
        return rmv::RMVSettings::Get().GetPaletteId(static_cast<RMVSettingID>(button_id));

    default:
        return -1;
    }
}
