//=============================================================================
// Copyright (c) 2023 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Function implementation for any resource userdata.
//=============================================================================

#include "rmt_resource_userdata.h"

#include "rmt_assert.h"
#include "rmt_print.h"
#include "rmt_string_memory_pool.h"

#include <map>
#include <string>
#include <unordered_map>
#include <unordered_set>

// Constant used to indicate an unknown driver resource ID.
static const RmtResourceIdentifier kUnknownDriverResourceId = 0;

// Constant used to indicate an unknown correlation ID.
static const RmtCorrelationIdentifier kUnknownCorrelationId = 0;

// UserData related Token types
enum TokenType
{
    kResourceCreate,
    kResourceDestroy,
    kResourceCorrelation,
    kResourceName,
    kResourceImplicit,
};

// A structure used to hold information about the tokens that are processed.
struct TokenData
{
    TokenType             token_type;
    RmtResourceType       resource_type;
    RmtResourceIdentifier internal_resource_id;
    RmtResourceIdentifier driver_resource_id;
    RmtResourceIdentifier correlation_id;
    char*                 resource_name;
};

// An ID used for tracking resource names.
typedef uint64_t RmtResourceNameHash;

// List of all the events we're interested in, in chronological order, taking into account the ETW lag time.
static std::multimap<RmtTimestamp, TokenData> tokens;

// Given a correlation ID, find the resource hash, and vice-versa.
static std::unordered_map<RmtCorrelationIdentifier, RmtResourceNameHash> correlation_id_to_resource_hash;
static std::unordered_map<RmtResourceNameHash, RmtCorrelationIdentifier> resource_hash_to_correlation_id;

// Given a resource hash, find the internal driver resource identifier, and vice-versa.
static std::unordered_map<RmtResourceNameHash, RmtResourceIdentifier> resource_hash_to_internal_resource_id;
static std::unordered_map<RmtResourceIdentifier, RmtResourceNameHash> internal_resource_id_to_resource_hash_id;

// Given a correlation ID, find the resource name.
static std::unordered_map<RmtCorrelationIdentifier, char*> correlation_id_to_resource_name;

// This is the one that sticks around. Map of resource id to string.  This is used to patch everything up in after processing completes.
static std::unordered_map<RmtResourceIdentifier, char*> internal_resource_id_to_resource_name;

// Set of implicit resources.
static std::unordered_set<RmtResourceIdentifier> resource_identifier_implicit;

// Set of resources needing correlations.
static std::unordered_set<RmtResourceIdentifier> internal_resource_ids_needing_correlation;

// 1MB memory pool for resource name strings.
static const uint64_t kMemoryPoolBlockSize = 1024 * 1024;

// Memory pool for managing resource name text string allocations.
static RmtStringMemoryPool resource_name_string_pool(kMemoryPoolBlockSize);

// Flag indicating, if true, that Name UserData tokens have been parsed.
static bool resource_name_token_tracked = false;

static RmtResourceNameHash ResourceNamingGenerateHash(RmtResourceIdentifier resource_id, RmtCorrelationIdentifier correlation_id)
{
    RMT_ASSERT((resource_id != kUnknownDriverResourceId) || (correlation_id != kUnknownCorrelationId));

    uint64_t hash1 = resource_id;
    uint64_t hash2 = correlation_id;

    // If the resource ID is unknown, replace it with the correlation ID.
    if (resource_id == kUnknownDriverResourceId)
    {
        hash1 = correlation_id;
    }

    // If the correlation ID is unknown, replace it with the resource ID.
    if (correlation_id == kUnknownCorrelationId)
    {
        hash2 = resource_id;
    }

    // Create hash value.
    return ((hash2 << 32) | (hash1 & 0xFFFFFFFF));
}

