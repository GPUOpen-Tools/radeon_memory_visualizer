//=============================================================================
/// Copyright (c) 2019-2020 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Implementation of a number of widget utilities. These functions
///  apply a common look and feel to various widget types.
//=============================================================================

#include <QHeaderView>

#include "qt_common/utils/scaling_manager.h"

#include "util/constants.h"
#include "util/widget_util.h"

namespace rmv
{
    void widget_util::InitDoubleSlider(DoubleSliderWidget* double_slider_widgets)
    {
        double_slider_widgets->setFixedWidth(ScalingManager::Get().Scaled(rmv::kDoubleSliderWidth));
        double_slider_widgets->setFixedHeight(ScalingManager::Get().Scaled(rmv::kDoubleSliderHeight));
        double_slider_widgets->setCursor(Qt::PointingHandCursor);
        double_slider_widgets->setMinimum(0);
        double_slider_widgets->setMaximum(kSizeSliderRange);
        double_slider_widgets->Init();
    }

    void widget_util::InitSingleSelectComboBox(QWidget*           parent,
                                               ArrowIconComboBox* combo_box,
                                               const QString&     default_text,
                                               bool               retain_default_text,
                                               const QString      prefix_text)
    {
        if (combo_box != nullptr)
        {
            combo_box->InitSingleSelect(parent, default_text, retain_default_text, prefix_text);
            combo_box->setCursor(Qt::PointingHandCursor);
        }
    }

    void widget_util::InitMultiSelectComboBox(QWidget* parent, ArrowIconComboBox* combo_box, const QString& default_text)
    {
        if (combo_box != nullptr)
        {
            combo_box->InitMultiSelect(parent, default_text);
            combo_box->setCursor(Qt::PointingHandCursor);
        }
    }

    void widget_util::InitGraphicsView(QGraphicsView* view, uint32_t fixed_height)
    {
        if (view != nullptr)
        {
            view->setFixedHeight(fixed_height);
            view->setFrameStyle(QFrame::NoFrame);
            view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
            view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        }
    }

    void widget_util::SetWidgetBackgroundColor(QWidget* widget, const QColor& color)
    {
        if (widget != nullptr)
        {
            QPalette palette(widget->palette());
            palette.setColor(QPalette::Background, color);
            widget->setPalette(palette);
            widget->setAutoFillBackground(true);
        }
    }

    void widget_util::ApplyStandardPaneStyle(QWidget* root, QWidget* main_content, QScrollArea* scroll_area)
    {
        SetWidgetBackgroundColor(root, Qt::white);
        SetWidgetBackgroundColor(main_content, Qt::white);
        scroll_area->setFrameStyle(QFrame::NoFrame);
        SetWidgetBackgroundColor(scroll_area, Qt::white);
    }

    void widget_util::InitCommonFilteringComponents(TextSearchWidget* text_search_widget, DoubleSliderWidget* double_slider_widget)
    {
        text_search_widget->setFixedWidth(rmv::kSearchBoxWidth);

        double_slider_widget->setFixedWidth(ScalingManager::Get().Scaled(rmv::kDoubleSliderWidth));
        double_slider_widget->setFixedHeight(ScalingManager::Get().Scaled(rmv::kDoubleSliderHeight));
        double_slider_widget->setCursor(Qt::PointingHandCursor);
        double_slider_widget->setMinimum(0);
        double_slider_widget->setMaximum(kSizeSliderRange);
        double_slider_widget->Init();
    }

    void widget_util::InitColorLegend(ColoredLegendScene*& legend_widget, QGraphicsView* view)
    {
        legend_widget = new ColoredLegendScene();
        view->setScene(legend_widget);
    }

    void widget_util::UpdateTablePalette(QTableView* view)
    {
        QPalette p = view->palette();
        p.setColor(QPalette::Inactive, QPalette::Highlight, p.color(QPalette::Active, QPalette::Highlight));
        p.setColor(QPalette::Inactive, QPalette::HighlightedText, p.color(QPalette::Active, QPalette::HighlightedText));
        view->setPalette(p);
    }

    int widget_util::GetTableHeight(const QTableView* table_view, int row_count)
    {
        // Calculate the maximum height the table needs to be.
        // Note the frame width is the gap between the frame and the surrounded
        // widget; there is no frame height since it's the same as the frame width.
        int row_height    = table_view->rowHeight(0);
        int header_height = table_view->horizontalHeader()->height();
        int frame_width   = 2 * table_view->frameWidth();
        return (row_count * row_height) + header_height + frame_width;
    }
}  // namespace rmv
