//=============================================================================
// Copyright (c) 2018-2024 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Header for a tree map block collection.
//=============================================================================

#include "views/custom_widgets/rmv_tree_map_blocks.h"

#ifndef _WIN32
#include "linux/safe_crt.h"  // for strcpy_s
#endif

#include <QPainter>
#include <QGraphicsSceneHoverEvent>
#include <QDebug>
#include <QVector>
#include <QMap>
#include <math.h>

#include "rmt_assert.h"
#include "rmt_data_snapshot.h"

#include "managers/message_manager.h"
#include "managers/trace_manager.h"
#include "models/snapshot/resource_overview_model.h"
#include "rmt_types.h"
#include "settings/rmv_settings.h"

// The mimimum area that a resource can use. Anything smaller than this is ignored.
const static int kMinArea = 4;

// The number of values a boolean can have (true or false).
const static int kBooleanCount = 2;

#ifdef _DEBUG
// Text for an unbound resource
const static char* kUnboundResourceName = "unbound";
#endif  // #ifdef _DEBUG

/// Sorting function.
///
/// @param [in] a1 First RmvMemoryBlockAllocation.
/// @param [in] a2 Second RmvMemoryBlockAllocation.
///
/// @return true if a1 > a2, false otherwise.
static bool SortResourcesBySizeFunc(const RmtResource* a1, const RmtResource* a2)
{
    return a1->adjusted_size_in_bytes > a2->adjusted_size_in_bytes;
}

RMVTreeMapBlocks::RMVTreeMapBlocks(const RMVTreeMapBlocksConfig& config)
    : config_(config)
    , hovered_resource_identifier_(0)
    , selected_resource_identifier_(0)
    , hovered_resource_(nullptr)
    , selected_resource_(nullptr)
    , colorizer_(nullptr)
{
    setAcceptHoverEvents(true);
}

RMVTreeMapBlocks::~RMVTreeMapBlocks()
{
    for (int32_t i = 0; i < unbound_resources_.size(); i++)
    {
        delete unbound_resources_[i];
    }
}

void RMVTreeMapBlocks::SetColorizer(const rmv::Colorizer* colorizer)
{
    colorizer_ = colorizer;
}

QRectF RMVTreeMapBlocks::boundingRect() const
{
    return QRectF(0, 0, config_.width, config_.height);
}

void RMVTreeMapBlocks::PaintClusterParents(QPainter* painter, const ResourceCluster& cluster)
{
    // This paints the borders around slicing modes.
    QPen pen;
    pen.setWidth(2);
    pen.setColor(Qt::black);
    painter->setPen(pen);
    painter->setBrush(Qt::NoBrush);
    painter->drawRect(cluster.geometry);

    // Go to next parent.
    if (cluster.sub_clusters.size() != 0)
    {
        QMapIterator<uint32_t, ResourceCluster> next_it(cluster.sub_clusters);
        while (next_it.hasNext())
        {
            next_it.next();

            const ResourceCluster& sub_cluster = next_it.value();

            PaintClusterParents(painter, sub_cluster);
        }
    }
}

void RMVTreeMapBlocks::PaintClusterChildren(QPainter*              painter,
                                            const ResourceCluster& cluster,
                                            TreeMapBlockData&      hovered_resource,
                                            TreeMapBlockData&      selected_resource)
{
    // This paints blocks inside each cluster.
    if (cluster.sub_clusters.size() == 0)
    {
        QMapIterator<const RmtResource*, QRectF> render_it(cluster.alloc_geometry_map);
        while (render_it.hasNext())
        {
            render_it.next();

            const RmtResource* resource      = render_it.key();
            const QRectF&      bounding_rect = render_it.value();

            const QRectF block_rect(bounding_rect.left() + 1, bounding_rect.top() + 1, bounding_rect.width() - 1, bounding_rect.height() - 1);

            if (block_rect.width() > 0 && block_rect.height() > 0)
            {
                const QColor& curr_color = colorizer_->GetColor(resource->bound_allocation, resource);

                // Figure out the brush style.
                Qt::BrushStyle style = (RmtResourceIsAliased(resource) ? Qt::BrushStyle::Dense1Pattern : Qt::BrushStyle::SolidPattern);
                const QBrush   curr_brush(curr_color, style);

                painter->fillRect(block_rect, curr_brush);

                // Figure out what we hovered over.
                if (hovered_resource.is_visible == false)
                {
                    if (hovered_resource_identifier_ == resource->identifier && hovered_resource_ == resource)
                    {
                        hovered_resource.bounding_rect = block_rect;
                        hovered_resource.resource      = resource;
                        hovered_resource.is_visible    = true;
                    }
                }

                // Figure out what we selected.
                if (selected_resource.is_visible == false)
                {
                    if (selected_resource_identifier_ == resource->identifier && selected_resource_ == resource)
                    {
                        selected_resource.bounding_rect = block_rect;
                        selected_resource.resource      = resource;
                        selected_resource.is_visible    = true;
                    }
                }
            }
        }
    }

    // Move onto next set of subslices.
    else
    {
        QMapIterator<uint32_t, ResourceCluster> next_it(cluster.sub_clusters);
        while (next_it.hasNext())
        {
            next_it.next();

            const ResourceCluster& sub_cluster = next_it.value();

            PaintClusterChildren(painter, sub_cluster, hovered_resource, selected_resource);
        }
    }
}