static RmtResourceIdentifier RemoveHash(const RmtResourceNameHash hash)
{
    const auto& it = resource_hash_to_internal_resource_id.find(hash);
    if (it != resource_hash_to_internal_resource_id.end())
    {
        const auto resource_identifier = (*it).second;
        resource_hash_to_internal_resource_id.erase(it);
        const auto& it2 = internal_resource_id_to_resource_hash_id.find(resource_identifier);
        if (it2 != internal_resource_id_to_resource_hash_id.end())
        {
            internal_resource_id_to_resource_hash_id.erase(resource_identifier);
            return resource_identifier;
        }
        else
        {
            RMT_ASSERT_MESSAGE(false, "Should have found resource!!");
        }
    }
    return 0;
}

static void ProcessResourceCreateToken(const TokenData& token_data)
{
    // If the resource already exists, it may be getting reused by the driver, so remove all instances of it.
    const auto& it = resource_hash_to_internal_resource_id.find(token_data.driver_resource_id);
    if (it != resource_hash_to_internal_resource_id.end())
    {
        // A mapping between the resource name and resource already exists (a previously used driver resource ID).  Remove it from the maps.
        const auto& res_it = internal_resource_id_to_resource_hash_id.find((*it).second);
        if (res_it != internal_resource_id_to_resource_hash_id.end())
        {
            RmtResourceIdentifier resource_id = (*res_it).second;
            internal_resource_id_to_resource_hash_id.erase(res_it);
            const auto& res_it2 = resource_hash_to_internal_resource_id.find(resource_id);
            if (res_it2 != resource_hash_to_internal_resource_id.end())
            {
                resource_hash_to_internal_resource_id.erase(res_it2);
            }
        }
    }

    const RmtResourceNameHash hash = ResourceNamingGenerateHash(token_data.driver_resource_id, kUnknownCorrelationId);

    RemoveHash(hash);

    resource_hash_to_internal_resource_id[hash]                               = token_data.internal_resource_id;
    internal_resource_id_to_resource_hash_id[token_data.internal_resource_id] = hash;

    // Mark this resource as needing a correlation.
    if (token_data.resource_type != kRmtResourceTypeBuffer)
    {
        internal_resource_ids_needing_correlation.insert(token_data.internal_resource_id);
    }
}

static void ProcessResourceCorrelationToken(const TokenData& token_data)
{
    // Look to see if a hash was created using a driver resource ID.  In this case, remove the hash maps and instead, create new mappings using the driver resource ID and correlation ID.
    const RmtResourceNameHash resource_hash = ResourceNamingGenerateHash(token_data.driver_resource_id, kUnknownCorrelationId);

    const RmtResourceIdentifier resource_identifier            = RemoveHash(resource_hash);
    const RmtResourceNameHash   hash                           = ResourceNamingGenerateHash(token_data.driver_resource_id, token_data.correlation_id);
    correlation_id_to_resource_hash[token_data.correlation_id] = hash;
    resource_hash_to_correlation_id[hash]                      = token_data.correlation_id;

    internal_resource_id_to_resource_hash_id[resource_identifier] = hash;
    resource_hash_to_internal_resource_id[hash]                   = resource_identifier;
}

