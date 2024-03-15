//=============================================================================
// Copyright (c) 2019-2024 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Implementation of a heap overview memory bar.
//=============================================================================

#include "views/custom_widgets/rmv_heap_overview_memory_bar.h"

#include <QPainter>
#include <QStylePainter>

#include "qt_common/utils/qt_util.h"
#include "qt_common/utils/scaling_manager.h"

#include "util/constants.h"
#include "util/string_util.h"

static const QColor kGrayProcessColor      = QColor(128, 128, 128);
static const QColor kGrayOtherProcessColor = QColor(184, 184, 184);
static const QColor kEmphasizedBarColor    = QColor(192, 192, 192);

RMVHeapOverviewMemoryBar::RMVHeapOverviewMemoryBar(QWidget* parent)
    : QWidget(parent)
    , size_(0)
    , extra_size_(0)
    , max_size_(0)
    , has_subscription_(false)
    , subscription_status_(kRmtSegmentSubscriptionStatusUnderLimit)
    , emphasize_(false)
{
}

RMVHeapOverviewMemoryBar::~RMVHeapOverviewMemoryBar()
{
}

QSize RMVHeapOverviewMemoryBar::sizeHint() const
{
    QSize size_hint = minimumSizeHint();

    // Double the width so that there is room to draw the bar as well.
    size_hint.rwidth() *= 2;

    return size_hint;
}

QSize RMVHeapOverviewMemoryBar::minimumSizeHint() const
{
    QSize minimum_size = ScalingManager::Get().ScaledFontMetrics(font()).size(0, rmv::string_util::LocalizedValueMemory(size_, false, false));

    return minimum_size;
}

void RMVHeapOverviewMemoryBar::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);

    QStylePainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    if (max_size_ > 0)
    {
        QRectF bound_rect = QRectF(0, 0, width(), height());

        int w = bound_rect.width();
        int h = bound_rect.height();

        QColor bar_color = emphasize_ ? kEmphasizedBarColor : kGrayProcessColor;
        if (has_subscription_)
        {
            switch (subscription_status_)
            {
            case kRmtSegmentSubscriptionStatusOverLimit:
                bar_color = rmv::kOverSubscribedColor;
                break;

            case kRmtSegmentSubscriptionStatusUnderLimit:
                bar_color = rmv::kUnderSubscribedColor;
                break;

            case kRmtSegmentSubscriptionStatusCloseToLimit:
                bar_color = rmv::kCloseToSubscribedColor;
                break;

            default:
                break;
            }
        }

        // Calculate length of first rectangle and paint it.
        uint64_t width       = std::max<uint64_t>(1, (size_ * w) / max_size_);
        QRect    memory_rect = QRect(0, 0, width, h);
        painter.fillRect(memory_rect, bar_color);

        // Draw the extra bar for other processes.
        if (extra_size_ > 0)
        {
            // Calculate extra rectangle.
            uint64_t width_2 = (extra_size_ * w) / max_size_;
            painter.fillRect(width, 0, width_2, h, kGrayOtherProcessColor);
        }

        // Draw the amount of memory in this rectangle.
        // If bar is < 50% of the max, display the text string after the bar. Otherwise display it over the bar.
        if (((size_ * 100) / max_size_) < 50)
        {
            painter.setPen(Qt::black);
            int   offset = width + 3;
            QRect text_rect(offset, 0, w - offset, h);
            painter.drawText(text_rect, Qt::AlignLeft | Qt::AlignVCenter, rmv::string_util::LocalizedValueMemory(size_, false, false));
        }
        else
        {
            QColor pen_color = emphasize_ ? Qt::black : Qt::white;
            painter.setPen(pen_color);
            painter.drawText(memory_rect, Qt::AlignCenter | Qt::AlignHCenter, rmv::string_util::LocalizedValueMemory(size_, false, false));
        }
    }
}

void RMVHeapOverviewMemoryBar::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
}

void RMVHeapOverviewMemoryBar::SetParameters(uint64_t                     size,
                                             uint64_t                     extra_size,
                                             uint64_t                     max_size,
                                             bool                         has_subscription,
                                             RmtSegmentSubscriptionStatus subscription_status,
                                             bool                         emphasize)
{
    size_                = size;
    extra_size_          = extra_size;
    max_size_            = max_size;
    has_subscription_    = has_subscription;
    subscription_status_ = subscription_status;
    emphasize_           = emphasize;
}
