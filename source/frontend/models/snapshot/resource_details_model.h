//=============================================================================
// Copyright (c) 2019-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Header for the Resource details model.
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
#include "util/thread_controller.h"

namespace rmv
{
    /// @brief Enum containing indices for the widgets shared between the model and UI.
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

    /// @brief Enum containing indices for the icon shapes.
    enum ResourceIconShape
    {
        kIconShapeCross,
        kIconShapeCircle,
        kIconShapeTriangle,
        kIconShapeInvertedTriangle,
        kIconShapeSquare,
    };

    /// @brief Struct containing resource event data for the timeline icons.
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

    /// @brief Container class that holds model data for the resource details pane.
    class ResourceDetailsModel : public ModelViewMapper
    {
    public:
        /// @brief Constructor.
        explicit ResourceDetailsModel();

        /// @brief Destructor.
        virtual ~ResourceDetailsModel();

        /// @brief Is the resource valid.
        ///
        /// @param [in] resource_identifier The resource to check for validity.
        ///
        /// @return true if resource is valid, false if not.
        bool IsResourceValid(RmtResourceIdentifier resource_identifier) const;

        /// @brief Get whether the resource base address is valid.
        ///
        /// @param [in] resource_identifier The resource whose base address to check for validity.
        ///
        /// @return true if base address is valid, false if not (orphaned).
        bool IsResourceBaseAddressValid(RmtResourceIdentifier resource_identifier) const;

        /// @brief Initialize the timeline table model.
        ///
        /// @param [in] timeline_table_view The view to the table.
        /// @param [in] num_rows            Total rows of the table.
        /// @param [in] num_columns         Total columns of the table.
        void InitializeTimelineTableModel(ScaledTableView* timeline_table_view, uint num_rows, uint num_columns);

        /// @brief Initialize the resource properties table model.
        ///
        /// @param [in] properties_table_view The view to the table.
        /// @param [in] num_rows              Total rows of the table.
        /// @param [in] num_columns           Total columns of the table.
        void InitializePropertiesTableModel(QTableView* properties_table_view, uint num_rows, uint num_columns);

        /// @brief Get the resource event data for a particular index. All events for a resource are logged in an
        /// array.
        ///
        /// @param [in]  index          The resource event table index to query.
        /// @param [in]  width          The width of the timeline. Timeline output will be scaled.
        /// @param [out] out_event_data The variable to accept the resource event data.
        ///
        /// @return true if event data was successfully obtained, false if error (ie index out of range).
        bool GetEventData(int index, int width, ResourceEvent& out_event_data) const;

        /// @brief Update the model.
        ///
        /// @param [in] resource_identifier The identifier of the resource being shown.
        ///
        /// @return The number of properties for the resource.
        int32_t Update(RmtResourceIdentifier resource_identifier);

        /// @brief Get the row in the resource event table that corresponds to the event selected on the timeline.
        /// Coordinate values passed in are logical positions between 0.0 and 1.0, where 0.0 corresponds to
        /// the left of the timeline and 1.0 corresponds to the right.
        ///
        /// @param [in] logical_position The logical position clicked on in the timeline.
        /// @param [in] icon_size        The size of the icon in logical coordinates.
        ///
        /// @return The row in the table, or -1 if nothing selected in the timeline.
        int GetEventRowFromTimeline(double logical_position, double icon_size);

        /// @brief Get the color based on the event type.
        ///
        /// @param [in] event_type  The type of resource event.
        /// @param [in] highlighted Is the event to be highlighted (true if so).
        ///
        /// @return The required color.
        QColor GetColorFromEventType(RmtResourceHistoryEventType event_type, bool highlighted) const;

        /// @brief Get the shape based on the event type.
        ///
        /// @param [in] event_type The type of resource event.
        ///
        /// @return The required shape.
        ResourceIconShape GetShapeFromEventType(RmtResourceHistoryEventType event_type) const;

        /// @brief Generate the resource history from the backend data.
        ///
        /// This is run in a background thread so it's important to check the data is valid before
        /// trying to access it.
        void GenerateResourceHistory(RmtResourceIdentifier resource_identifier);

