//=============================================================================
// Copyright (c) 2019-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Definition of platform-abstracted atomic functions.
//=============================================================================

#ifndef RMV_BACKEND_RMT_ATOMIC_H_
#define RMV_BACKEND_RMT_ATOMIC_H_

#ifdef __cplusplus
extern "C" {
#endif  // #ifdef __cplusplus

/// Atomically read a 64bit value from a specified address.
///
/// @param [in] address                     The address of the 64bit value to read.
///
/// @returns
/// The 64bit value at the specified address.
///
uint64_t RmtThreadAtomicRead(volatile uint64_t* address);

/// Atomically write a 64bit value from a specified address.
///
/// @param [in] address                     The address of the 64bit value to write.
/// @param [in] value                       The 64bit value to write to <c><i>address</i></c>.
///
/// @returns
/// The 64bit value that was written to the specified address.
///
uint64_t RmtThreadAtomicWrite(volatile uint64_t* address, uint64_t value);

/// Atomically OR a 64bit value into the 64bit value at a specified address.
///
/// @param [in] address                     The address of the 64bit value to OR with.
/// @param [in] value                       The 64bit value to OR with the value at <c><i>address</i></c>.
///
/// @returns
/// The 64bit value that resulted from the OR operation to the specified address.
///
uint64_t RmtThreadAtomicOr(volatile uint64_t* address, uint64_t value);

/// Atomically add a 32bit value into the 32bit value at a specified address.
///
/// @param [in] address                     The address of the 32bit value to add with.
/// @param [in] value                       The 32bit value to add to the value at <c><i>address</i></c>.
///
/// @returns
/// The 32bit value that resulted from adding <c><i>value</i></c> with the value at <c><i>address</i></c>.
///
int32_t RmtThreadAtomicAdd(volatile int32_t* address, int32_t value);

/// Atomically add a 64bit value into the 64bit value at a specified address.
///
/// @param [in] address                     The address of the 64bit value to add with.
/// @param [in] value                       The 64bit value to add to the value at <c><i>address</i></c>.
///
/// @returns
/// The 64bit value that resulted from adding <c><i>value</i></c> with the value at <c><i>address</i></c>.
///
int64_t RmtThreadAtomicAdd64(volatile int64_t* address, int64_t value);

/// Atomically max a 64bit value into the 64bit value at a specified address.
///
/// @param [in] address                     The address of the 64bit value to max with.
/// @param [in] value                       The 64bit value to max against the value at <c><i>address</i></c>.
///
/// @returns
/// The 64bit value that resulted from adding <c><i>value</i></c> with the value at <c><i>address</i></c>.
///
int64_t RmtThreadAtomicMax64(volatile int64_t* address, int64_t value);

#ifdef __cplusplus
}
#endif  // #ifdef __cplusplus
#endif  // #ifndef RMV_BACKEND_RMT_ATOMIC_H_