void RMVTreeMapBlocks::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    TreeMapBlockData hovered_block  = {};
    TreeMapBlockData selected_block = {};

    PaintClusterChildren(painter, clusters_[kSliceTypeNone], hovered_block, selected_block);

    PaintClusterParents(painter, clusters_[kSliceTypeNone]);

    const Qt::BrushStyle hover_style = (RmtResourceIsAliased(hovered_block.resource) ? Qt::BrushStyle::Dense1Pattern : Qt::BrushStyle::SolidPattern);
    if (hovered_block.resource && selected_block.resource && hovered_block.resource->identifier == selected_block.resource->identifier &&
        hovered_block.resource->bound_allocation == selected_block.resource->bound_allocation)
    {
        // Decide what to do if a resource is clicked on.
        if ((hovered_block.is_visible == true) && (selected_block.is_visible == true))
        {
            QPen pen;
            pen.setBrush(Qt::black);
            pen.setWidth(2);
            painter->setPen(pen);
            QBrush brush(colorizer_->GetColor(selected_block.resource->bound_allocation, selected_block.resource).darker(rmv::kHoverDarkenColor), hover_style);
            painter->setBrush(brush);
            painter->drawRect(selected_block.bounding_rect);
        }
    }
    else
    {
        if (hovered_block.is_visible == true)
        {
            if (hovered_block.resource->identifier != 0 || hovered_block.resource == hovered_resource_)
            {
                painter->setPen(Qt::NoPen);
                QBrush brush(colorizer_->GetColor(hovered_block.resource->bound_allocation, hovered_block.resource).darker(rmv::kHoverDarkenColor),
                             hover_style);
                painter->setBrush(brush);
                painter->drawRect(hovered_block.bounding_rect);
            }
        }

        if (selected_block.is_visible == true)
        {
            QPen pen;
            pen.setBrush(Qt::black);
            pen.setWidth(2);
            painter->setPen(pen);
            QBrush brush(colorizer_->GetColor(selected_block.resource->bound_allocation, selected_block.resource), hover_style);
            painter->setBrush(brush);
            painter->drawRect(selected_block.bounding_rect);
        }
    }
}

bool RMVTreeMapBlocks::FindBlockData(ResourceCluster&       cluster,
                                     QPointF                user_location,
                                     RmtResourceIdentifier& resource_identifier,
                                     const RmtResource*&    resource) const
{
    resource_identifier = 0;

    bool result = false;

    // Move onto next levels.
    QMutableMapIterator<uint32_t, ResourceCluster> next_it(cluster.sub_clusters);
    while (next_it.hasNext())
    {
        next_it.next();

        ResourceCluster& sub_cluster = next_it.value();

        result = FindBlockData(sub_cluster, user_location, resource_identifier, resource);

        if (result == true)
        {
            return true;
        }
    }

    // Only perform search at bottom-most level.
    if (cluster.sub_clusters.size() == 0)
    {
        QMutableMapIterator<const RmtResource*, QRectF> search_it(cluster.alloc_geometry_map);
        while (search_it.hasNext())
        {
            search_it.next();

            const RmtResource* res           = search_it.key();
            const QRectF&      bounding_rect = search_it.value();

            if (user_location.x() > bounding_rect.left() && user_location.x() < bounding_rect.right())
            {
                if (user_location.y() > bounding_rect.top() && user_location.y() < bounding_rect.bottom())
                {
                    resource_identifier = res->identifier;
                    resource            = res;

                    result = true;
                    break;
                }
            }
        }
    }

    return result;
}

void RMVTreeMapBlocks::hoverMoveEvent(QGraphicsSceneHoverEvent* event)
{
    setCursor(Qt::PointingHandCursor);

    const bool found_resource = FindBlockData(clusters_[kSliceTypeNone], event->pos(), hovered_resource_identifier_, hovered_resource_);
    update();

    if (!found_resource)
    {
        hovered_resource_identifier_ = 0;
        hovered_resource_            = nullptr;
    }
}

void RMVTreeMapBlocks::hoverLeaveEvent(QGraphicsSceneHoverEvent* event)
{
    Q_UNUSED(event);

    hovered_resource_identifier_ = 0;
    hovered_resource_            = nullptr;

    update();
}

void RMVTreeMapBlocks::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    const bool found_resource = FindBlockData(clusters_[kSliceTypeNone], event->pos(), selected_resource_identifier_, selected_resource_);
    update();

    if (!found_resource)
    {
        return;
    }

    bool broadcast = true;

    if (selected_resource_identifier_ != 0)
    {
        emit ResourceSelected(selected_resource_identifier_, broadcast, false);
    }
    else if (selected_resource_ != nullptr)
    {
        emit UnboundResourceSelected(selected_resource_, broadcast, false);
    }
}

void RMVTreeMapBlocks::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event)
{
    Q_UNUSED(event);

    if (selected_resource_identifier_ != 0)
    {
        emit ResourceSelected(selected_resource_identifier_, true, true);
    }
    else if (selected_resource_ != nullptr)
    {
        emit UnboundResourceSelected(selected_resource_, true, true);
    }
}

