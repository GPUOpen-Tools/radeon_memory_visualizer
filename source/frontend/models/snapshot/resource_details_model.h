//=============================================================================
/// Copyright (c) 2019-2020 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Header for the Resource details model
//=============================================================================

#ifndef RMV_MODELS_SNAPSHOT_RESOURCE_DETAILS_MODEL_H_
#define RMV_MODELS_SNAPSHOT_RESOURCE_DETAILS_MODEL_H_

#include <QColor>

#include "qt_common/custom_widgets/scaled_table_view.h"
#include "qt_common/utils/model_view_mapper.h"

#include "rmt_resource_history.h"
#include "rmt_resource_list.h"

#include "models/proxy_models/resource_details_proxy_model.h"
#include "models/snapshot/resource_properties_model.h"
#include "models/snapshot/resource_timeline_item_model.h"

namespace rmv
{
    /// Enum containing indices for the widgets shared between the model and UI.
    enum ResourceDetailsWidgets
    {
        kResourceDetailsResourceName,

        kResourceDetailsAllocationBaseAddress,
        kResourceDetailsAllocationOffset,
        kResourceDetailsBaseAddress,
        kResourceDetailsSize,
        kResourceDetailsType,
        kResourceDetailsHeap,
        kResourceDetailsFullyMapped,
        kResourceDetailsUnmappedPercentage,
        kResourceDetailsCreateTime,
        kResourceDetailsBindTime,
        kResourceDetailsCommitTime,
        kResourceDetailsOwnerTime,
        kResourceDetailsFlags,

        kResourceDetailsNumWidgets,
    };

    /// Enum containing indices for the icon shapes.
    enum ResourceIconShape
    {
        kIconShapeCross,
        kIconShapeCircle,
        kIconShapeTriangle,
        kIconShapeInvertedTriangle,
        kIconShapeSquare,
    };

    /// struct containing resource event data for the timeline icons.
    struct ResourceEvent
    {
        ResourceEvent()
            : timestamp(0)
            , shape(kIconShapeCross)
        {
        }

        uint64_t          timestamp;  ///< The timestamp for the event.
        QColor            color;      ///< The event color.
        ResourceIconShape shape;      ///< The event shape.
    };

    /// Container class that holds model data for a given pane.
    class ResourceDetailsModel : public ModelViewMapper
    {
    public:
        /// Constructor.
        explicit ResourceDetailsModel();

        /// Destructor.
        virtual ~ResourceDetailsModel();

        /// Is the resource valid.
        /// \param resource_identifier The resource to check for validity.
        /// \return true if resource is valid, false if not.
        bool IsResourceValid(RmtResourceIdentifier resource_identifier) const;

        /// Get whether the resource base address is valid.
        /// \param resource_identifier The resource whose base address to check for validity.
        /// \return true if base address is valid, false if not (orphaned).
        bool IsResourceBaseAddressValid(RmtResourceIdentifier resource_identifier) const;

        /// Initialize the timeline table model.
        /// \param timeline_table_view The view to the table.
        /// \param num_rows Total rows of the table.
        /// \param num_columns Total columns of the table.
        void InitializeTimelineTableModel(QTableView* timeline_table_view, uint num_rows, uint num_columns);

        /// Initialize the resource properties table model.
        /// \param properties_table_view The view to the table.
        /// \param num_rows Total rows of the table.
        /// \param num_columns Total columns of the table.
        void InitializePropertiesTableModel(QTableView* properties_table_view, uint num_rows, uint num_columns);

        /// Get the resource event data for a particular index. All events for a resource are logged in an
        /// array.
        /// \param index The resource event table index to query.
        /// \param width The width of the timeline. Timeline output will be scaled.
        /// \param out_event_data The variable to accept the resource event data.
        /// \return true if event data was successfully obtained, false if error (ie index out of range).
        bool GetEventData(int index, int width, ResourceEvent& out_event_data) const;

        /// Update the model.
        /// \param resource_identifier The identifier of the resource being shown.
        /// \return The number of properties for the resource.
        int32_t Update(RmtResourceIdentifier resource_identifier);

