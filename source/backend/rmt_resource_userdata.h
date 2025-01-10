//=============================================================================
// Copyright (c) 2023-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Function definitions for any resource sideband data.
///
/// This is typically resource information from other sources, for example,
/// resource names and buffers marked as implicit will come from userdata
/// tokens and could come from other sources ie ETW.
//=============================================================================

#ifndef RMV_BACKEND_RMT_RESOURCE_USERDATA_H_
#define RMV_BACKEND_RMT_RESOURCE_USERDATA_H_

#include "rmt_types.h"
#include "rmt_resource_history.h"
#include "rmt_resource_list.h"

#ifdef __cplusplus
extern "C" {
#endif  // #ifdef __cplusplus

// Type definition for a timestamp value.
typedef uint64_t RmtTimestamp;

/// @brief Process the tracked tokens to generate the final names to resources mapping.
///
/// @param [in]  any_correlations               A flag that indicates, if true, that Correlation UserData tokens where parsed.
///
/// @retval
/// kRmtOk                                      The operation completed successfully.
RmtErrorCode RmtResourceUserdataProcessEvents(const bool any_correlations);

/// @brief Track a ResourceCreate token.
///
/// @param [in]  driver_resource_id             The resource identifier contained in the ResourceCreate token.
/// @param [in]  internal_resource_id           The internally generated resource identifier.
/// @param [in]  resource_type                  The resource type.
/// @param [in]  timestamp                      The timestamp for the token.
///
/// @retval
/// kRmtOk                                      The operation completed successfully.
RmtErrorCode RmtResourceUserdataTrackResourceCreateToken(const RmtResourceIdentifier driver_resource_id,
                                                         const RmtResourceIdentifier internal_resource_id,
                                                         const RmtResourceType       resource_type,
                                                         const RmtTimestamp          timestamp);

/// @brief Track a ResourceDestroy token.
///
/// @param [in]  internal_resource_id           The internal resource ID for the previously created resource.
/// @param [in]  timestamp                      The timestamp for the token.
///
/// @retval
/// kRmtOk                                      The operation completed successfully.
RmtErrorCode RmtResourceUserdataTrackResourceDestroyToken(const RmtResourceIdentifier internal_resource_id, const RmtTimestamp timestamp);

/// @brief Track a Correlation UserData token.
///
/// @param [in]  driver_resource_id             The 32-bit driver resource ID.
/// @param [in]  correlation_id                 The correlation ID.
/// @param [in]  timestamp                      The timestamp for the token.
///
/// @retval
/// kRmtOk                                      The operation completed successfully.
RmtErrorCode RmtResourceUserdataTrackResourceCorrelationToken(const RmtResourceIdentifier    driver_resource_id,
                                                              const RmtCorrelationIdentifier correlation_id,
                                                              const RmtTimestamp             timestamp);

/// @brief Track a Name UserData token.
///
/// @param [in]  resource_name_id               The identifier contained in the Name UserData token (either a resource ID or correlation id).
/// @param [in]  resource_name                  A pointer to the resource name.
/// @param [in]  timestamp                      The timestamp for the token.
/// @param [in]  delay_time                     The delay between when the data for the token was generated and when the token was emitted.
///
/// @retval
/// kRmtOk                                      The operation completed successfully.
/// @retval
/// kRmtErrorMalformedData                      An invalid delay_time was specified.
RmtErrorCode RmtResourceUserdataTrackResourceNameToken(const RmtCorrelationIdentifier resource_name_id,
                                                       const char*                    resource_name,
                                                       const RmtTimestamp             timestamp,
                                                       const RmtTimestamp             delay_time);

/// @brief Lookup the name associated with a resource and update the resource object.
///
/// @param [in]  resource_list                  A pointer to the list of resource objects.
/// @param [in]  internal_resource_id           The internal resource identifier of the resource to be updated.
/// @param [in]  timestamp                      The point on the timeline when the resource name was updated.
///
/// @retval
/// kRmtOk                                      The operation completed successfully.
/// @retval
/// kRmtErrorInvalidPointer                     The operation failed due to <c><i>resource_list</i></c> being an invalid pointer.
/// @retval
/// kRmtErrorResourceNotFound                   A resource name could not be found for the <c><i>internal_resource_id</i></c> specified.
RmtErrorCode RmtResourceUserdataUpdateResourceName(const RmtResourceList*      resource_list,
                                                   const RmtResourceIdentifier internal_resource_id,
                                                   const RmtTimestamp          timestamp);

/// @brief Insert all Resource Named events (if any) into the resource history.
///
/// @param [in]  out_resource_history_list      A pointer to the the resource history to be updated.  The resource history must already have been initialized with the resource ID to use.
///
/// @retval
/// kRmtOk                                      The operation completed successfully.
/// @retval
/// kRmtErrorInvalidPointer                     The operation failed due to <c><i>out_resource_history_list</i></c> being an invalid pointer.
/// @retval
/// kRmtErrorResourceNotFound                   The  internal resource ID specified could not be found.
RmtErrorCode RmtResourceUserdataUpdateNamedResourceHistoryEvents(RmtResourceHistory* out_resource_history_list);

/// @brief Track an implicit resource UserData token.
///
/// @param [in]  correlation_id                 The correlation ID.
/// @param [in]  timestamp                      The timestamp for the token.
/// @param [in]  delay_time                     The delay between when the data for the token was generated and when the token was emitted.
/// @param [in]  implicit_resource_type         The type of implicit resource.
///
/// @retval
/// kRmtOk                                      The operation completed successfully.
/// @retval
/// kRmtErrorMalformedData                      An invalid delay_time was specified.
RmtErrorCode RmtResourceUserdataTrackImplicitResourceToken(const RmtResourceIdentifier   correlation_id,
                                                           const uint64_t                timestamp,
                                                           const uint64_t                delay_time,
                                                           const RmtImplicitResourceType implicit_resource_type);

/// @brief Track when a resource is bound to an allocation.
///
/// @param [in]  resource                       A pointer to a resource object.
/// @param [in]  allocation_identifier          A unique identifier for the allocation this resource is bound to.
///
/// @retval
/// kRmtOk                                      The operation completed successfully.
/// @retval
/// kRmtErrorInvalidPointer                     The resource pointer is invalid.
RmtErrorCode RmtResourceUserDataTrackBoundResource(const RmtResource* resource, const uint64_t allocation_identifier);

/// @brief Retrieve the resource name associated with a resource.
///
/// @param [in]  internal_resource_id           The resource identifier for the name to be retrieved.
/// @param [out] out_resource_name              A pointer to the resource name.
///
/// @retval
/// kRmtOk                                      The operation completed successfully.
/// @retval
/// kRmtErrorInvalidPointer                     The operation failed due to <c><i>out_resource_name</i></c> being an invalid pointer.
/// @retval
/// kRmtErrorResourceNotFound                   A resource name could not be found for the <c><i>internal_resource_id</i></c> specified.
RmtErrorCode RmtResourceUserdataGetResourceName(const RmtResourceIdentifier internal_resource_id, const char** out_resource_name);

/// @brief Retrieve the resource name associated with a resource at a specified time.
///
/// @param [in]  internal_resource_id           The resource identifier for the name to be retrieved.
/// @param [in]  creation_time                  The creation time of the resource.
/// @param [in]  timestamp                      Specifies the point on the timeline to retrieve the resource name.
/// @param [out] out_resource_name              A pointer to the resource name.
///
/// @retval
/// kRmtOk                                      The operation completed successfully.
/// @retval
/// kRmtErrorInvalidPointer                     The operation failed due to <c><i>out_resource_name</i> being an invalid pointer.
/// @retval
/// kRmtErrorResourceNotFound                   A resource name could not be found for the <c><i>internal_resource_id</i></c> specified.
RmtErrorCode RmtResourceUserdataGetResourceNameAtTimestamp(const RmtResourceIdentifier resource_id,
                                                           const RmtTimestamp          creation_time,
                                                           const RmtTimestamp          timestamp,
                                                           const char**                out_resource_name);

/// @brief Retrieve whether a resource is implicit or not.
///
/// @param [in]  resource_id                    The resource identifier for the name to be retrieved.
///
/// @returns                                    true if resource is implicit, false otherwise.
bool RmtResourceUserDataIsResourceImplicit(const RmtResourceIdentifier resource_id);

/// @brief Retrieve the resource ID for a paired heap or image/buffer resource.
///
/// @param [in]  internal_resource_id               The resource identifier to match against.
/// @param [out] out_paired_internal_resource_id    The resource identifier of the paired heap or image/buffer resource.
///
/// @retval
/// kRmtOk                                      The operation completed successfully.
/// @retval
/// kRmtErrorInvalidPointer                     The out_heap_internal_resource_id pointer is invalid.
/// @retval
/// kRmtErrorResourceNotFound                   A resource could not be found for the <c><i>internal_resource_id</i></c> specified.
RmtErrorCode RmtResourceUserDataFindPairedResource(const RmtResourceIdentifier internal_resource_id, RmtResourceIdentifier* out_paired_internal_resource_id);

/// @brief Clear internal UserData lookup maps.
///
void RmtResourceUserDataCleanup();

#ifdef __cplusplus
}
#endif  // #ifdef __cplusplus
#endif  // #ifndef RMV_BACKEND_RMT_RESOURCE_USERDATA_H_