void RMVTreeMapBlocks::UpdateDimensions(int width, int height)
{
    config_.width  = width;
    config_.height = height;
}

int32_t RMVTreeMapBlocks::ScaledHeight() const
{
    return config_.height;
}

int32_t RMVTreeMapBlocks::ScaledWidth() const
{
    return config_.width;
}

void RMVTreeMapBlocks::ResetSelections()
{
    hovered_resource_identifier_  = 0;
    selected_resource_identifier_ = 0;
    hovered_resource_             = nullptr;
    selected_resource_            = nullptr;
}

void RMVTreeMapBlocks::Reset()
{
    clusters_.clear();
}

double RMVTreeMapBlocks::CalculateAspectRatio(double width, double height) const
{
    return RMT_MAXIMUM(width / height, height / width);
}

bool RMVTreeMapBlocks::ShouldDrawVertically(double width, double height) const
{
    return (width > height);
}

void RMVTreeMapBlocks::DumpCut(const CutData& existing_cut, uint32_t offset_x, uint32_t offset_y, AllocGeometryMap& alloc_geometry_map) const
{
    // Dump all the rectangles from the cut into the allocation rectangles.
    if (!existing_cut.is_null)
    {
        for (int32_t current_allocation_index = 0; current_allocation_index < existing_cut.resources.size(); ++current_allocation_index)
        {
            bool visible_rects = true;

            for (int i = 0; i < existing_cut.rectangles.size(); i++)
            {
                if (existing_cut.rectangles[i].width() < 1 || existing_cut.rectangles[i].height() < 1)
                {
                    visible_rects = false;
                    break;
                }
            }

            if (visible_rects == true)
            {
                for (int i = 0; i < existing_cut.rectangles.size(); i++)
                {
                    QRectF newBound = existing_cut.rectangles[i];
                    newBound.translate(offset_x, offset_y);

                    alloc_geometry_map[existing_cut.resources[i]] = newBound;
                }
            }
        }
    }
}

void RMVTreeMapBlocks::SelectResource(RmtResourceIdentifier resource_identifier)
{
    selected_resource_identifier_ = resource_identifier;
    selected_resource_            = nullptr;

    update();
}

const RmtResource* RMVTreeMapBlocks::GetHoveredResource() const
{
    return hovered_resource_;
}