static void ProcessResourceNameToken(const TokenData& token_data, bool any_correlations)
{
    correlation_id_to_resource_name[token_data.correlation_id] = token_data.resource_name;

    if (any_correlations)
    {
        // The trace file contains Correlation UserData tokens.
        const RmtResourceNameHash hash = ResourceNamingGenerateHash(kUnknownDriverResourceId, token_data.correlation_id);

        const auto& internal_resource_id_iterator = resource_hash_to_internal_resource_id.find(hash);
        if (internal_resource_id_iterator != resource_hash_to_internal_resource_id.end())
        {
            const RmtResourceIdentifier internal_resource_identifier = (*internal_resource_id_iterator).second;

            // Make sure this resource doesn't require a correlation ID to match the resource name (it shouldn't be found in the internal_resource_ids_needing_correlation set).
            if (internal_resource_ids_needing_correlation.find(internal_resource_identifier) == internal_resource_ids_needing_correlation.end())
            {
                // Update the internal resource ID to resource name lookup map.
                internal_resource_id_to_resource_name[internal_resource_identifier] = correlation_id_to_resource_name[token_data.correlation_id];
                return;
            }
        }

        // The internal resource id wasn't found using the ID in the Name UserData token as a driver resource ID.
        // Instead, attempt to match the ID in the Name UserData token to a correlation ID.
        const auto& resource_name_hash_iterator = correlation_id_to_resource_hash.find(token_data.correlation_id);
        if (resource_name_hash_iterator != correlation_id_to_resource_hash.end())
        {
            // A matching correlation ID was found in the map.  Use this to lookup the internal resource ID.
            const auto& internal_resource_id_iterator2 = resource_hash_to_internal_resource_id.find((*resource_name_hash_iterator).second);
            if (internal_resource_id_iterator2 != resource_hash_to_internal_resource_id.end())
            {
                // The internal resource ID was found.  Map the resource name to the internal resource ID.
                const RmtResourceIdentifier internal_resource_identifier            = (*internal_resource_id_iterator2).second;
                internal_resource_id_to_resource_name[internal_resource_identifier] = correlation_id_to_resource_name[token_data.correlation_id];
            }
        }
    }
    else
    {
        // No correlation tokens, so correlation ID and resource ID are the same, so bypass the correlation lookup
        // Attempt to lookup the internal resource ID using the ID in the Name UserData token as a driver resource ID.
        RmtResourceIdentifier     driver_resource_id = token_data.correlation_id;
        const RmtResourceNameHash hash               = ResourceNamingGenerateHash(kUnknownDriverResourceId, driver_resource_id);

        const auto& it = resource_hash_to_internal_resource_id.find(hash);
        if (it != resource_hash_to_internal_resource_id.end())
        {
            // The internal resource ID was found.  Map the resource name to the internal resource ID.
            const RmtResourceIdentifier internal_resource_identifier            = (*it).second;
            internal_resource_id_to_resource_name[internal_resource_identifier] = correlation_id_to_resource_name[token_data.correlation_id];
        }
    }
}

static void ProcessImplicitResourceToken(const TokenData& token_data, bool any_correlations)
{
    if (any_correlations)
    {
        // The trace file contains Correlation UserData tokens.
        const RmtResourceNameHash hash = ResourceNamingGenerateHash(kUnknownDriverResourceId, token_data.correlation_id);

        const auto& internal_resource_id_iterator = resource_hash_to_internal_resource_id.find(hash);
        if (internal_resource_id_iterator != resource_hash_to_internal_resource_id.end())
        {
            const RmtResourceIdentifier internal_resource_identifier = (*internal_resource_id_iterator).second;

            // Make sure this resource doesn't require a correlation ID to match the implicit resource.
            if (internal_resource_ids_needing_correlation.find(internal_resource_identifier) == internal_resource_ids_needing_correlation.end())
            {
                // Mark this resource as implicit.
                resource_identifier_implicit.insert(internal_resource_identifier);
                return;
            }
        }

        // The internal resource id wasn't found using the ID in the implicit resource UserData token as a driver resource ID.
        // Instead, attempt to match the ID in the Name UserData token to a correlation ID.
        const auto& resource_name_hash_iterator = correlation_id_to_resource_hash.find(token_data.correlation_id);
        if (resource_name_hash_iterator != correlation_id_to_resource_hash.end())
        {
            // A matching correlation ID was found in the map. Use this to lookup the internal resource ID.
            const auto& internal_resource_id_iterator2 = resource_hash_to_internal_resource_id.find((*resource_name_hash_iterator).second);
            if (internal_resource_id_iterator2 != resource_hash_to_internal_resource_id.end())
            {
                // The internal resource ID was found. Mark this resource as implicit.
                const RmtResourceIdentifier internal_resource_identifier = (*internal_resource_id_iterator2).second;
                resource_identifier_implicit.insert(internal_resource_identifier);
            }
        }
    }
    else
    {
        // No correlation tokens, so correlation ID and resource ID are the same, so bypass the correlation lookup.
        RmtResourceIdentifier     driver_resource_id = token_data.correlation_id;
        const RmtResourceNameHash hash               = ResourceNamingGenerateHash(kUnknownDriverResourceId, driver_resource_id);

        const auto& it = resource_hash_to_internal_resource_id.find(hash);
        if (it != resource_hash_to_internal_resource_id.end())
        {
            // The internal resource ID was found. Mark this resource as implicit.
            const RmtResourceIdentifier internal_resource_identifier = (*it).second;
            resource_identifier_implicit.insert(internal_resource_identifier);
        }
    }
}

