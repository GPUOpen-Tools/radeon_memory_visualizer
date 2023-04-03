//=============================================================================
// Copyright (c) 2018-2023 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Header for a tree map block collection.
//=============================================================================

#ifndef RMV_VIEWS_CUSTOM_WIDGETS_RMV_TREE_MAP_BLOCKS_H_
#define RMV_VIEWS_CUSTOM_WIDGETS_RMV_TREE_MAP_BLOCKS_H_

#include <QGraphicsObject>

#include "models/colorizer.h"
#include "models/combo_box_model.h"
#include "models/heap_combo_box_model.h"
#include "models/resource_usage_combo_box_model.h"
#include "models/snapshot/resource_overview_model.h"

/// @brief A single block seen in the tree map.
struct TreeMapBlockData
{
    const RmtResource* resource;       ///< The represented resource.
    QRectF             bounding_rect;  ///< The offset and size.
    bool               is_visible;     ///< Rendered or not.
};

/// @brief Basic rendering information about the tree map.
struct RMVTreeMapBlocksConfig
{
    int32_t width;   ///< Widget width.
    int32_t height;  ///< Widget height.
};

/// @brief Holds information about how rectangles are sliced during tree map generation.
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

/// @brief Describes a cluster, which is a square with potentially other child clusters.
struct ResourceCluster
{
    /// @brief Constructor.
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

/// @brief Various models used to filter the tree map.
struct TreeMapModels
{
    rmv::HeapComboBoxModel*          preferred_heap_model;  ///< The preferred heap model.
    rmv::HeapComboBoxModel*          actual_heap_model;     ///< The actual heap model.
    rmv::ResourceUsageComboBoxModel* resource_usage_model;  ///< The resource usage model.
};

typedef QMap<uint32_t, ResourceCluster> ClusterMap;

/// @brief Container class for a widget that manages TreeMap rendering.
class RMVTreeMapBlocks : public QGraphicsObject
{
    Q_OBJECT

public:
    /// @brief Enum of slicing mode types.
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

    /// @brief Constructor.
    ///
    /// @param [in] config A configuration struct for this object.
    explicit RMVTreeMapBlocks(const RMVTreeMapBlocksConfig& config);

    /// @brief Destructor.
    virtual ~RMVTreeMapBlocks();

    /// @brief Mouse hover over event.
    ///
    /// @param [in] event The QGraphicsSceneHoverEvent event object.
    virtual void hoverMoveEvent(QGraphicsSceneHoverEvent* event) Q_DECL_OVERRIDE;

    /// @brief Mouse hover leave event.
    ///
    /// @param [in] event The QGraphicsSceneHoverEvent event object.
    virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) Q_DECL_OVERRIDE;

    /// @brief Mouse press event.
    ///
    /// @param [in] event The QGraphicsSceneMouseEvent event object.
    virtual void mousePressEvent(QGraphicsSceneMouseEvent* event) Q_DECL_OVERRIDE;

    /// @brief Mouse double click event.
    ///
    /// @param [in] event The QGraphicsSceneMouseEvent event object.
    virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) Q_DECL_OVERRIDE;

    /// @brief Implementation of Qt's bounding volume for this item.
    ///
    /// @return The item's bounding rectangle.
    virtual QRectF boundingRect() const Q_DECL_OVERRIDE;

    /// @brief Implementation of Qt's paint for this item.
    ///
    /// @param [in] painter The painter object to use.
    /// @param [in] option  Provides style options for the item, such as its state, exposed area and its level-of-detail hints.
    /// @param [in] widget  Points to the widget that is being painted on if specified.
    virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) Q_DECL_OVERRIDE;

    /// @brief Parse dataset and generate rectangle positions.
    ///
    /// @param [in] overview_model  The model data for the resource overview model.
    /// @param [in] tree_map_models The models used to filter the resource data.
    /// @param [in] view_width      The available width.
    /// @param [in] view_height     The available height.
    void GenerateTreemap(const rmv::ResourceOverviewModel* overview_model, const TreeMapModels& tree_map_models, uint32_t view_width, uint32_t view_height);

    /// @brief Reset state.
    void Reset();

    /// @brief Reset selections.
    void ResetSelections();

    /// @brief Select a resource.
    ///
    /// @param [in] resource_identifier The resource identifier.
    void SelectResource(RmtResourceIdentifier resource_identifier);

    /// @brief Update the dimensions.
    ///
    /// @param [in] width  The width.
    /// @param [in] height The height.
    void UpdateDimensions(int width, int height);

    /// @brief Update slicing types coming in from UI.
    ///
    /// @param [in] slice_types The slicing types in a vector.
    void UpdateSliceTypes(const QVector<SliceType>& slice_types);

    /// @brief Set the colorizer so that the widget knows which colors to draw the resources.
    ///
    /// @param [in] colorizer The colorizer to use.
    void SetColorizer(const rmv::Colorizer* colorizer);

    /// @brief Get the resource the mouse is hovered over.
    ///
    /// @return The resource that the mouse is currently over, or nullptr if mouse isn't over a resource.
    const RmtResource* GetHoveredResource() const;

