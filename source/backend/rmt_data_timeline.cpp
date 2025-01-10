//=============================================================================
// Copyright (c) 2019-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Implementation of the data timeline functions.
//=============================================================================

#include "rmt_data_timeline.h"
#include "rmt_data_set.h"
#include "rmt_data_snapshot.h"
#include "rmt_file_format.h"
#include "rmt_job_system.h"
#include "rmt_assert.h"
#include <string.h>  // for memcpy()
#include <stdlib.h>  // for malloc(), free()

// Helper function to call the correct free function.
static void PerformFree(RmtDataSet* data_set, void* pointer)
{
    if (data_set->free_func == nullptr)
    {
        return free(pointer);
    }

    return (data_set->free_func)(pointer);
}

RmtErrorCode RmtDataTimelineDestroy(RmtDataTimeline* timeline)
{
    RMT_RETURN_ON_ERROR(timeline, kRmtErrorInvalidPointer);
    RMT_RETURN_ON_ERROR(timeline->data_set, kRmtErrorMalformedData);

    PerformFree(timeline->data_set, timeline->series_memory_buffer);
    return kRmtOk;
}

// input structure to the histogram job.
typedef struct HistogramJobInput
{
    RmtDataTimelineHistogram* out_timeline_histogram;  // histogram being generated.
    const RmtDataTimeline*    timeline;                // timeline being processed to form histogram.
    uint64_t                  start_timestamp;         // the start timestamp of the histogram.
    uint64_t                  end_timestamp;           // the end timestmap of the histogram.
    int64_t                   bucket_width_in_cycles;  // the width of each bucket in RMT cycles.
    int64_t                   bucket_count;            // the number of buckets in the histogram.
} HistogramJobInput;

// check the input parameters are good.
static bool ValidateInputParameters(HistogramJobInput* input_parameters)
{
    if (input_parameters == NULL)
    {
        return false;
    }

    if (input_parameters->timeline == NULL)
    {
        return false;
    }

    if (input_parameters->out_timeline_histogram == NULL)
    {
        return false;
    }

    return true;
}

// Job function to create histogram data for a single bucket from RMT mip-mapped data series in timeline.
static void CreateHistogramJob(int32_t thread_id, int32_t index, void* input)
{
    RMT_UNUSED(thread_id);

    // Get hold of the job input and validate it.
    HistogramJobInput* input_parameters = (HistogramJobInput*)input;
    const bool         is_input_valid   = ValidateInputParameters(input_parameters);
    RMT_ASSERT(is_input_valid == true);
    if (!is_input_valid)
    {
        return;
    }

    const uint64_t start_timestamp = input_parameters->start_timestamp + (input_parameters->bucket_width_in_cycles * index);
    const uint64_t end_timestamp   = start_timestamp + input_parameters->bucket_width_in_cycles;
    RMT_UNUSED(end_timestamp);
    int32_t       value_index = RmtDataSetGetSeriesIndexForTimestamp(NULL, start_timestamp);
    const int32_t level_index = 0;

    for (int32_t current_series_index = 0; current_series_index < input_parameters->timeline->series_count; ++current_series_index)
    {
        const int32_t value_count = input_parameters->timeline->series[current_series_index].levels[level_index].value_count;
        if (value_count == 0)
        {
            return;
        }

        if (value_index >= value_count)
        {
            value_index = value_count - 1;
        }

        if (value_index < 0)
        {
            value_index = 0;
        }

        const uint64_t current_value = input_parameters->timeline->series[current_series_index].levels[level_index].values[value_index];

        const int32_t bucket_index = RmtDataTimelineHistogramGetIndex(input_parameters->out_timeline_histogram, index, current_series_index);
        input_parameters->out_timeline_histogram->bucket_data[bucket_index] = current_value;
    }
}