void RMVTreeMapBlocks::GenerateTreeMapRects(QVector<const RmtResource*>& resources,
                                            uint64_t                     total_size,
                                            uint32_t                     view_width,
                                            uint32_t                     view_height,
                                            uint32_t                     offset_x,
                                            uint32_t                     offset_y,
                                            AllocGeometryMap&            alloc_geometry_map)
{
    if (resources.isEmpty() == true)
    {
        return;
    }

    // Work out how the bytes map to pixels.
    QRectF draw_space(0, 0, view_width, view_height);
    double bytes_per_pixel = (double)total_size / (double)(draw_space.width() * draw_space.height());

    CutData existing_cut = {};
    existing_cut.is_null = true;

    for (int32_t i = 0; i < resources.size(); i++)
    {
        const RmtResource* resource = resources[i];

        if (existing_cut.is_null == false)
        {
            int64_t attempted_cut_size = existing_cut.size_in_bytes + resource->adjusted_size_in_bytes;
            double  total_cut_area     = attempted_cut_size / bytes_per_pixel;
            double  total_cut_width    = total_cut_area / existing_cut.bounding_rect.height();
            double  total_cut_height   = total_cut_area / existing_cut.bounding_rect.width();

            if (existing_cut.is_vertical)
            {
                // Does this improve aspect ratio of most significant rectangle?
                double primary_cut_area       = existing_cut.resources[0]->adjusted_size_in_bytes / bytes_per_pixel;
                double primary_cut_new_height = primary_cut_area / total_cut_width;
                double existing_aspect_ratio  = CalculateAspectRatio(existing_cut.rectangles[0].width(), existing_cut.rectangles[0].height());
                double new_aspect_ratio       = CalculateAspectRatio(total_cut_width, primary_cut_new_height);
                double existing_error         = fabs(existing_aspect_ratio - 1.0);
                double new_error              = fabs(new_aspect_ratio - 1.0);

                // Check if we should accept the cut. This is only true if the most
                // significant rectangle is now closer to the perfect aspect ratio of 1.
                if (new_error < existing_error)
                {
                    float currentX = existing_cut.bounding_rect.x();
                    float currentY = existing_cut.bounding_rect.y();

                    // To accept the cut we have to re-evaluate all rectangles in the existing cut to account for new widths.
                    for (int32_t currentAllocationIndex = 0; currentAllocationIndex < existing_cut.resources.size(); ++currentAllocationIndex)
                    {
                        double allocateArea        = existing_cut.resources[currentAllocationIndex]->adjusted_size_in_bytes / bytes_per_pixel;
                        double newAllocationWidth  = total_cut_width;
                        double newAllocationHeight = allocateArea / total_cut_width;

                        QRectF newAllocationRectangle(currentX, currentY, (float)newAllocationWidth, (float)newAllocationHeight);

                        // Update the rectangle.
                        existing_cut.rectangles[currentAllocationIndex] = newAllocationRectangle;

                        // Update current X and Y.
                        currentX = newAllocationRectangle.x();
                        currentY = newAllocationRectangle.bottom();
                    }

                    // Add the new cut will always occupy bottom of the cut.
                    QRectF allocationRectangle(currentX, currentY, total_cut_width, (static_cast<qreal>(view_height) - currentY));
                    existing_cut.rectangles.push_back(allocationRectangle);
                    existing_cut.resources.push_back(resource);

                    // Update the exist cut's bounding area.
                    existing_cut.bounding_rect =
                        QRectF(existing_cut.bounding_rect.x(), existing_cut.bounding_rect.y(), (float)total_cut_width, existing_cut.bounding_rect.height());

                    existing_cut.size_in_bytes = attempted_cut_size;

                    // Update draw space.
                    draw_space.setX(existing_cut.bounding_rect.right());

                    // Move to next allocation.
                    continue;
                }
            }
            else
            {
                // Does this improve aspect ratio of most significant rectangle?
                double primary_cut_area      = existing_cut.resources[0]->adjusted_size_in_bytes / bytes_per_pixel;
                double primary_cut_new_width = primary_cut_area / total_cut_height;
                double existing_aspect_ratio = CalculateAspectRatio(existing_cut.rectangles[0].width(), existing_cut.rectangles[0].height());
                double new_aspect_ratio      = CalculateAspectRatio(primary_cut_new_width, total_cut_height);
                double existing_error        = fabs(existing_aspect_ratio - 1.0);
                double new_error             = fabs(new_aspect_ratio - 1.0);

                // Check if we should accept the cut. This is only true if the most
                // significant rectangle is now closer to the perfect aspect ratio of 1.
                if (new_error < existing_error)
                {
                    float currentX = existing_cut.bounding_rect.x();
                    float currentY = existing_cut.bounding_rect.y();

                    // To accept the cut we have to re-evaluate all rectangles in the existing cut to account for new widths.
                    for (int32_t currentAllocationIndex = 0; currentAllocationIndex < existing_cut.resources.size(); ++currentAllocationIndex)
                    {
                        double allocateArea        = existing_cut.resources[currentAllocationIndex]->adjusted_size_in_bytes / bytes_per_pixel;
                        double newAllocationWidth  = allocateArea / total_cut_height;
                        double newAllocationHeight = total_cut_height;
                        QRectF newAllocationRectangle(currentX, currentY, (int32_t)newAllocationWidth, (int32_t)newAllocationHeight);

                        // Update the rectangle.
                        existing_cut.rectangles[currentAllocationIndex] = newAllocationRectangle;

                        // Update current X and Y.
                        currentX = newAllocationRectangle.right();
                        currentY = newAllocationRectangle.y();
                    }

                    // Add the new cut will always occupy bottom of the cut.
                    QRectF allocationRectangle(currentX, currentY, (static_cast<qreal>(view_width) - currentX), total_cut_height);
                    existing_cut.rectangles.push_back(allocationRectangle);
                    existing_cut.resources.push_back(resource);

                    // Update the exist cut's bounding area.
                    existing_cut.bounding_rect =
                        QRectF(existing_cut.bounding_rect.x(), existing_cut.bounding_rect.y(), existing_cut.bounding_rect.width(), (float)total_cut_height);
                    existing_cut.size_in_bytes = attempted_cut_size;

                    // Update draw space.
                    draw_space.setY(existing_cut.bounding_rect.bottom());

                    // Move to next allocation.
                    continue;
                }
            }
        }

        DumpCut(existing_cut, offset_x, offset_y, alloc_geometry_map);
        existing_cut.is_null = true;

        // Fall back to making a new cut.
        if (ShouldDrawVertically(draw_space.width(), draw_space.height()))
        {
            double area  = (double)resource->adjusted_size_in_bytes / bytes_per_pixel;
            double width = area / draw_space.height();
            QRectF allocation_rectangle(draw_space.x(), draw_space.y(), (int32_t)width, draw_space.height());

            // Create a new cut.
            existing_cut               = {};
            existing_cut.is_null       = false;
            existing_cut.bounding_rect = allocation_rectangle;
            existing_cut.is_vertical   = true;
            existing_cut.size_in_bytes = resource->adjusted_size_in_bytes;
            existing_cut.rectangles.push_back(allocation_rectangle);
            existing_cut.resources.push_back(resource);

            // Update draw space to remove what we just used.
            draw_space.setX(allocation_rectangle.right());
        }
        else
        {
            double area   = (double)resource->adjusted_size_in_bytes / bytes_per_pixel;
            double height = area / draw_space.width();
            QRectF allocation_rectangle(draw_space.x(), draw_space.y(), draw_space.width(), (int32_t)height);

            // Create a new cut.
            existing_cut               = {};
            existing_cut.is_null       = false;
            existing_cut.bounding_rect = allocation_rectangle;
            existing_cut.is_vertical   = false;
            existing_cut.size_in_bytes = resource->adjusted_size_in_bytes;
            existing_cut.rectangles.push_back(allocation_rectangle);
            existing_cut.resources.push_back(resource);

            // Update draw space to remove what we just used.
            draw_space.setY(allocation_rectangle.bottom());
        }
    }

    DumpCut(existing_cut, offset_x, offset_y, alloc_geometry_map);
    existing_cut.is_null = true;
}

