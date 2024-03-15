//=============================================================================
// Copyright (c) 2018-2024 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Header for the resource details widget.
//=============================================================================

#include "views/custom_widgets/rmv_resource_details.h"

#include <QPainter>

#include "qt_common/utils/qt_util.h"
#include "qt_common/utils/scaling_manager.h"

#include "rmt_print.h"
#include "util/rmv_util.h"
#include "util/string_util.h"

RMVResourceDetails::RMVResourceDetails(const RMVResourceDetailsConfig& config)
    : config_(config)
{
}

RMVResourceDetails::~RMVResourceDetails()
{
}

QRectF RMVResourceDetails::boundingRect() const
{
    return QRectF(0, 0, config_.width, config_.height);
}

void RMVResourceDetails::paint(QPainter* painter, const QStyleOptionGraphicsItem* item, QWidget* widget)
{
    Q_UNUSED(item);
    Q_UNUSED(widget);

    if (config_.resource_valid == true)
    {
        const int header_height = ScalingManager::Get().Scaled(20);
        const int value_height  = ScalingManager::Get().Scaled(40);
        const int bar_width     = ScalingManager::Get().Scaled(500);
        const int bar_height    = ScalingManager::Get().Scaled(15);
        const int bar_y_pos     = ScalingManager::Get().Scaled(30);

        const RmtVirtualAllocation* allocation      = config_.resource.bound_allocation;
        const uint64_t              allocation_size = RmtVirtualAllocationGetSizeInBytes(allocation);
        const RmtResourceIdentifier identifier      = config_.resource.identifier;
        const int64_t               size_in_bytes   = config_.resource.size_in_bytes;
        const uint64_t              offset          = RmtResourceGetOffsetFromBoundAllocation(&config_.resource);
        const RmtResourceUsageType  usage_type      = (identifier != 0) ? RmtResourceGetUsageType(&config_.resource) : kRmtResourceUsageTypeFree;

        int x_pos = 0;

        if (config_.allocation_thumbnail == true)
        {
            painter->setPen(Qt::black);

            // It's possible for a resource to not have a parent allocation, so handle it.
            if (allocation != nullptr)
            {
                if (identifier != 0)
                {
                    painter->drawText(
                        0,
                        header_height,
                        "Resource ID " + QString::number(identifier) + " in allocation " + rmv_util::GetVirtualAllocationName(allocation));
                }
                else
                {
                    painter->drawText(0, header_height, "Unbound resource in allocation " + rmv_util::GetVirtualAllocationName(allocation));
                }
            }
            else
            {
                painter->drawText(0, header_height, "Resource ID " + QString::number(identifier) + " has no parent allocation");
            }

            painter->setPen(Qt::NoPen);
            painter->setBrush(QColor(230, 230, 230));
            painter->drawRect(0, bar_y_pos, bar_width, bar_height);

            // Only draw the resource in the parent allocation if it has one.
            if (allocation != nullptr)
            {
                double        pixels_per_byte = static_cast<double>(bar_width) / static_cast<double>(allocation_size);
                const int32_t width           = (int32_t)(size_in_bytes * pixels_per_byte);

                QPen pen;
                pen.setBrush(Qt::black);
                pen.setWidth(ScalingManager::Get().Scaled(1.0));
                painter->setPen(pen);
                painter->setBrush(config_.colorizer->GetColor(allocation, &config_.resource));
                painter->drawRect((offset * pixels_per_byte) + ScalingManager::Get().Scaled(1.0), bar_y_pos, width, bar_height);
            }
            x_pos += ScalingManager::Get().Scaled(550);
        }

        painter->setPen(Qt::black);
        painter->drawText(x_pos, header_height, "Actual size");
        painter->drawText(x_pos, value_height, rmv::string_util::LocalizedValueMemory(size_in_bytes, false, false));

        if (RmtResourceGetAliasCount(&config_.resource) > 0)
        {
            x_pos += ScalingManager::Get().Scaled(100);
            painter->drawText(x_pos, header_height, "Size after aliasing");
            painter->drawText(x_pos, value_height, rmv::string_util::LocalizedValueMemory(config_.resource.adjusted_size_in_bytes, false, false));
        }

        x_pos += ScalingManager::Get().Scaled(100);
        painter->drawText(x_pos, header_height, "Offset");
        painter->drawText(x_pos, value_height, rmv::string_util::LocalizedValueAddress(offset));

        x_pos += ScalingManager::Get().Scaled(250);
        painter->drawText(x_pos, header_height, "Usage");
        painter->drawText(x_pos, value_height, RmtGetResourceUsageTypeNameFromResourceUsageType(usage_type));
    }
    else
    {
        const QString  nothing_selected_string = "Nothing selected";
        const uint32_t string_length           = QtCommon::QtUtils::GetPainterTextWidth(painter, nothing_selected_string);
        const uint32_t x_pos                   = config_.width / 2 - string_length / 2;

        QFont font = painter->font();
        font.setPixelSize(ScalingManager::Get().Scaled(18));

        painter->setFont(font);
        painter->setPen(Qt::gray);
        painter->drawText(x_pos, ScalingManager::Get().Scaled(30), nothing_selected_string);
    }
}

void RMVResourceDetails::UpdateResource(const RmtResource* resource)
{
    if (resource != nullptr)
    {
        // Make a copy of the resource since in the case of unbounded resources
        // the pointers will change when the heap overview size is changed.
        memcpy(&config_.resource, resource, sizeof(RmtResource));
        config_.resource_valid = true;
    }
    else
    {
        config_.resource_valid = false;
    }
    update();
}

const RmtResource* RMVResourceDetails::GetResource() const
{
    return &config_.resource;
}

void RMVResourceDetails::UpdateDimensions(int width, int height)
{
    config_.width  = width - 2;
    config_.height = height - 2;
}
