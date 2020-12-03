//=============================================================================
/// Copyright (c) 2018-2020 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Header for a tree map block collection
//=============================================================================

#ifndef RMV_VIEWS_CUSTOM_WIDGETS_RMV_TREE_MAP_BLOCKS_H_
#define RMV_VIEWS_CUSTOM_WIDGETS_RMV_TREE_MAP_BLOCKS_H_

#include <QGraphicsObject>

#include "rmt_resource_list.h"

#include "models/combo_box_model.h"
#include "models/heap_combo_box_model.h"
#include "models/resource_usage_combo_box_model.h"
#include "models/snapshot/resource_overview_model.h"
#include "views/colorizer.h"

/// A single block seen in the tree map.
struct TreeMapBlockData
{
    const RmtResource* resource;       ///< The represented resource.
    QRectF             bounding_rect;  ///< The offset and size.
    bool               is_visible;     ///< Rendered or not.
};

/// Basic rendering information about the tree map.
struct RMVTreeMapBlocksConfig
{
    int32_t width;   ///< Widget width.
    int32_t height;  ///< Widget height.
};

/// Holds information about how rectangles are sliced during tree map generation.
struct CutData
{
    QVector<QRectF>             rectangles;     ///< Children rects.
    QVector<const RmtResource*> resources;      ///< Children allocations.
    QRectF                      bounding_rect;  ///< Encompassing rect.
    bool                        is_vertical;    ///< Is it vertical or horizontal.
    int64_t                     size_in_bytes;  ///< How much mem does it represent.
    bool                        is_null;        ///< Good or bad.
};

typedef QMap<const RmtResource*, QRectF> AllocGeometryMap;

/// Describes a cluster, which is a square with potentially other child clusters.
struct ResourceCluster
{
    ResourceCluster()
        : amount(0)
    {
    }

    AllocGeometryMap                alloc_geometry_map;  ///< Association of allocation pointers to their rendered geometry.
    QVector<const RmtResource*>     sorted_resources;    ///< Array of all child allocations, sorted by size.
    QMap<uint32_t, ResourceCluster> sub_clusters;        ///< Collection of children clusters.
    uint64_t                        amount;              ///< Total size of this cluster.
    QRectF                          geometry;            ///< Encompassing geometry.
};

/// Various models used to filter the tree map.
struct TreeMapModels
{
    rmv::HeapComboBoxModel*          preferred_heap_model;  ///< The preferred heap model.
    rmv::HeapComboBoxModel*          actual_heap_model;     ///< The actual heap model.
    rmv::ResourceUsageComboBoxModel* resource_usage_model;  ///< The resource usage model.
};

typedef QMap<uint32_t, ResourceCluster> ClusterMap;

/// Container class for a widget that manages TreeMap rendering.
class RMVTreeMapBlocks : public QGraphicsObject
{
    Q_OBJECT

public:
    /// Enum of slicing mode types.
    enum SliceType
    {
        kSliceTypeNone,
        kSliceTypeResourceUsageType,
        kSliceTypeResourceCreateAge,
        kSliceTypeResourceBindAge,
        kSliceTypeAllocationAge,
        kSliceTypeVirtualAllocation,
        kSliceTypePreferredHeap,
        kSliceTypeActualHeap,
        kSliceTypeCpuMapped,
        kSliceTypeResourceCommitType,
        kSliceTypeResourceOwner,
        kSliceTypeInPreferredHeap,

        kSliceTypeCount,
    };

    /// Constructor.
    /// \param config A configuration struct for this object.
    explicit RMVTreeMapBlocks(const RMVTreeMapBlocksConfig& config);

    /// Destructor.
    virtual ~RMVTreeMapBlocks();

    /// Mouse hover over event.
    /// \param event the QGraphicsSceneHoverEvent.
    virtual void hoverMoveEvent(QGraphicsSceneHoverEvent* event) Q_DECL_OVERRIDE;