void RMVTreeMapBlocks::GenerateTreemap(const rmv::ResourceOverviewModel* overview_model,
                                       const TreeMapModels&              tree_map_models,
                                       uint32_t                          view_width,
                                       uint32_t                          view_height)
{
    RmtDataSnapshot* open_snapshot = rmv::SnapshotManager::Get().GetOpenSnapshot();

    if (rmv::TraceManager::Get().DataSetValid() && open_snapshot != nullptr)
    {
        clusters_.clear();
        for (int32_t i = 0; i < unbound_resources_.size(); i++)
        {
            delete unbound_resources_[i];
        }
        unbound_resources_.clear();

        ResourceCluster& parent_cluster   = clusters_[kSliceTypeNone];
        const int32_t    allocation_count = open_snapshot->virtual_allocation_list.allocation_count;

        // Calculate how much memory is to be displayed.
        uint64_t total_memory = 0;
        for (int32_t i = 0; i < allocation_count; i++)
        {
            RmtVirtualAllocation* current_virtual_allocation = &open_snapshot->virtual_allocation_list.allocation_details[i];

            if (tree_map_models.preferred_heap_model->ItemInList(current_virtual_allocation->heap_preferences[0]) == false)
            {
                continue;
            }

            for (int32_t j = 0; j < current_virtual_allocation->resource_count; j++)
            {
                RmtResource* resource = current_virtual_allocation->resources[j];
                if (ResourceFiltered(overview_model, tree_map_models, open_snapshot, resource) == true)
                {
                    total_memory += resource->adjusted_size_in_bytes;
                }
            }

            if (tree_map_models.resource_usage_model->ItemInList(kRmtResourceUsageTypeFree) == false)
            {
                continue;
            }

            for (int32_t current_unbound_region_index = 0; current_unbound_region_index < current_virtual_allocation->unbound_memory_region_count;
                 ++current_unbound_region_index)
            {
                // Add any unbound memory.
                const RmtMemoryRegion* current_unbound_region = &current_virtual_allocation->unbound_memory_regions[current_unbound_region_index];
                if (current_unbound_region->size == 0)
                {
                    continue;
                }
                if (overview_model->IsSizeInSliderRange(current_unbound_region->size) == false)
                {
                    continue;
                }
                total_memory += current_unbound_region->size;
            }
        }

        const double bytes_per_pixel = static_cast<double>(total_memory) / (static_cast<double>(view_width) * view_height);
        for (int32_t i = 0; i < allocation_count; i++)
        {
            RmtVirtualAllocation* current_virtual_allocation = &open_snapshot->virtual_allocation_list.allocation_details[i];

            if (tree_map_models.preferred_heap_model->ItemInList(current_virtual_allocation->heap_preferences[0]) == false)
            {
                continue;
            }

            for (int32_t j = 0; j < current_virtual_allocation->resource_count; j++)
            {
                RmtResource* resource = current_virtual_allocation->resources[j];
                if (ResourceFiltered(overview_model, tree_map_models, open_snapshot, resource) == true)
                {
                    const double total_cut_area = resource->adjusted_size_in_bytes / bytes_per_pixel;

                    // Only include allocations that could actually be visible.
                    if (total_cut_area >= kMinArea)
                    {
                        parent_cluster.amount += resource->adjusted_size_in_bytes;
                        parent_cluster.sorted_resources.push_back(resource);
                    }
                }
            }

            if (tree_map_models.resource_usage_model->ItemInList(kRmtResourceUsageTypeFree) == false)
            {
                continue;
            }

            for (int32_t current_unbound_region_index = 0; current_unbound_region_index < current_virtual_allocation->unbound_memory_region_count;
                 ++current_unbound_region_index)
            {
                // Add any unbound memory.
                const RmtMemoryRegion* current_unbound_region = &current_virtual_allocation->unbound_memory_regions[current_unbound_region_index];
                if (current_unbound_region->size == 0)
                {
                    continue;
                }
                if (overview_model->IsSizeInSliderRange(current_unbound_region->size) == false)
                {
                    continue;
                }
                const double total_cut_area = current_unbound_region->size / bytes_per_pixel;
                if (total_cut_area >= kMinArea)
                {
                    // Create a temporary 'unbound' resource so the unbound resource can be sorted along with other resources
                    // in the tree map. All unbound resources have a resource identifier of 0. Their base address and size are
                    // copied from their unbound region data.
                    RmtResource* unbound_resource            = new RmtResource();
                    unbound_resource->identifier             = 0;
                    unbound_resource->adjusted_size_in_bytes = current_unbound_region->size;
                    unbound_resource->size_in_bytes          = current_unbound_region->size;
                    unbound_resource->address                = current_virtual_allocation->base_address + current_unbound_region->offset;
                    unbound_resource->bound_allocation       = current_virtual_allocation;
                    unbound_resource->resource_type          = kRmtResourceTypeCount;
#ifdef _DEBUG
                    unbound_resource->name = kUnboundResourceName;
#endif
                    // Keep track of the unbound resource.
                    unbound_resources_.push_back(unbound_resource);

                    parent_cluster.amount += current_unbound_region->size;
                    parent_cluster.sorted_resources.push_back(unbound_resource);
                }
            }
        }

        std::stable_sort(parent_cluster.sorted_resources.begin(), parent_cluster.sorted_resources.end(), SortResourcesBySizeFunc);

        // Something actually selected in the UI.
        if (slice_types_.empty() == false)
        {
            FillClusterResources(parent_cluster, slice_types_, 0, slice_types_.size());
        }

        // Nothing selected, so just show all allocations without slicing.
        else
        {
            for (int i = 0; i < parent_cluster.sorted_resources.size(); i++)
            {
                const RmtResource* resource = parent_cluster.sorted_resources[i];

                parent_cluster.sub_clusters[kSliceTypeNone].amount += resource->adjusted_size_in_bytes;
                parent_cluster.sub_clusters[kSliceTypeNone].sorted_resources.push_back(resource);
            }
        }
        FillClusterGeometry(clusters_[kSliceTypeNone], view_width, view_height, 0, 0);
    }
}