        /// @brief Get the data for the heap residency.
        ///
        /// @param [in]  resource_identifier The ID of the resource to obtain.
        /// @param [in]  index               The index of the heap (an RmtHeapType).
        /// @param [out] value               The percentage value of memory in the heap specified.
        /// @param [out] name                The name of the heap specified.
        /// @param [out] color               The color to use for the heap specified.
        ///
        /// @return true if residency data was found, false if error.
        bool GetResidencyData(RmtResourceIdentifier resource_identifier, int index, float& value, QString& name, QColor& color) const;

        /// @brief Get the data for the unmapped memory.
        ///
        /// @param [in]  resource_identifier The ID of the resource to obtain.
        /// @param [out] value               The percentage value of unmapped memory.
        /// @param [out] name                The name to use for the unmapped memory.
        /// @param [out] color               The color to use for the unmapped memory.
        ///
        /// @return true if residency data was found, false if error.
        bool GetUnmappedResidencyData(RmtResourceIdentifier resource_identifier, int& value, QString& name, QColor& color) const;

        /// @brief Is all the physical memory mapped to the preferred heap.
        /// This will be used to show a warning message in the UI.
        ///
        /// @param [in] resource_identifier The ID of the resource to check.
        ///
        /// @return true if all physical memory is where it was requested, false if not.
        bool PhysicalMemoryInPreferredHeap(RmtResourceIdentifier resource_identifier) const;

        /// @brief Get the timeline proxy model. Used to set up a connection between the table being sorted and the UI update.
        ///
        /// @return the timeline proxy model.
        ResourceDetailsProxyModel* GetTimelineProxyModel() const;

        /// @brief Create a worker thread to process the backend data and extract the resource details for a given resource.
        ///
        /// @param [in] resource_identifier The resource identifier for the resource to process.
        ///
        /// @return A pointer to the worker thread object.
        BackgroundTask* CreateWorkerThread(RmtResourceIdentifier resource_identifier);

        /// @brief Get the contents of the Properties table as a string.
        ///
        /// @param [in] resource_identifier  The identifier of the resource currently shown in the properties table.
        /// @param [in] as_csv               If true, format the text in csv format, otherwise use raw text.
        ///
        /// @return
        QString GetPropertiesString(RmtResourceIdentifier resource_identifier, bool as_csv) const;

    public slots:
        /// @brief Slot to handle what happens when a row is selected in the timeline table.
        ///
        /// @param [in] proxy_index The proxy model index.
        void TimelineEventSelected(const QModelIndex& proxy_index);

    private:
        /// @brief Get a resource from its ID.
        ///
        /// @param [in]  resource_identifier The ID of the resource to obtain.
        /// @param [out] resource            The returned resource (if valid).
        ///
        /// @return true if the resource has been found, false if error.
        bool GetResourceFromResourceId(RmtResourceIdentifier resource_identifier, const RmtResource** resource) const;

        /// @brief Initialize blank data for the model.
        void ResetModelValues();

        /// @brief Update the resource timeline table.
        void UpdateTimelineTable();

        /// @brief Given a property name and value, provide a formatted string containing both.
        ///
        /// @param [in] name            The name of the property, as a string.
        /// @param [in] value           The value of the property, as a string.
        /// @param [in] max_name_length The length of the longest name, in characters
        ///
        /// Note: The max_name_length will be used to pad all names to the same length so the columns are
        /// lined up. If the length is -1, a comma will be used to separate the name and value.
        ///
        /// @return The property name and value, as a newline-terminated string.
        QString GetPropertyString(QString name, QString value, int32_t max_name_length) const;

        ResourceTimelineItemModel*    timeline_model_;        ///< Holds data for the resource timeline table.
        ResourceDetailsProxyModel*    timeline_proxy_model_;  ///< Timeline table proxy.
        rmv::ResourcePropertiesModel* properties_model_;      ///< Holds data for the resource properties model.
        int                           highlighted_row_;       ///< The row in the timeline table currently selected.
        RmtResourceHistory            resource_history_;      ///< The resource history for the selected event.
    };

}  // namespace rmv

#endif  // RMV_MODELS_SNAPSHOT_RESOURCE_DETAILS_MODEL_H_
