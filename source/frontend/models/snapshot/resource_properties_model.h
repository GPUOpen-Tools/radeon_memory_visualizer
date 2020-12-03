//=============================================================================
/// Copyright (c) 2019-2020 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Header for the Resource properties model. This is a model to go
/// with a QTableView showing the properties for each type of resource.
/// The table will be populated with properties specific to a resource type.
//=============================================================================

#ifndef RMV_MODELS_SNAPSHOT_RESOURCE_PROPERTIES_MODEL_H_
#define RMV_MODELS_SNAPSHOT_RESOURCE_PROPERTIES_MODEL_H_

#include <QTableView>

#include "qt_common/utils/model_view_mapper.h"

#include "rmt_resource_list.h"

namespace rmv
{
    /// Container class that holds model data for the resource properties table
    /// in the resource details pane.
    class ResourcePropertiesModel
    {
    public:
        /// Constructor.
        ResourcePropertiesModel();

        /// Destructor.
        ~ResourcePropertiesModel();

        /// Initialize the table model.
        /// \param table_view The view to the table.
        /// \param num_rows Total rows of the table.
        /// \param num_columns Total columns of the table.
        void InitializeTableModel(QTableView* table_view, uint num_rows, uint num_columns);

        /// Update the model.
        /// \param resource_identifier The resource identifier.
        /// \return The number of properties for the resource.
        int32_t Update(RmtResourceIdentifier resource_identifier);

    private:
        /// Initialize blank data for the model.
        void ResetModelValues();

        /// Update the resource list table.
        /// \param resource_identifier the resource identifier.
        /// \return The number of properties for the resource.
        int32_t UpdateTable(RmtResourceIdentifier resource_identifier);

        /// Set up the data for 1 row in the table.
        /// \param name The property name.
        /// \param value The property value.
        /// \param row The row in the table where the data is to be written.
        void SetupResourceRow(QString name, QString value, int row);

        /// Add the image properties to the table.
        /// \param resource The resource containing the data to add to the table.
        /// \param row_index The index of the row in the table where data is to be written.
        /// \return the new row index.
        int AddImageTableData(const RmtResource* resource, int row_index);

        /// Add the buffer properties to the table.
        /// \param resource The resource containing the data to add to the table.
        /// \param row_index The index of the row in the table where data is to be written.
        /// \return the new row index.
        int AddBufferTableData(const RmtResource* resource, int row_index);

        /// Add the GPU event properties to the table.
        /// \param resource The resource containing the data to add to the table.
        /// \param row_index The index of the row in the table where data is to be written.
        /// \return the new row index.
        int AddGPUEventTableData(const RmtResource* resource, int row_index);

        /// Add the border color palette properties to the table.
        /// \param resource The resource containing the data to add to the table.
        /// \param row_index The index of the row in the table where data is to be written.
        /// \return the new row index.
        int AddBorderColorPaletteTableData(const RmtResource* resource, int row_index);

        /// Add the perf experiment properties to the table.
        /// \param resource The resource containing the data to add to the table.
        /// \param row_index The index of the row in the table where data is to be written.
        /// \return the new row index.
        int AddPerfExperimentTableData(const RmtResource* resource, int row_index);

        /// Add the query heap properties to the table.
        /// \param resource The resource containing the data to add to the table.
        /// \param row_index The index of the row in the table where data is to be written.
        /// \return the new row index.
        int AddQueryHeapTableData(const RmtResource* resource, int row_index);

        /// Add the video decoder properties to the table.
        /// \param resource The resource containing the data to add to the table.
        /// \param row_index The index of the row in the table where data is to be written.
        /// \return the new row index.
        int AddVideoDecoderTableData(const RmtResource* resource, int row_index);

        /// Add the video encoder properties to the table.
        /// \param resource The resource containing the data to add to the table.
        /// \param row_index The index of the row in the table where data is to be written.
        /// \return the new row index.
        int AddVideoEncoderTableData(const RmtResource* resource, int row_index);

        /// Add the heap properties to the table.
        /// \param resource The resource containing the data to add to the table.
        /// \param row_index The index of the row in the table where data is to be written.
        /// \return the new row index.
        int AddHeapTableData(const RmtResource* resource, int row_index);

        /// Add the pipeline properties to the table.
        /// \param resource The resource containing the data to add to the table.
        /// \param row_index The index of the row in the table where data is to be written.
        /// \return the new row index.
        int AddPipelineTableData(const RmtResource* resource, int row_index);

        /// Add the descriptor heap properties to the table.
        /// \param resource The resource containing the data to add to the table.
        /// \param row_index The index of the row in the table where data is to be written.
        /// \return the new row index.
        int AddDescriptorHeapTableData(const RmtResource* resource, int row_index);

        /// Add the descriptor pool properties to the table.
        /// \param resource The resource containing the data to add to the table.
        /// \param row_index The index of the row in the table where data is to be written.
        /// \return the new row index.
        int AddDescriptorPoolTableData(const RmtResource* resource, int row_index);

        /// Add the command allocator properties to the table.
        /// \param resource The resource containing the data to add to the table.
        /// \param row_index The index of the row in the table where data is to be written.
        /// \return the new row index.
        int AddCommandAllocatorTableData(const RmtResource* resource, int row_index);

        QStandardItemModel* table_model_;  ///< Holds table data.
    };
}  // namespace rmv

#endif  // RMV_MODELS_SNAPSHOT_RESOURCE_PROPERTIES_MODEL_H_