bool RMVTreeMapBlocks::ResourceFiltered(const rmv::ResourceOverviewModel* overview_model,
                                        const TreeMapModels&              tree_map_models,
                                        const RmtDataSnapshot*            snapshot,
                                        const RmtResource*                resource)
{
    if (tree_map_models.actual_heap_model->ItemInList(RmtResourceGetActualHeap(snapshot, resource)) == false)
    {
        return false;
    }
    if (tree_map_models.resource_usage_model->ItemInList(RmtResourceGetUsageType(resource)) == false)
    {
        return false;
    }
    if (overview_model->IsSizeInSliderRange(resource->adjusted_size_in_bytes) == false)
    {
        return false;
    }
    return true;
}

void RMVTreeMapBlocks::FillClusterGeometry(ResourceCluster& parent_cluster,
                                           uint32_t         parent_width,
                                           uint32_t         parent_height,
                                           int32_t          parent_offset_x,
                                           int32_t          parent_offset_y)
{
    parent_cluster.geometry = QRectF(parent_offset_x, parent_offset_y, parent_width, parent_height);

    if (parent_cluster.sub_clusters.size() == 0)
    {
        return;
    }

    // Helper map to associate a slice key to a resource.
    QMap<uint32_t, const RmtResource*> sliceIdToAlloc;  // probably can just be an array of vectors.

    // Create some temporary allocations so we can later compute geometry for parent bounds.
    uint64_t                                temp_parent_allocs_size = 0;
    QVector<const RmtResource*>             temp_resources;
    QMapIterator<uint32_t, ResourceCluster> it_1(parent_cluster.sub_clusters);
    while (it_1.hasNext())
    {
        it_1.next();

        uint32_t        sliceType  = it_1.key();
        ResourceCluster subCluster = it_1.value();

        RmtResource* temp_resource   = new RmtResource();
        temp_resource->adjusted_size_in_bytes = subCluster.amount;
        temp_resources.push_back(temp_resource);
        temp_parent_allocs_size += subCluster.amount;

        // Fill out helper map.
        sliceIdToAlloc[sliceType] = temp_resource;
    }

    std::stable_sort(temp_resources.begin(), temp_resources.end(), SortResourcesBySizeFunc);

    // Figure out geometry for parent bounds.
    GenerateTreeMapRects(
        temp_resources, temp_parent_allocs_size, parent_width, parent_height, parent_offset_x, parent_offset_y, parent_cluster.alloc_geometry_map);

    // Clean up temp RmvMemoryBlockAllocations.
    for (int32_t i = 0; i < temp_resources.size(); i++)
    {
        delete temp_resources[i];
    }

    // Figure out geometry for sub-clusters, but bound by parent bounds.
    QMutableMapIterator<uint32_t, ResourceCluster> it_2(parent_cluster.sub_clusters);
    while (it_2.hasNext())
    {
        it_2.next();

        uint32_t         sliceType  = it_2.key();
        ResourceCluster& subCluster = it_2.value();

        QRectF& bounding_rect = parent_cluster.alloc_geometry_map[sliceIdToAlloc[sliceType]];

        // Figure out child geometry.
        GenerateTreeMapRects(subCluster.sorted_resources,
                             subCluster.amount,
                             bounding_rect.width(),
                             bounding_rect.height(),
                             bounding_rect.left(),
                             bounding_rect.top(),
                             subCluster.alloc_geometry_map);

        // Next child.
        FillClusterGeometry(subCluster, bounding_rect.width(), bounding_rect.height(), bounding_rect.left(), bounding_rect.top());
    }
}

int32_t RMVTreeMapBlocks::GetSliceCount(const SliceType slice_type, const RmtDataSnapshot* snapshot) const
{
    switch (slice_type)
    {
    case kSliceTypeResourceUsageType:
        return kRmtResourceUsageTypeCount;

    case kSliceTypeResourceCreateAge:
    case kSliceTypeResourceBindAge:
    case kSliceTypeAllocationAge:
        return colorizer_->GetNumAgeBuckets();

    case kSliceTypePreferredHeap:
    case kSliceTypeActualHeap:
        return kRmtHeapTypeCount;

    case kSliceTypeVirtualAllocation:
        return snapshot->virtual_allocation_list.allocation_count;

    case kSliceTypeCpuMapped:
    case kSliceTypeInPreferredHeap:
        return kBooleanCount;

    case kSliceTypeResourceCommitType:
        return kRmtCommitTypeCount;

    case kSliceTypeResourceOwner:
        return kRmtOwnerTypeCount;

    default:
        RMT_ASSERT(false);
        break;
    }
    return 0;
}

