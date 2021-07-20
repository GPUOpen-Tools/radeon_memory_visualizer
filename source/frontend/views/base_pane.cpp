//=============================================================================
// Copyright (c) 2018-2021 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Implementation for a base pane class.
//=============================================================================

#include "base_pane.h"

BasePane::BasePane(QWidget* parent)
    : QWidget(parent)
{
}

BasePane::~BasePane()
{
}

void BasePane::OpenSnapshot(RmtDataSnapshot* snapshot)
{
    Q_UNUSED(snapshot);
}

void BasePane::SwitchTimeUnits()
{
}

void BasePane::OnTraceClose()
{
}

void BasePane::Reset()
{
}

void BasePane::ChangeColoring()
{
}
