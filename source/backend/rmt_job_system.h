//=============================================================================
// Copyright (c) 2019-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  A job system to run work on multiple threads.
//=============================================================================

#ifndef RMV_BACKEND_RMT_JOB_SYSTEM_H_
#define RMV_BACKEND_RMT_JOB_SYSTEM_H_

#include "rmt_atomic.h"
#include "rmt_error.h"
#include "rmt_mutex.h"
#include "rmt_thread.h"
#include "rmt_thread_event.h"

/// The maximum number of worker threads that <c><i>RmtJobQueue</i></c> supports.
#define RMT_MAXIMUM_WORKER_THREADS (24)

/// The maximum number of jobs that can be queued up.
#define RMT_MAXIMUM_JOB_COUNT (1024)

#ifdef __cplusplus
extern "C" {
#endif  // #ifdef __cplusplus

typedef struct RmtJobQueue RmtJobQueue;

/// The prototype for a job function.
///
/// This is the form that all functions should take that are used with the job
/// system. When using <c><i>RmtJobQueueAddSingle</i></c> or
/// <c><i>RmtJobQueueAddMultiple</i></c> your function will be called on a
/// worker thread asynchronously from the main thread.
///
/// @param [in] thread_id                   The unique identifier of the worker thread running this function.
/// @param [in] index                       The index of the job, only applicable when using <c><i>RmtJobQueueAddMultiple</i></c>.
/// @param [in] input                       The user data provided to your call to <c><i>RmtJobQueueAddSingle</i></c> or <c><i>RmtJobQueueAddMultiple</i></c>.
///
typedef void (*RmtJobFunction)(int32_t thread_id, int32_t index, void* input);

/// Input structures for the job queue worker threads.
typedef struct RmtJobQueueWorkerThreadInput
{
    RmtJobQueue* job_queue;  ///< Pointer to the job queue that owns the thread.
    uint64_t     flags;      ///< Flags to control execution of worker thread.
    uint32_t     thread_id;  ///< The id assigned to this worker thread.
} RmtJobQueueWorkerThreadInput;

/// A type to represent a handle to a job.
///
/// Handles can be optionally retrieved from <c><i>RmtJobQueueAddSingle</i></c>
/// or <c><i>RmtJobQueueAddMultiple</i></c>. They can then be passed to
/// <c><i>RmtJobQueueIsCompleted</i></c> to check if a job has completed.
///
typedef uint64_t RmtJobHandle;

/// A structure encapsulating a single job and it's input.
typedef struct RmtJobQueueJob
{
    RmtJobFunction function;
    void*          input;
    RmtJobHandle   handle;
    int32_t        run_count;
    int32_t        base_index;
    int32_t        count;
    int64_t        completed_count;
} RmtJobQueueJob;

/// A structure encapsulating the state of the job system.
typedef struct RmtJobQueue
{
    RmtThread                    worker_threads[RMT_MAXIMUM_WORKER_THREADS];        ///< An array of threads where the jobs may run.
    RmtJobQueueWorkerThreadInput worker_thread_inputs[RMT_MAXIMUM_WORKER_THREADS];  ///< A queue of inputs to the jobs.
    int32_t                      worker_thread_count;                               ///< The total number of worker threads in use.
    RmtThreadEvent               signal;                                            ///< Signal when there is work in the global queue.

    RmtJobQueueJob queue_items[RMT_MAXIMUM_JOB_COUNT];  ///< A queue of jobs.
    int32_t        queue_head_index;                    ///< The head index of the queue.
    int32_t        queue_tail_index;                    ///< The tail index of the queue.
    int32_t        queue_size;                          ///< The current size of the queue.
    RmtMutex       queue_mutex;                         ///< A mutex controlling queue access.

    RmtJobHandle handle_next;  ///< The next handle to use.
} RmtJobQueue;

/// Initialize the job queue for the specified number of threads.
///
/// @param [in,out] job_queue                       A pointer to the <c><i>RmtJobSystem</i></c> structure to initialize.
/// @param [in]     worker_thread_count             The number of worker threads to create.
///
/// @retval
/// kRmtOk                                      The operation completed successfully.
/// @retval
/// kRmtErrorInvalidPointer                     The operation failed because <c><i>jobQueue</i></c> was <c><i>NULL</i></c>.
/// @retval
/// kRmtErrorIndexOutOfRange                    The operation failed because <c><i>workerThreadCount</i></c> was not in range [0..<c><i>RMT_MAXIMUM_WORKER_THREADS</i></c>].
///
RmtErrorCode RmtJobQueueInitialize(RmtJobQueue* job_queue, int32_t worker_thread_count);

/// Shutdown the job queue.
///
/// @param [in,out] job_queue                       A pointer to the <c><i>RmtJobSystem</i></c> structure to shutdown.
///
/// @retval
/// kRmtOk                                      The operation completed successfully.
/// @retval
/// kRmtErrorInvalidPointer                     The operation failed because <c><i>jobQueue</i></c> was <c><i>NULL</i></c>.
///
RmtErrorCode RmtJobQueueShutdown(RmtJobQueue* job_queue);

/// Add a single job to the queue.
///
/// @param [in,out] job_queue                       A pointer to the <c><i>RmtJobSystem</i></c> structure.
/// @param [in]     func                            A pointer to the function containing your job.
/// @param [in]     input                           A pointer to some user data to pass to <c><i>func</i></c>. See <c><i>RmtJobFunction</i></c>.
/// @param [out]    out_handle                      An optional pointer to a <c><i>RmtJobHandle</i></c>.
///
/// @retval
/// kRmtOk                                      The operation completed successfully.
/// @retval
/// kRmtErrorInvalidPointer                     The operation failed because <c><i>jobQueue</i></c> was <c><i>NULL</i></c>.
/// @retval
/// kRmtErrorOutOfMemory                        The operation failed because there was no space left in the queue.
///
RmtErrorCode RmtJobQueueAddSingle(RmtJobQueue* job_queue, RmtJobFunction func, void* input, RmtJobHandle* out_handle);

/// Add multiple copies of the same job to the queue.
///
/// This is similar to a parallel for. The <c><i>func</i></c> provided will be
/// called <c><i>count</i></c> times. With the index being passed to
/// <c><i>func</i></c> being calculated as i + <c><i>baseIndex</i></c>. This is
/// a much more space-efficient way of adding multiple copies of the same job
/// to the queue. It takes a single queue slot, rather than n.
///
/// Conceptually you can think of it as the equivalent to an async. version of a
/// for loop, see the following pseduo code:
///
/// for(int32_t index=baseIndex; index<(baseIndex+count); ++index) {
///     func(thread_id, index, input);
/// }
///
/// @param [in,out] job_queue                       A pointer to the <c><i>RmtJobSystem</i></c> structure.
/// @param [in]     func                            A pointer to the function containing your job.
/// @param [in]     input                           A pointer to some user data to pass to <c><i>func</i></c>. See <c><i>RmtJobFunction</i></c>.
/// @param [in]     base_index                      The start index of the job.
/// @param [in]     count                           The total number of copies of the job to execute.
/// @param [out]    out_handle                      An optional pointer to a <c><i>RmtJobHandle</i></c>.
///
/// @retval
/// kRmtOk                                      The operation completed successfully.
/// @retval
/// kRmtErrorInvalidPointer                     The operation failed because <c><i>jobQueue</i></c> was <c><i>NULL</i></c>.
/// @retval
/// kRmtErrorOutOfMemory                        The operation failed because there was no space left in the queue.
///
RmtErrorCode RmtJobQueueAddMultiple(RmtJobQueue* job_queue, RmtJobFunction func, void* input, int32_t base_index, int32_t count, RmtJobHandle* out_handle);

/// Wait for a job handle to complete.
///
/// @param [in]     job_queue                       A pointer to the <c><i>RmtJobSystem</i></c> structure.
/// @param [in]     handle                          A handle to the job that was added using <c><i>RmtJobQueueAddSingle</i></c> or <c><i>RmtJobQueueAddMultiple</i></c>.
///
/// @retval
/// kRmtOk                                      The operation completed successfully.
/// @retval
/// kRmtErrorInvalidPointer                     The operation failed because <c><i>jobQueue</i></c> was <c><i>NULL</i></c>.
///
RmtErrorCode RmtJobQueueWaitForCompletion(RmtJobQueue* job_queue, RmtJobHandle handle);

#ifdef __cplusplus
}
#endif  // #ifdef __cplusplus
#endif  // #ifndef RMV_BACKEND_RMT_JOB_SYSTEM_H_