// create the historgram.
RmtErrorCode RmtDataTimelineCreateHistogram(const RmtDataTimeline*    timeline,
                                            RmtJobQueue*              job_queue,
                                            int32_t                   bucket_count,
                                            uint64_t                  bucket_width_in_rmt_cycles,
                                            uint64_t                  start_timestamp,
                                            uint64_t                  end_timestamp,
                                            RmtDataTimelineHistogram* out_timeline_histogram)
{
    // Validate inputs are okay.
    RMT_ASSERT_MESSAGE(timeline, "Parameter timeline is NULL.");
    RMT_ASSERT_MESSAGE(job_queue, "Parameter jobQueue is NULL.");
    RMT_ASSERT_MESSAGE(bucket_width_in_rmt_cycles > 0, "Parameter bucketWidthInCycles must be larger than 0 cycles.");
    RMT_ASSERT_MESSAGE(out_timeline_histogram, "Parameter outTimelineHistogram is NULL.");

    // Handle generating error codes in release builds.
    RMT_RETURN_ON_ERROR(timeline, kRmtErrorInvalidPointer);
    RMT_RETURN_ON_ERROR(job_queue, kRmtErrorInvalidPointer);
    RMT_RETURN_ON_ERROR(bucket_width_in_rmt_cycles > 0, kRmtErrorInvalidSize);
    RMT_RETURN_ON_ERROR(out_timeline_histogram, kRmtErrorInvalidPointer);

    // This check is added to see that the time intervals provided by the input timestamp arguments
    // need to be larger than the time intervals provided by bucket arguments..
    const int64_t cycle_delta = (end_timestamp - start_timestamp) - (bucket_count * bucket_width_in_rmt_cycles);
    RMT_ASSERT_MESSAGE(cycle_delta >= 0, "The time delta is not correct.");
    RMT_RETURN_ON_ERROR(cycle_delta >= 0, kRmtErrorInvalidSize);

    // Fill out the initial fields of the data set.
    out_timeline_histogram->timeline               = timeline;
    out_timeline_histogram->bucket_width_in_cycles = bucket_width_in_rmt_cycles;
    out_timeline_histogram->bucket_count           = bucket_count;

    // Allocate memory for the data and prepare it.
    out_timeline_histogram->bucket_group_count = timeline->series_count;
    const int32_t size_in_bytes                = timeline->series_count * out_timeline_histogram->bucket_count * sizeof(uint64_t);
    out_timeline_histogram->bucket_data        = (uint64_t*)malloc(size_in_bytes);
    RMT_ASSERT_MESSAGE(out_timeline_histogram->bucket_data, "Failed to allocate timeline histogram.");
    RMT_RETURN_ON_ERROR(out_timeline_histogram->bucket_data, kRmtErrorOutOfMemory);
    memset(out_timeline_histogram->bucket_data, 0, size_in_bytes);

    // Setup the inputs to our job in the scratch memory.
    HistogramJobInput* input_parameters = (HistogramJobInput*)out_timeline_histogram->scratch_buffer;
    RMT_STATIC_ASSERT(sizeof(HistogramJobInput) <= sizeof(out_timeline_histogram->scratch_buffer));
    input_parameters->bucket_count           = out_timeline_histogram->bucket_count;
    input_parameters->bucket_width_in_cycles = out_timeline_histogram->bucket_width_in_cycles;
    input_parameters->end_timestamp          = end_timestamp;
    input_parameters->start_timestamp        = start_timestamp;
    input_parameters->out_timeline_histogram = out_timeline_histogram;
    input_parameters->timeline               = out_timeline_histogram->timeline;

    // Kick the jobs off to the worker threads.
    // NOTE: This can be done as one per bucket (or range of buckets).
    RmtJobHandle       job_handle = 0;
    const RmtErrorCode error_code = RmtJobQueueAddMultiple(job_queue, CreateHistogramJob, input_parameters, 0, bucket_count, &job_handle);
    RMT_ASSERT(error_code == kRmtOk);
    RMT_RETURN_ON_ERROR(error_code == kRmtOk, error_code);

    // Wait for job to complete.
    RmtJobQueueWaitForCompletion(job_queue, job_handle);

    return kRmtOk;
}

// destroy the histogram.
RmtErrorCode RmtDataTimelineHistogramDestroy(RmtDataTimelineHistogram* timeline_histogram)
{
    RMT_RETURN_ON_ERROR(timeline_histogram, kRmtErrorInvalidPointer);

    free(timeline_histogram->bucket_data);

    timeline_histogram->timeline               = NULL;
    timeline_histogram->bucket_data            = NULL;
    timeline_histogram->bucket_width_in_cycles = 0;
    timeline_histogram->bucket_count           = 0;
    timeline_histogram->bucket_group_count     = 0;
    return kRmtOk;
}

// get index from a bucket address.
int32_t RmtDataTimelineHistogramGetIndex(RmtDataTimelineHistogram* timeline_histogram, int32_t bucket_index, int32_t bucket_group_index)
{
    RMT_ASSERT_MESSAGE(timeline_histogram, "Parameter timelineHistogram is NULL.");
    RMT_ASSERT_MESSAGE(bucket_index < timeline_histogram->bucket_count, "bucketIndex is out of range, should be in range [0..bucketCount-1].");
    const int32_t index = (bucket_index * timeline_histogram->bucket_group_count) + bucket_group_index;
    return index;
}

// get the value from the bucket address.
int64_t RmtDataTimelineHistogramGetValue(RmtDataTimelineHistogram* timeline_histogram, int32_t bucket_index, int32_t bucket_group_index)
{
    RMT_ASSERT_MESSAGE(timeline_histogram, "Parameter timelineHistogram is NULL.");
    const int32_t index = RmtDataTimelineHistogramGetIndex(timeline_histogram, bucket_index, bucket_group_index);
    return timeline_histogram->bucket_data[index];
}