        /// Get the row in the resource event table that corresponds to the event selected on the timeline.
        /// Coordinate values passed in are logical positions between 0.0 and 1.0, where 0.0 corresponds to
        /// the left of the timeline and 1.0 corresponds to the right.
        /// \param logical_position The logical position clicked on in the timeline.
        /// \param icon_size The size of the icon in logical coordinates.
        /// \return The row in the table, or -1 if nothing selected in the timeline.
        int GetEventRowFromTimeline(double logical_position, double icon_size);

        /// Get the color based on the event type.
        /// \param event_type The type of resource event.
        /// \param highlighted Is the event to be highlighted (true if so).
        /// \return The required color.
        QColor GetColorFromEventType(RmtResourceHistoryEventType event_type, bool highlighted) const;

        /// Get the shape based on the event type.
        /// \param event_type The type of resource event.
        /// \return The required shape.
        ResourceIconShape GetShapeFromEventType(RmtResourceHistoryEventType event_type) const;

        /// Generate the resource history from the backend data.
        /// This is run in a background thread so it's important to check the data is valid before
        /// trying to access it.
        void GenerateResourceHistory(RmtResourceIdentifier resource_identifier);

        /// Get the data for the heap residency.
        /// \param resource_identifier The ID of the resource to obtain.
        /// \param index The index of the heap (an RmtHeapType).
        /// \param [out] value The percentage value of memory in the heap specified.
        /// \param [out] name The name of the heap specified.
        /// \param [out] color The color to use for the heap specified.
        /// \return true if residency data was found, false if error.
        bool GetResidencyData(RmtResourceIdentifier resource_identifier, int index, int& value, QString& name, QColor& color) const;

        /// Get the data for the unmapped memory.
        /// \param resource_identifier The ID of the resource to obtain.
        /// \param [out] value The percentage value of unmapped memory.
        /// \param [out] name The name to use for the unmapped memory.
        /// \param [out] color The color to use for the unmapped memory.
        /// \return true if residency data was found, false if error.
        bool GetUnmappedResidencyData(RmtResourceIdentifier resource_identifier, int& value, QString& name, QColor& color) const;

        /// Is all the physical memory mapped to the preferred heap.
        /// This will be used to show a warning message in the UI.
        /// \param resource_identifier The ID of the resource to check.
        /// \return true if all physical memory is where it was requested, false if not.
        bool PhysicalMemoryInPreferredHeap(RmtResourceIdentifier resource_identifier) const;

        /// Get the timeline proxy model. Used to set up a connection between the table being sorted and the UI update.
        /// \return the timeline proxy model.
        ResourceDetailsProxyModel* GetTimelineProxyModel() const;

    public slots:
        /// Slot to handle what happens when a row is selected in the timeline table.
        /// \param proxy_index The proxy model index.
        void TimelineEventSelected(const QModelIndex& proxy_index);

    private:
        /// Get a resource from its ID.
        /// \param resource_identifier The ID of the resource to obtain.
        /// \param resource The returned resource (if valid).
        /// \return true if the resource has been found, false if error.
        bool GetResourceFromResourceId(RmtResourceIdentifier resource_identifier, const RmtResource** resource) const;

        /// Initialize blank data for the model.
        void ResetModelValues();

        /// Update the resource timeline table.
        void UpdateTimelineTable();

        ResourceTimelineItemModel*    timeline_model_;        ///< Holds data for the resource timeline table.
        ResourceDetailsProxyModel*    timeline_proxy_model_;  ///< Timeline table proxy.
        rmv::ResourcePropertiesModel* properties_model_;      ///< Holds data for the resource properties model.
        int                           highlighted_row_;       ///< The row in the timeline table currently selected.
        RmtResourceHistory            resource_history_;      ///< The resource history for the selected event.
    };

}  // namespace rmv

#endif  // RMV_MODELS_SNAPSHOT_RESOURCE_DETAILS_MODEL_H_
