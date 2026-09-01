//=============================================================================
// Copyright Advanced Micro Devices, Inc.
// SPDX-License-Identifier: MIT
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