    /// Mouse hover leave event.
    /// \param event the QGraphicsSceneHoverEvent.
    virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) Q_DECL_OVERRIDE;

    /// Mouse press event.
    /// \param event the QGraphicsSceneMouseEvent.
    virtual void mousePressEvent(QGraphicsSceneMouseEvent* event) Q_DECL_OVERRIDE;

    /// Mouse double click event.
    /// \param event the QGraphicsSceneMouseEvent.
    virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) Q_DECL_OVERRIDE;

    /// Implementation of Qt's bounding volume for this item.
    /// \return The item's bounding rectangle.
    virtual QRectF boundingRect() const Q_DECL_OVERRIDE;

    /// Implementation of Qt's paint for this item.
    /// \param painter The painter object to use.
    /// \param option Provides style options for the item, such as its state, exposed area and its level-of-detail hints.
    /// \param widget Points to the widget that is being painted on if specified.
    virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) Q_DECL_OVERRIDE;

    /// Parse dataset and generate rectangle positions.
    /// \param overview_model The model data for the resource overview model.
    /// \param tree_map_models The models used to filter the resource data.
    /// \param view_width available width.
    /// \param view_height available height.
    void GenerateTreemap(const rmv::ResourceOverviewModel* overview_model, const TreeMapModels& tree_map_models, uint32_t view_width, uint32_t view_height);

    /// Reset state.
    void Reset();

    /// Reset selections.
    void ResetSelections();

    /// Select a resource.
    /// \param resource_identifier the resource identifier.
    void SelectResource(RmtResourceIdentifier resource_identifier);

    /// Update the dimensions.
    /// \param width the width.
    /// \param height the height.
    void UpdateDimensions(int width, int height);

    /// Update slicing types coming in from UI.
    /// \param slice_types types in a vector.
    void UpdateSliceTypes(const QVector<SliceType>& slice_types);

    /// Set the colorizer so that the widget knows which colors to draw the resources.
    /// \param colorizer The colorizer to use.
    void SetColorizer(const Colorizer* colorizer);

signals:
    /// Signal that a resource has been selected.
    /// \param resource_identifier The selected resource.
    /// \param broadcast If true, broadcast the ResourceSelected message to any other panes listening.
    /// \param navigate_to_pane If true, indicate that navigation to another pane is requested.
    /// It is up the the slot to decide how to process the second 2 arguments (which panes to
    /// broadcast to and which pane to navigate to).
    void ResourceSelected(RmtResourceIdentifier resource_identifier, bool broadcast, bool navigate_to_pane);

    /// signal that an unbound resource has been selected.
    /// \param unbound_resource The selected unbound resource.
    /// \param broadcast If true, broadcast the ResourceSelected message to any other panes listening
    /// \param navigate_to_pane If true, indicate that navigation to another pane is requested.
    /// It is up the the slot to decide how to process the second 2 arguments (which panes to
    /// broadcast to and which pane to navigate to).
    void UnboundResourceSelected(const RmtResource* unbound_resource, bool broadcast, bool navigate_to_pane);