RmtErrorCode RmtResourceUserdataProcessEvents(const bool any_correlations)
{
    internal_resource_id_to_resource_name.clear();
    resource_identifier_implicit.clear();

    // Process events.
    for (auto it = tokens.begin(); it != tokens.end(); ++it)
    {
        switch ((*it).second.token_type)
        {
        case kResourceCreate:
            ProcessResourceCreateToken((*it).second);
            break;

        case kResourceCorrelation:
            ProcessResourceCorrelationToken((*it).second);
            break;

        case kResourceName:
            ProcessResourceNameToken((*it).second, any_correlations);
            break;

        case kResourceImplicit:
            ProcessImplicitResourceToken((*it).second, any_correlations);
            break;

        default:
            break;
        }
    }

    correlation_id_to_resource_hash.clear();
    resource_hash_to_correlation_id.clear();
    resource_hash_to_internal_resource_id.clear();
    internal_resource_id_to_resource_hash_id.clear();
    correlation_id_to_resource_name.clear();
    internal_resource_ids_needing_correlation.clear();
    tokens.clear();

    // Clear the flag that indicates there are Name UserData tokens waiting to be processed.
    resource_name_token_tracked = false;

    return kRmtOk;
}

RmtErrorCode RmtResourceUserdataTrackResourceCreateToken(const RmtResourceIdentifier driver_resource_id,
                                                         const RmtResourceIdentifier internal_resource_id,
                                                         const RmtResourceType       resource_type,
                                                         const RmtTimestamp          timestamp)
{
    if (resource_type == kRmtResourceTypeBuffer || resource_type == kRmtResourceTypeImage || resource_type == kRmtResourceTypeHeap)
    {
        // Insert the token data in the tokens map.  Resource Create tokens are guaranteed to arrive before correlation tokens.
        TokenData token_data            = {};
        token_data.token_type           = kResourceCreate;
        token_data.resource_type        = resource_type;
        token_data.driver_resource_id   = driver_resource_id;
        token_data.internal_resource_id = internal_resource_id;
        tokens.insert(std::make_pair(timestamp, token_data));
    }

    return kRmtOk;
}

RmtErrorCode RmtResourceUserdataTrackResourceDestroyToken(const RmtResourceIdentifier internal_resource_id, const RmtTimestamp timestamp)
{
    TokenData token_data            = {};
    token_data.token_type           = kResourceDestroy;
    token_data.internal_resource_id = internal_resource_id;
    tokens.insert(std::make_pair(timestamp, token_data));

    return kRmtOk;
}

RmtErrorCode RmtResourceUserdataTrackResourceCorrelationToken(const RmtResourceIdentifier    driver_resource_id,
                                                              const RmtCorrelationIdentifier correlation_id,
                                                              const RmtTimestamp             timestamp)
{
    RMT_RETURN_ON_ERROR(driver_resource_id != kUnknownDriverResourceId, kRmtErrorMalformedData);
    RMT_RETURN_ON_ERROR(correlation_id != kUnknownCorrelationId, kRmtErrorMalformedData);

    TokenData token_data          = {};
    token_data.token_type         = kResourceCorrelation;
    token_data.driver_resource_id = driver_resource_id;
    token_data.correlation_id     = correlation_id;
    tokens.insert(std::make_pair(timestamp, token_data));

    return kRmtOk;
}

