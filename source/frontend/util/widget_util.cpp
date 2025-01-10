//=============================================================================
// Copyright (c) 2019-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Implementation of a number of widget utilities.
///
/// These functions apply a common look and feel to various widget types.
///
//=============================================================================

#include <QHeaderView>
#include <QScrollBar>

#include "util/constants.h"
#include "util/widget_util.h"

namespace rmv
{
    void widget_util::InitRangeSlider(RmvRangeSlider* slider_widget)
    {
        slider_widget->setFixedWidth(rmv::kDoubleSliderWidth);
        slider_widget->setFixedHeight(rmv::kDoubleSliderHeight);
        slider_widget->setCursor(Qt::PointingHandCursor);
        slider_widget->setMinimum(0);
        slider_widget->setMaximum(kSizeSliderRange - 1);
        slider_widget->Init();
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

    void widget_util::ApplyStandardPaneStyle(QScrollArea* scroll_area)
    {
        scroll_area->setFrameStyle(QFrame::NoFrame);
    }

    void widget_util::InitCommonFilteringComponents(TextSearchWidget* text_search_widget, DoubleSliderWidget* double_slider_widget)
    {
        text_search_widget->setFixedWidth(rmv::kSearchBoxWidth);

        double_slider_widget->setFixedWidth(rmv::kDoubleSliderWidth);
        double_slider_widget->setFixedHeight(rmv::kDoubleSliderHeight);
        double_slider_widget->setCursor(Qt::PointingHandCursor);
        double_slider_widget->setMinimum(0);
        double_slider_widget->setMaximum(kSizeSliderRange);
        double_slider_widget->Init();
    }

    ColoredLegendScene* widget_util::InitColorLegend(QGraphicsView* view)
    {
        ColoredLegendScene* legend_widget = new ColoredLegendScene();
        view->setScene(legend_widget);
        return legend_widget;
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
        int         row_height            = table_view->rowHeight(0);
        int         header_height         = table_view->horizontalHeader()->height();
        int         frame_width           = 2 * table_view->frameWidth();
        int         scroll_bar_height     = 0;
        QScrollBar* horizontal_scroll_bar = table_view->horizontalScrollBar();
        if (horizontal_scroll_bar != nullptr)
        {
            scroll_bar_height = horizontal_scroll_bar->height();
        }
        return (row_count * row_height) + header_height + frame_width + scroll_bar_height;
    }
}  // namespace rmv