signals:
    /// @brief Signal that a resource has been selected.
    ///
    /// It is up the the slot to decide how to process the second 2 arguments (which panes to
    /// broadcast to and which pane to navigate to).
    ///
    /// @param [in] resource_identifier The selected resource.
    /// @param [in] broadcast           If true, broadcast the ResourceSelected message to any other panes listening.
    /// @param [in] navigate_to_pane    If true, indicate that navigation to another pane is requested.
    void ResourceSelected(RmtResourceIdentifier resource_identifier, bool broadcast, bool navigate_to_pane);

    /// @brief Signal that an unbound resource has been selected.
    ///
    /// It is up the the slot to decide how to process the second 2 arguments (which panes to
    /// broadcast to and which pane to navigate to).
    ///
    /// @param [in] unbound_resource The selected unbound resource.
    /// @param [in] broadcast        If true, broadcast the ResourceSelected message to any other panes listening.
    /// @param [in] navigate_to_pane If true, indicate that navigation to another pane is requested.
    void UnboundResourceSelected(const RmtResource* unbound_resource, bool broadcast, bool navigate_to_pane);

private:
    /// @brief Calculate aspect ratio from a given width and height.
    ///
    /// @param [in] width  The width.
    /// @param [in] height The height.
    ///
    /// @return The aspect ratio.
    double CalculateAspectRatio(double width, double height) const;

    /// @brief Add a cut to the list of rectangles that will be rendered out.
    ///
    /// @param [in] existing_cut       The cut.
    /// @param [in] offset_x           The x offset.
    /// @param [in] offset_y           The y offset.
    /// @param [in] alloc_geometry_map The geometry map.
    void DumpCut(const CutData& existing_cut, uint32_t offset_x, uint32_t offset_y, AllocGeometryMap& alloc_geometry_map) const;

    /// @brief Fill in a cluster with resources that fall within it.
    ///
    /// @param [in] parent_cluster     The parent cluster.
    /// @param [in] target_slice_types A vector of the currently selected slicing types.
    /// @param [in] level              The current recursion level.
    /// @param [in] max_level          The maximum allowed recursion depth.
    void FillClusterResources(ResourceCluster& parent_cluster, QVector<SliceType>& target_slice_types, int32_t level, int32_t max_level);

    /// @brief Compute geometry for a cluster.
    ///
    /// @param [in] parent_cluster  The parent cluster.
    /// @param [in] parent_width    The parent width.
    /// @param [in] parent_height   The parent height.
    /// @param [in] parent_offset_x The Starting X-offset.
    /// @param [in] parent_offset_y The starting Y-offset.
    void FillClusterGeometry(ResourceCluster& parent_cluster, uint32_t parent_width, uint32_t parent_height, int32_t parent_offset_x, int32_t parent_offset_y);

    /// @brief Get block data given a set of coordinates.
    ///
    /// @param [in]  cluster             The cluster to look at.
    /// @param [in]  user_location       The position.
    /// @param [out] resource_identifier The output resource identifier.
    /// @param [out] resource            The output parent resource.
    ///
    /// @return true if user_location is over the resource, false if not.
    bool FindBlockData(ResourceCluster& cluster, QPointF user_location, RmtResourceIdentifier& resource_identifier, const RmtResource*& resource) const;

    /// @brief Workhorse function to calculate tree map geometry.
    ///
    /// @param [in]  resources          Incoming resources to map out.
    /// @param [in]  total_size         The total size, in bytes.
    /// @param [in]  view_width         The available width.
    /// @param [in]  view_height        The available height.
    /// @param [in]  offset_x           The x offset.
    /// @param [in]  offset_y           The y offset.
    /// @param [out] alloc_geometry_map The output data containing alloc offsets and sizes.
    void GenerateTreeMapRects(QVector<const RmtResource*>& resources,
                              uint64_t                     total_size,
                              uint32_t                     view_width,
                              uint32_t                     view_height,
                              uint32_t                     offset_x,
                              uint32_t                     offset_y,
                              AllocGeometryMap&            alloc_geometry_map);

    /// @brief Recursive paint function to draw borders around slicing modes.
    ///
    /// @param [in] painter The Qt painter object.
    /// @param [in] cluster The current cluster being rendered.
    void PaintClusterParents(QPainter* painter, const ResourceCluster& cluster);

    /// @brief Recursive paint function to paint blocks inside each cluster.
    ///
    /// @param [in]  painter           The Qt painter object.
    /// @param [in]  cluster           The current cluster being rendered.
    /// @param [out] hovered_resource  The output hovered resource.
    /// @param [out] selected_resource The output selected resource.
    void PaintClusterChildren(QPainter* painter, const ResourceCluster& cluster, TreeMapBlockData& hovered_resource, TreeMapBlockData& selected_resource);

    /// @brief Get scaled height.
    ///
    /// @return scaled height.
    int32_t ScaledHeight() const;

    /// @brief Get scaled width.
    ///
    /// @return scaled width.
    int32_t ScaledWidth() const;

    /// @brief Determine if we should draw vertically.
    ///
    /// @param [in] width  The width.
    /// @param [in] height The height.
    ///
    /// @return true if should draw vertically.
    bool ShouldDrawVertically(double width, double height) const;

    /// @brief Apply the filters to the resource to see if it should be shown in the treemap.
    ///
    /// @param [in] overview_model  The model data for the resource overview model.
    /// @param [in] tree_map_models The models used to filter the resource data.
    /// @param [in] snapshot        The current snapshot.
    /// @param [in] resource        The resource to test.
    ///
    /// @return true if the resource should be included, false if not.
    bool ResourceFiltered(const rmv::ResourceOverviewModel* overview_model,
                          const TreeMapModels&              tree_map_models,
                          const RmtDataSnapshot*            snapshot,
                          const RmtResource*                resource);

    /// @brief Get the slice count depending on the slicing mode.
    ///
    /// This is the number of slices needed to show the data, for example, slicing by whether
    /// a resource is in its preferred heap would return a count of 2 (those in the preferred
    /// heap and those not).
    ///
    /// @param [in] slice_type The slicing mode.
    /// @param [in] snapshot   The currently opened snapshot.
    ///
    /// @return The slicing count.
    int32_t GetSliceCount(const SliceType slice_type, const RmtDataSnapshot* snapshot) const;

    /// @brief Add a resource to the current cluster.
    ///
    /// @param [in]      parent_cluster The parent cluster.
    /// @param [in]      slice_index    The slice index.
    /// @param [in]      resource       The resource to add.
    /// @param [in, out] resource_added A variable that is updated to indicate that a resource has been added.
    void AddClusterResource(ResourceCluster& parent_cluster, const int32_t slice_index, const RmtResource* resource, bool& resource_added) const;

    /// @brief Filter to slice by resource usage type.
    ///
    /// @param [in]      parent_cluster The parent cluster.
    /// @param [in]      slice_index    The slice index.
    /// @param [in]      snapshot       The currently opened snapshot.
    /// @param [in]      resource       The resource to add.
    /// @param [in, out] resource_added A variable that is updated to indicate that a resource has been added.
    void FilterResourceUsageType(ResourceCluster&       parent_cluster,
                                 int32_t                slice_index,
                                 const RmtDataSnapshot* snapshot,
                                 const RmtResource*     resource,
                                 bool&                  resource_added) const;

    /// @brief Filter to slice by resource create age.
    ///
    /// @param [in]      parent_cluster The parent cluster.
    /// @param [in]      slice_index    The slice index.
    /// @param [in]      snapshot       The currently opened snapshot.
    /// @param [in]      resource       The resource to add.
    /// @param [in, out] resource_added A variable that is updated to indicate that a resource has been added.
    void FilterResourceCreateAge(ResourceCluster&       parent_cluster,
                                 int32_t                slice_index,
                                 const RmtDataSnapshot* snapshot,
                                 const RmtResource*     resource,
                                 bool&                  resource_added) const;

    /// @brief Filter to slice by resource bind age.
    ///
    /// @param [in]      parent_cluster The parent cluster.
    /// @param [in]      slice_index    The slice index.
    /// @param [in]      snapshot       The currently opened snapshot.
    /// @param [in]      resource       The resource to add.
    /// @param [in, out] resource_added A variable that is updated to indicate that a resource has been added.
    void FilterResourceBindAge(ResourceCluster&       parent_cluster,
                               int32_t                slice_index,
                               const RmtDataSnapshot* snapshot,
                               const RmtResource*     resource,
                               bool&                  resource_added) const;

    /// @brief Filter to slice by allocation age.
    ///
    /// @param [in]      parent_cluster The parent cluster.
    /// @param [in]      slice_index    The slice index.
    /// @param [in]      snapshot       The currently opened snapshot.
    /// @param [in]      resource       The resource to add.
    /// @param [in, out] resource_added A variable that is updated to indicate that a resource has been added.
    void FilterAllocationAge(ResourceCluster&       parent_cluster,
                             int32_t                slice_index,
                             const RmtDataSnapshot* snapshot,
                             const RmtResource*     resource,
                             bool&                  resource_added) const;

    /// @brief Filter to slice by preferred heap.
    ///
    /// @param [in]      parent_cluster The parent cluster.
    /// @param [in]      slice_index    The slice index.
    /// @param [in]      snapshot       The currently opened snapshot.
    /// @param [in]      resource       The resource to add.
    /// @param [in, out] resource_added A variable that is updated to indicate that a resource has been added.
    void FilterPreferredHeap(ResourceCluster&       parent_cluster,
                             int32_t                slice_index,
                             const RmtDataSnapshot* snapshot,
                             const RmtResource*     resource,
                             bool&                  resource_added) const;

    /// @brief Filter to slice by whether a resource is CPU mapped.
    ///
    /// @param [in]      parent_cluster The parent cluster.
    /// @param [in]      slice_index    The slice index.
    /// @param [in]      snapshot       The currently opened snapshot.
    /// @param [in]      resource       The resource to add.
    /// @param [in, out] resource_added A variable that is updated to indicate that a resource has been added.
    void FilterCpuMapped(ResourceCluster&       parent_cluster,
                         int32_t                slice_index,
                         const RmtDataSnapshot* snapshot,
                         const RmtResource*     resource,
                         bool&                  resource_added) const;

    /// @brief Filter to slice by resource commit type.
    ///
    /// @param [in]      parent_cluster The parent cluster.
    /// @param [in]      slice_index    The slice index.
    /// @param [in]      snapshot       The currently opened snapshot.
    /// @param [in]      resource       The resource to add.
    /// @param [in, out] resource_added A variable that is updated to indicate that a resource has been added.
    void FilterResourceCommitType(ResourceCluster&       parent_cluster,
                                  int32_t                slice_index,
                                  const RmtDataSnapshot* snapshot,
                                  const RmtResource*     resource,
                                  bool&                  resource_added) const;

    /// @brief Filter to slice by resource owner.
    ///
    /// @param [in]      parent_cluster The parent cluster.
    /// @param [in]      slice_index    The slice index.
    /// @param [in]      snapshot       The currently opened snapshot.
    /// @param [in]      resource       The resource to add.
    /// @param [in, out] resource_added A variable that is updated to indicate that a resource has been added.
    void FilterResourceOwner(ResourceCluster&       parent_cluster,
                             int32_t                slice_index,
                             const RmtDataSnapshot* snapshot,
                             const RmtResource*     resource,
                             bool&                  resource_added) const;

    /// @brief Filter to slice by actual heap.
    ///
    /// @param [in]      parent_cluster The parent cluster.
    /// @param [in]      slice_index    The slice index.
    /// @param [in]      snapshot       The currently opened snapshot.
    /// @param [in]      resource       The resource to add.
    /// @param [in, out] resource_added A variable that is updated to indicate that a resource has been added.
    void FilterActualHeap(ResourceCluster&       parent_cluster,
                          int32_t                slice_index,
                          const RmtDataSnapshot* snapshot,
                          const RmtResource*     resource,
                          bool&                  resource_added) const;

    /// @brief Filter to slice by whether a resource is in its preferred heap.
    ///
    /// @param [in]      parent_cluster The parent cluster.
    /// @param [in]      slice_index    The slice index.
    /// @param [in]      snapshot       The currently opened snapshot.
    /// @param [in]      resource       The resource to add.
    /// @param [in, out] resource_added A variable that is updated to indicate that a resource has been added.
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
    const rmv::Colorizer*           colorizer_;                     ///< The colorizer for deciding how to color the blocks.
    QVector<RmtResource*>           unbound_resources_;             ///< A list of unbound resources.
};

#endif  // RMV_VIEWS_CUSTOM_WIDGETS_RMV_TREE_MAP_BLOCKS_H_
