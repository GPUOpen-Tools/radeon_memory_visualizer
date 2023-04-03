//=============================================================================
// Copyright (c) 2023 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Implementation of the base class that handles writing snapshot data.
//=============================================================================

#include "rmt_rdf_snapshot_writer.h"

#include "rmt_data_set.h"

RmtSnapshotWriter::RmtSnapshotWriter(RmtDataSet* data_set)
    : data_set_(data_set)
{
}

RmtSnapshotWriter::~RmtSnapshotWriter()
{
}