RmtErrorCode RmtResourceUserdataTrackResourceNameToken(const RmtCorrelationIdentifier resource_name_id,
                                                       const char*                    resource_name,
                                                       const RmtTimestamp             timestamp,
                                                       const RmtTimestamp             delay_time)
{
    RMT_RETURN_ON_ERROR(resource_name != nullptr, kRmtErrorInvalidPointer);
    RMT_RETURN_ON_ERROR(resource_name_id != kUnknownDriverResourceId, kRmtErrorMalformedData);
    RmtErrorCode result = kRmtErrorMalformedData;

    if (timestamp >= delay_time)
    {
        // The first time a Name UserData token is tracked when loading
        // a memory trace file, the memory pool for resource name strings
        // needs to be reset.  This flag is checked when the first Name
        // UserData token is tracked and, if false, the cache of resource
        // name strings is cleared.  The flag is then set to true. Once
        // all tokens have been tracked and processed, the flag is set
        // to false so that it is ready when the next memory trace file
        // is loaded.
        if (!resource_name_token_tracked)
        {
            resource_name_string_pool.FreeAll();
            resource_name_token_tracked = true;
        }

        TokenData token_data      = {};
        token_data.token_type     = kResourceName;
        token_data.correlation_id = resource_name_id;
        size_t name_length        = strlen(resource_name) + 1;
        result                    = resource_name_string_pool.Allocate(name_length, &token_data.resource_name);
        RMT_ASSERT(result == kRmtOk);
        if (result == kRmtOk)
        {
            memcpy(token_data.resource_name, resource_name, name_length);
            tokens.insert(std::make_pair(timestamp - delay_time, token_data));
            result = kRmtOk;
        }
    }

    return result;
}

RmtErrorCode RmtResourceUserdataTrackImplicitResourceToken(RmtResourceIdentifier correlation_id, uint64_t timestamp, uint64_t delay_time)
{
    if (timestamp - delay_time <= timestamp)
    {
        TokenData token_data      = {};
        token_data.token_type     = kResourceImplicit;
        token_data.correlation_id = correlation_id;
        tokens.insert(std::make_pair(timestamp - delay_time, token_data));
        return kRmtOk;
    }
    return kRmtErrorMalformedData;
}

RmtErrorCode RmtResourceUserdataGetResourceName(const RmtResourceIdentifier resource_id, const char** out_resource_name)
{
    RMT_RETURN_ON_ERROR(out_resource_name != nullptr, kRmtErrorInvalidPointer);

    RmtErrorCode result = kRmtOk;

    const auto& resource_name_iterator = internal_resource_id_to_resource_name.find(resource_id);
    if (resource_name_iterator != internal_resource_id_to_resource_name.end())
    {
        *out_resource_name = (*resource_name_iterator).second;
    }
    else
    {
        *out_resource_name = nullptr;
        result             = kRmtErrorNoResourceFound;
    }

    return result;
}

RmtErrorCode RmtResourceUserdataUpdateResourceName(const RmtResourceList* resource_list, const RmtResourceIdentifier internal_resource_id)
{
    RmtResource* found_resource = NULL;
    RmtErrorCode result         = RmtResourceListGetResourceByResourceId(resource_list, internal_resource_id, (const RmtResource**)&found_resource);
    if (result == kRmtOk)
    {
        const char* resource_name = nullptr;
        if (RmtResourceUserdataGetResourceName(internal_resource_id, &resource_name) == kRmtOk)
        {
            found_resource->name = resource_name;
        }
    }

    return result;
}

bool RmtResourceUserDataIsResourceImplicit(const RmtResourceIdentifier resource_id)
{
    if (resource_identifier_implicit.find(resource_id) != resource_identifier_implicit.end())
    {
        return true;
    }
    return false;
}