void RMVTreeMapBlocks::AddClusterResource(ResourceCluster& parent_cluster, const int32_t slice_index, const RmtResource* resource, bool& resource_added) const
{
    parent_cluster.sub_clusters[slice_index].amount += resource->adjusted_size_in_bytes;
    parent_cluster.sub_clusters[slice_index].sorted_resources.push_back(resource);
    resource_added = true;
}

void RMVTreeMapBlocks::FilterResourceUsageType(ResourceCluster&       parent_cluster,
                                               int32_t                slice_index,
                                               const RmtDataSnapshot* snapshot,
                                               const RmtResource*     resource,
                                               bool&                  resource_added) const
{
    RMT_ASSERT(snapshot);
    if (resource != nullptr && RmtResourceGetUsageType(resource) == slice_index)
    {
        AddClusterResource(parent_cluster, slice_index, resource, resource_added);
    }
}

void RMVTreeMapBlocks::FilterResourceCreateAge(ResourceCluster&       parent_cluster,
                                               int32_t                slice_index,
                                               const RmtDataSnapshot* snapshot,
                                               const RmtResource*     resource,
                                               bool&                  resource_added) const
{
    RMT_ASSERT(snapshot);
    if (resource != nullptr && colorizer_->GetAgeIndex(resource->create_time) == slice_index)
    {
        AddClusterResource(parent_cluster, slice_index, resource, resource_added);
    }
}

void RMVTreeMapBlocks::FilterResourceBindAge(ResourceCluster&       parent_cluster,
                                             int32_t                slice_index,
                                             const RmtDataSnapshot* snapshot,
                                             const RmtResource*     resource,
                                             bool&                  resource_added) const
{
    RMT_ASSERT(snapshot);
    if (resource != nullptr && colorizer_->GetAgeIndex(resource->bind_time) == slice_index)
    {
        AddClusterResource(parent_cluster, slice_index, resource, resource_added);
    }
}

void RMVTreeMapBlocks::FilterAllocationAge(ResourceCluster&       parent_cluster,
                                           int32_t                slice_index,
                                           const RmtDataSnapshot* snapshot,
                                           const RmtResource*     resource,
                                           bool&                  resource_added) const
{
    RMT_ASSERT(snapshot);
    if (resource != nullptr && resource->bound_allocation != nullptr && colorizer_->GetAgeIndex(resource->bound_allocation->timestamp) == slice_index)
    {
        AddClusterResource(parent_cluster, slice_index, resource, resource_added);
    }
}

void RMVTreeMapBlocks::FilterPreferredHeap(ResourceCluster&       parent_cluster,
                                           int32_t                slice_index,
                                           const RmtDataSnapshot* snapshot,
                                           const RmtResource*     resource,
                                           bool&                  resource_added) const
{
    RMT_ASSERT(snapshot);
    if (resource != nullptr && resource->bound_allocation->heap_preferences[0] == slice_index)
    {
        AddClusterResource(parent_cluster, slice_index, resource, resource_added);
    }
}

void RMVTreeMapBlocks::FilterCpuMapped(ResourceCluster&       parent_cluster,
                                       int32_t                slice_index,
                                       const RmtDataSnapshot* snapshot,
                                       const RmtResource*     resource,
                                       bool&                  resource_added) const
{
    RMT_ASSERT(snapshot);
    if (((resource->bound_allocation->flags & kRmtAllocationDetailIsCpuMapped) == kRmtAllocationDetailIsCpuMapped) && slice_index == 1)
    {
        AddClusterResource(parent_cluster, slice_index, resource, resource_added);
    }
    else if (((resource->bound_allocation->flags & kRmtAllocationDetailIsCpuMapped) != kRmtAllocationDetailIsCpuMapped) && slice_index == 0)
    {
        AddClusterResource(parent_cluster, slice_index, resource, resource_added);
    }
}

void RMVTreeMapBlocks::FilterResourceCommitType(ResourceCluster&       parent_cluster,
                                                int32_t                slice_index,
                                                const RmtDataSnapshot* snapshot,
                                                const RmtResource*     resource,
                                                bool&                  resource_added) const
{
    RMT_ASSERT(snapshot);
    if (resource != nullptr && resource->commit_type == slice_index)
    {
        AddClusterResource(parent_cluster, slice_index, resource, resource_added);
    }
}

void RMVTreeMapBlocks::FilterResourceOwner(ResourceCluster&       parent_cluster,
                                           int32_t                slice_index,
                                           const RmtDataSnapshot* snapshot,
                                           const RmtResource*     resource,
                                           bool&                  resource_added) const
{
    RMT_ASSERT(snapshot);
    if (resource != nullptr && resource->owner_type == slice_index)
    {
        AddClusterResource(parent_cluster, slice_index, resource, resource_added);
    }
}

void RMVTreeMapBlocks::FilterActualHeap(ResourceCluster&       parent_cluster,
                                        int32_t                slice_index,
                                        const RmtDataSnapshot* snapshot,
                                        const RmtResource*     resource,
                                        bool&                  resource_added) const
{
    if (resource != nullptr && RmtResourceGetActualHeap(snapshot, resource) == slice_index)
    {
        AddClusterResource(parent_cluster, slice_index, resource, resource_added);
    }
}

