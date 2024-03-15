//=============================================================================
// Copyright (c) 2023-2024 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Include the pevents source file the required pragmas.
///
/// Switch off anything that will fail due to warnings as errors.
//=============================================================================

#ifndef _WIN32
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
#include "pevents/src/pevents.cpp"
#pragma GCC diagnostic pop
#endif