private:
    /// Calculate aspect ratio.
    /// \param width width.
    /// \param height height.
    /// \return aspect ratio.
    double CalculateAspectRatio(double width, double height);

    /// Add a cut to the list of rectangles that will be rendered out.
    /// \param existing_cut the cut.
    /// \param offset_x x offset.
    /// \param offset_y y offset.
    /// \param alloc_geometry_map the geometry map.
    void DumpCut(const CutData& existing_cut, uint32_t offset_x, uint32_t offset_y, AllocGeometryMap& alloc_geometry_map) const;

    /// Fill in a cluster with resources that fall within it.
    /// \param parent_cluster the cluster process.
    /// \param target_slice_types a vector of the currently selected slicing types.
    /// \param level current recursion level.
    /// \param max_level max allowed depth.
    void FillClusterResources(ResourceCluster& parent_cluster, QVector<SliceType>& target_slice_types, int level, int max_mevel);

    /// Compute geometry for a cluster.
    /// \param parent_cluster the cluster to look at.
    /// \param parent_width parent width.
    /// \param parent_height parent height.
    /// \param parent_offset_x starting X-offset.
    /// \param parent_offset_y starting Y-offset.
    void FillClusterGeometry(ResourceCluster& parent_cluster, int parent_width, int parent_height, int parent_offset_x, int parent_offset_y);

    /// Get block data given a set of coordinates.
    /// \param cluster the cluster to look at.
    /// \param user_location the position.
    /// \param resource_identifier output resource identifier.
    /// \param out_resource output parent resource.
    bool FindBlockData(ResourceCluster& cluster, QPointF user_location, RmtResourceIdentifier& resource_identifier, const RmtResource*& resource);

    /// Workhorse function to calculate tree map geometry.
    /// \param resources incoming resources to map out.
    /// \param total_size total byte size.
    /// \param view_width available width.
    /// \param view_height available height.
    /// \param offset_x x offset.
    /// \param offset_y y offset.
    /// \param alloc_geometry_map output data containing alloc offsets and sizes.
    void GenerateTreeMapRects(QVector<const RmtResource*>& resources,
                              uint64_t                     total_size,
                              uint32_t                     view_width,
                              uint32_t                     view_height,
                              uint32_t                     offset_x,
                              uint32_t                     offset_y,
                              AllocGeometryMap&            alloc_geometry_map);

    /// Recursive paint function to draw borders around slicing modes.
    /// \param painter Qt painter pointer.
    /// \param cluster The current cluster we're rendering.
    void PaintClusterParents(QPainter* painter, ResourceCluster& cluster);

    /// Recursive paint function to paint blocks inside each cluster.
    /// \param painter Qt painter pointer.
    /// \param cluster The current cluster we're rendering.
    /// \param hovered_resource output hovered resource.
    /// \param selected_resource output selected resource.
    void PaintClusterChildren(QPainter* painter, ResourceCluster& cluster, TreeMapBlockData& hovered_resource, TreeMapBlockData& selected_resource);

    /// Get scaled height.
    /// \return scaled height.
    int32_t ScaledHeight() const;

    /// Get scaled width.
    /// \return scaled width.
    int32_t ScaledWidth() const;

    /// Determine if we should draw vertically.
    /// \param width width.
    /// \param height height.
    /// \return true if should draw vertically.
    bool ShouldDrawVertically(double width, double height);

    /// Apply the filters to the resource to see if it should be shown in the treemap.
    /// \param overview_model The model data for the resource overview model.
    /// \param tree_map_models The models used to filter the resource data.
    /// \param snapshot The current snapshot.
    /// \param resource The resource to test.
    /// \return true if the resource should be included, false if not.
    bool ResourceFiltered(const rmv::ResourceOverviewModel* overview_model,
                          const TreeMapModels&              tree_map_models,
                          const RmtDataSnapshot*            snapshot,
                          const RmtResource*                resource);

    /// Get the slice count depending on the slicing mode. This is the number of slices needed to
    /// show the data, for example, slicing by whether a resource is in its preferred heap would
    /// return a count of 2 (those in the preferred heap and those not).
    /// \param slice_type The slicing mode.
    /// \param snapshot The currently opened snapshot.
    /// \return The slicing count.
    int32_t GetSliceCount(const SliceType slice_type, const RmtDataSnapshot* snapshot) const;

    /// Add a resource to the current cluster.
    /// \param parent_cluster the parent cluster.
    /// \param slice_index The slice index.
    /// \param resource The resource to add.
    /// \param resource_added A variable that is updated to indicate that a resource has been added.
    void AddClusterResource(ResourceCluster& parent_cluster, const int32_t slice_index, const RmtResource* resource, bool& resource_added) const;

    /// Filter to slice by resource usage type.
    /// \param parent_cluster the parent cluster.
    /// \param slice_index The slice index.
    /// \param snapshot The currently opened snapshot.
    /// \param resource The resource to add.
    /// \param resource_added A variable that is updated to indicate that a resource has been added.
    void FilterResourceUsageType(ResourceCluster&       parent_cluster,
                                 int32_t                slice_index,
                                 const RmtDataSnapshot* snapshot,
                                 const RmtResource*     resource,
                                 bool&                  resource_added) const;

    /// Filter to slice by resource create age.
    /// \param parent_cluster the parent cluster.
    /// \param slice_index The slice index.
    /// \param snapshot The currently opened snapshot.
    /// \param resource The resource to add.
    /// \param resource_added A variable that is updated to indicate that a resource has been added.
    void FilterResourceCreateAge(ResourceCluster&       parent_cluster,
                                 int32_t                slice_index,
                                 const RmtDataSnapshot* snapshot,
                                 const RmtResource*     resource,
                                 bool&                  resource_added) const;

    /// Filter to slice by resource bind age.
    /// \param parent_cluster the parent cluster.
    /// \param slice_index The slice index.
    /// \param snapshot The currently opened snapshot.
    /// \param resource The resource to add.
    /// \param resource_added A variable that is updated to indicate that a resource has been added.
    void FilterResourceBindAge(ResourceCluster&       parent_cluster,
                               int32_t                slice_index,
                               const RmtDataSnapshot* snapshot,
                               const RmtResource*     resource,
                               bool&                  resource_added) const;

    /// Filter to slice by allocation age.
    /// \param parent_cluster the parent cluster.
    /// \param slice_index The slice index.
    /// \param snapshot The currently opened snapshot.
    /// \param resource The resource to add.
    /// \param resource_added A variable that is updated to indicate that a resource has been added.
    void FilterAllocationAge(ResourceCluster&       parent_cluster,
                             int32_t                slice_index,
                             const RmtDataSnapshot* snapshot,
                             const RmtResource*     resource,
                             bool&                  resource_added) const;

    /// Filter to slice by preferred heap.
    /// \param parent_cluster the parent cluster.
    /// \param slice_index The slice index.
    /// \param snapshot The currently opened snapshot.
    /// \param resource The resource to add.
    /// \param resource_added A variable that is updated to indicate that a resource has been added.
    void FilterPreferredHeap(ResourceCluster&       parent_cluster,
                             int32_t                slice_index,
                             const RmtDataSnapshot* snapshot,
                             const RmtResource*     resource,
                             bool&                  resource_added) const;

    /// Filter to slice by whether a resource is CPU mapped.
    /// \param parent_cluster the parent cluster.
    /// \param slice_index The slice index.
    /// \param snapshot The currently opened snapshot.
    /// \param resource The resource to add.
    /// \param resource_added A variable that is updated to indicate that a resource has been added.
    void FilterCpuMapped(ResourceCluster&       parent_cluster,
                         int32_t                slice_index,
                         const RmtDataSnapshot* snapshot,
                         const RmtResource*     resource,
                         bool&                  resource_added) const;

    /// Filter to slice by resource commit type.
    /// \param parent_cluster the parent cluster.
    /// \param slice_index The slice index.
    /// \param snapshot The currently opened snapshot.
    /// \param resource The resource to add.
    /// \param resource_added A variable that is updated to indicate that a resource has been added.
    void FilterResourceCommitType(ResourceCluster&       parent_cluster,
                                  int32_t                slice_index,
                                  const RmtDataSnapshot* snapshot,
                                  const RmtResource*     resource,
                                  bool&                  resource_added) const;

    /// Filter to slice by resource owner.
    /// \param parent_cluster the parent cluster.
    /// \param slice_index The slice index.
    /// \param snapshot The currently opened snapshot.
    /// \param resource The resource to add.
    /// \param resource_added A variable that is updated to indicate that a resource has been added.
    void FilterResourceOwner(ResourceCluster&       parent_cluster,
                             int32_t                slice_index,
                             const RmtDataSnapshot* snapshot,
                             const RmtResource*     resource,
                             bool&                  resource_added) const;

    /// Filter to slice by actual heap.
    /// \param parent_cluster the parent cluster.
    /// \param slice_index The slice index.
    /// \param snapshot The currently opened snapshot.
    /// \param resource The resource to add.
    /// \param resource_added A variable that is updated to indicate that a resource has been added.
    void FilterActualHeap(ResourceCluster&       parent_cluster,
                          int32_t                slice_index,
                          const RmtDataSnapshot* snapshot,
                          const RmtResource*     resource,
                          bool&                  resource_added) const;

    /// Filter to slice by whether a resource is in its preferred heap.
    /// \param parent_cluster the parent cluster.
    /// \param slice_index The slice index.
    /// \param snapshot The currently opened snapshot.
    /// \param resource The resource to add.
    /// \param resource_added A variable that is updated to indicate that a resource has been added.
    void FilterInPreferredHeap(ResourceCluster&       parent_cluster,
                               int32_t                slice_index,
                               const RmtDataSnapshot* snapshot,
                               const RmtResource*     resource,
                               bool&                  resource_added) const;

    RMVTreeMapBlocksConfig          config_;                        ///< Description of this widget.
    RmtResourceIdentifier           hovered_resource_identifier_;   ///< Id of the allocation hovered over.
    RmtResourceIdentifier           selected_resource_identifier_;  ///< Id of the selected allocation.
    const RmtResource*              hovered_resource_;              ///< The hovered resource (incase the resource is unbound).
    const RmtResource*              selected_resource_;             ///< The selected resource (incase the resource is unbound).
    QMap<uint32_t, ResourceCluster> clusters_;                      ///< The master data structure that holds all recursive block layouts.
    QVector<SliceType>              slice_types_;                   ///< Holds UI slicing selections.
    const Colorizer*                colorizer_;                     ///< The colorizer for deciding how to color the blocks.
    QVector<RmtResource*>           unbound_resources_;             ///< A list of unbound resources.
};

#endif  // RMV_VIEWS_CUSTOM_WIDGETS_RMV_TREE_MAP_BLOCKS_H_