void RMVTreeMapBlocks::FilterInPreferredHeap(ResourceCluster&       parent_cluster,
                                             int32_t                slice_index,
                                             const RmtDataSnapshot* snapshot,
                                             const RmtResource*     resource,
                                             bool&                  resource_added) const
{
    if (!resource || !resource->bound_allocation || resource->resource_type == kRmtResourceTypeCount)
        return;

    uint64_t memory_segment_histogram[kRmtResourceBackingStorageCount] = {0};
    RmtResourceGetBackingStorageHistogram(snapshot, resource, memory_segment_histogram);

    const RmtHeapType preferred_heap = resource->bound_allocation->heap_preferences[0];
    if (memory_segment_histogram[preferred_heap] != resource->adjusted_size_in_bytes && slice_index == 0)
    {
        AddClusterResource(parent_cluster, slice_index, resource, resource_added);
    }
    else if (memory_segment_histogram[preferred_heap] == resource->adjusted_size_in_bytes && slice_index == 1)
    {
        AddClusterResource(parent_cluster, slice_index, resource, resource_added);
    }
}

void RMVTreeMapBlocks::FillClusterResources(ResourceCluster& parent_cluster, QVector<SliceType>& target_slice_types, int32_t level, int32_t max_level)
{
    RMT_ASSERT(level <= max_level);

    if (level == max_level)
    {
        return;
    }

    RMT_ASSERT(level < target_slice_types.size());
    SliceType slice_type = target_slice_types[level];

    const RmtDataSnapshot* snapshot = rmv::SnapshotManager::Get().GetOpenSnapshot();
    if (!rmv::TraceManager::Get().DataSetValid() || snapshot == nullptr)
    {
        return;
    }

    int32_t slice_count = GetSliceCount(slice_type, snapshot);

    // Special case the virtual allocation.
    if (slice_type == kSliceTypeVirtualAllocation)
    {
        for (int32_t slice_index = 0; slice_index < slice_count; slice_index++)
        {
            const RmtVirtualAllocation* virtual_allocation = &snapshot->virtual_allocation_list.allocation_details[slice_index];
            bool                        found_allocs       = false;

            for (int32_t resource_index = 0; resource_index < parent_cluster.sorted_resources.size(); resource_index++)
            {
                const RmtResource* resource = parent_cluster.sorted_resources[resource_index];
                if (resource != nullptr && resource->bound_allocation == virtual_allocation)
                {
                    AddClusterResource(parent_cluster, slice_index, resource, found_allocs);
                }
            }

            if (found_allocs)
            {
                FillClusterResources(parent_cluster.sub_clusters[slice_index], target_slice_types, level + 1, max_level);
            }
        }
    }

    else
    {
        typedef void (RMVTreeMapBlocks::*FilterFunction)(
            ResourceCluster & parent_cluster, int32_t slice_index, const RmtDataSnapshot* snapshot, const RmtResource* resource, bool& found_allocs) const;

        FilterFunction fn = nullptr;

        switch (slice_type)
        {
        case kSliceTypeResourceUsageType:
            fn = &RMVTreeMapBlocks::FilterResourceUsageType;
            break;

        case kSliceTypeResourceCreateAge:
            fn = &RMVTreeMapBlocks::FilterResourceCreateAge;
            break;

        case kSliceTypeResourceBindAge:
            fn = &RMVTreeMapBlocks::FilterResourceBindAge;
            break;

        case kSliceTypeAllocationAge:
            fn = &RMVTreeMapBlocks::FilterAllocationAge;
            break;

        case kSliceTypePreferredHeap:
            fn = &RMVTreeMapBlocks::FilterPreferredHeap;
            break;

        case kSliceTypeCpuMapped:
            fn = &RMVTreeMapBlocks::FilterCpuMapped;
            break;

        case kSliceTypeResourceCommitType:
            fn = &RMVTreeMapBlocks::FilterResourceCommitType;
            break;

        case kSliceTypeResourceOwner:
            fn = &RMVTreeMapBlocks::FilterResourceOwner;
            break;

        case kSliceTypeActualHeap:
            fn = &RMVTreeMapBlocks::FilterActualHeap;
            break;

        case kSliceTypeInPreferredHeap:
            fn = &RMVTreeMapBlocks::FilterInPreferredHeap;
            break;

        default:
            break;
        }

        if (fn != nullptr)
        {
            for (int32_t slice_index = 0; slice_index < slice_count; slice_index++)
            {
                bool found_allocs = false;

                for (int32_t resource_index = 0; resource_index < parent_cluster.sorted_resources.size(); resource_index++)
                {
                    const RmtResource* resource = parent_cluster.sorted_resources[resource_index];
                    (this->*fn)(parent_cluster, slice_index, snapshot, resource, found_allocs);
                }

                if (found_allocs)
                {
                    FillClusterResources(parent_cluster.sub_clusters[slice_index], target_slice_types, level + 1, max_level);
                }
            }
        }
    }
}

void RMVTreeMapBlocks::UpdateSliceTypes(const QVector<SliceType>& slice_types)
{
    slice_types_ = slice_types;
}