//=============================================================================
// Copyright (c) 2019-2021 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Implementation for a job system to run work on multiple threads.
//=============================================================================

#include "rmt_job_system.h"
#include "rmt_platform.h"
#include "rmt_assert.h"
#include <string.h>  // for memset()

// flag to signal worker thread should terminate
#define WORKER_THREAD_FLAGS_TERMINATE (1 << 0)

// job system main function
static uint32_t RMT_THREAD_FUNC JobSystemThreadFunc(void* input_data)
{
    RMT_ASSERT_MESSAGE(input_data, "No thread id address passed to thread func.");

    // read the thread input structure
    RmtJobQueueWorkerThreadInput* thread_input = (RmtJobQueueWorkerThreadInput*)input_data;

    if (thread_input == NULL)
    {
        return 0;
    }

    // run until the thread terminate signal is set
    while (true)
    {
        // sleep the thread until work is available
        RmtThreadEventWait(&thread_input->job_queue->signal);

        // check if we should quit
        const uint64_t flags = RmtThreadAtomicRead((volatile uint64_t*)&thread_input->flags);

        if ((flags & WORKER_THREAD_FLAGS_TERMINATE) == WORKER_THREAD_FLAGS_TERMINATE)
        {
            break;
        }

        // acquire the mutex.
        const RmtErrorCode error_code = RmtMutexLock(&thread_input->job_queue->queue_mutex);
        RMT_ASSERT(error_code == kRmtOk);

        // if there is no work then reset the signal and release the mutex
        if (thread_input->job_queue->queue_size == 0)
        {
            RmtThreadEventReset(&thread_input->job_queue->signal);
            RmtMutexUnlock(&thread_input->job_queue->queue_mutex);
        }
        else
        {
            // there is some work in the queue grab the job from the head
            const int32_t job_index = thread_input->job_queue->queue_head_index;

            RmtJobQueueJob*   current_job    = &thread_input->job_queue->queue_items[job_index];
            const int32_t     index          = current_job->run_count++;
            const int32_t     adjusted_index = current_job->base_index + index;
            void*             job_input      = current_job->input;
            RmtJobFunction    func           = current_job->function;
            volatile int64_t* addr           = &current_job->completed_count;

            // if we ran the last instance, then remove it from the queue
            if (current_job->run_count == current_job->count)
            {
                thread_input->job_queue->queue_head_index = (thread_input->job_queue->queue_head_index + 1) % RMT_MAXIMUM_JOB_COUNT;
                thread_input->job_queue->queue_size--;
            }

            // release queue access
            RmtMutexUnlock(&thread_input->job_queue->queue_mutex);

            // run the job
            (*func)(thread_input->thread_id, adjusted_index, job_input);

            // signal that it is done.
            RmtThreadAtomicAdd64(addr, 1);
        }
    }

    return 0;
}

// initialize the job queue
RmtErrorCode RmtJobQueueInitialize(RmtJobQueue* job_queue, int32_t worker_thread_count)
{
    RMT_ASSERT_MESSAGE(job_queue, "Parameter jobQueue is NULL.");
    RMT_RETURN_ON_ERROR(job_queue, kRmtErrorInvalidPointer);
    RMT_RETURN_ON_ERROR(worker_thread_count > 0 && worker_thread_count <= RMT_MAXIMUM_WORKER_THREADS, kRmtErrorIndexOutOfRange);

    // stash anything we need in the structure
    job_queue->worker_thread_count = worker_thread_count;
    job_queue->handle_next         = 0;

    // clear the queue contents
    memset(job_queue->queue_items, 0, sizeof(job_queue->queue_items));

    // create the event to signal when queue has work
    RmtErrorCode error_code = kRmtOk;
    error_code              = RmtThreadEventCreate(&job_queue->signal, false, true, "");
    RMT_RETURN_ON_ERROR(error_code == kRmtOk, error_code);

    // create the mutex for altering the job queue
    error_code = RmtMutexCreate(&job_queue->queue_mutex, "RMT Job Queue Mutex");
    RMT_RETURN_ON_ERROR(error_code == kRmtOk, error_code);

    // set up the queue
    job_queue->queue_tail_index = 0;
    job_queue->queue_head_index = 0;
    job_queue->queue_size       = 0;

    // create our worker threads.
    for (int32_t current_worker_thread_index = 0; current_worker_thread_index < worker_thread_count; ++current_worker_thread_index)
    {
        // set up the inputs
        RmtJobQueueWorkerThreadInput* input = &job_queue->worker_thread_inputs[current_worker_thread_index];
        input->thread_id                    = current_worker_thread_index;
        input->job_queue                    = job_queue;
        input->flags                        = 0;

        // create the thread
        error_code = RmtThreadCreate(&job_queue->worker_threads[current_worker_thread_index], JobSystemThreadFunc, input);

        RMT_RETURN_ON_ERROR(error_code == kRmtOk, error_code);
    }

    return kRmtOk;
}

// shutdown the job queue
RmtErrorCode RmtJobQueueShutdown(RmtJobQueue* job_queue)
{
    RMT_ASSERT_MESSAGE(job_queue, "Parameter jobQueue is NULL.");
    RMT_RETURN_ON_ERROR(job_queue, kRmtErrorInvalidPointer);

    // singal to terminate all the threads
    for (int32_t current_worker_thread_index = 0; current_worker_thread_index < job_queue->worker_thread_count; ++current_worker_thread_index)
    {
        RmtJobQueueWorkerThreadInput* input = &job_queue->worker_thread_inputs[current_worker_thread_index];
        RmtThreadAtomicOr((volatile uint64_t*)&input->flags, WORKER_THREAD_FLAGS_TERMINATE);
    }

    // signal all threads that there is work
    RmtThreadEventSignal(&job_queue->signal);

    // wait for all the threads to finish
    for (int32_t current_worker_thread_index = 0; current_worker_thread_index < job_queue->worker_thread_count; ++current_worker_thread_index)
    {
        const RmtErrorCode error_code = RmtThreadWaitForExit(&job_queue->worker_threads[current_worker_thread_index]);
        RMT_ASSERT(error_code == kRmtOk);
    }

    RmtThreadEventDestroy(&job_queue->signal);
    RmtMutexDestroy(&job_queue->queue_mutex);

    return kRmtOk;
}

// add a single job the queue
RmtErrorCode RmtJobQueueAddSingle(RmtJobQueue* job_queue, RmtJobFunction func, void* input, RmtJobHandle* out_handle)
{
    return RmtJobQueueAddMultiple(job_queue, func, input, 0, 1, out_handle);
}

// add a job to the queue
RmtErrorCode RmtJobQueueAddMultiple(RmtJobQueue* job_queue, RmtJobFunction func, void* input, int32_t base_index, int32_t count, RmtJobHandle* out_handle)
{
    RMT_ASSERT_MESSAGE(job_queue, "Parameter jobQueue is NULL.");
    RMT_ASSERT_MESSAGE(func, "Parameter func is NULL.");
    RMT_RETURN_ON_ERROR(job_queue, kRmtErrorInvalidPointer);
    RMT_RETURN_ON_ERROR(func, kRmtErrorInvalidPointer);
    RMT_RETURN_ON_ERROR(count, kRmtErrorInvalidSize);

    // add work to the queue.
    const RmtErrorCode error_code = RmtMutexLock(&job_queue->queue_mutex);
    RMT_ASSERT(error_code == kRmtOk);

    // check the size of the queue
    if (job_queue->queue_size == RMT_MAXIMUM_JOB_COUNT)
    {
        RmtMutexUnlock(&job_queue->queue_mutex);
        return kRmtErrorOutOfMemory;
    }

    // work out the handle
    const RmtJobHandle job_handle = job_queue->handle_next++;

    // block until this item in the queue is available
    RmtJobQueueWaitForCompletion(job_queue, job_handle);

    // work out the location in the queue and write the data
    const int32_t job_index                           = job_queue->queue_tail_index;
    job_queue->queue_items[job_index].function        = func;
    job_queue->queue_items[job_index].input           = input;
    job_queue->queue_items[job_index].base_index      = base_index;
    job_queue->queue_items[job_index].count           = count;
    job_queue->queue_items[job_index].run_count       = 0;
    job_queue->queue_items[job_index].handle          = job_handle;
    job_queue->queue_items[job_index].completed_count = 0;

    // advance the tail and size
    job_queue->queue_tail_index = (job_queue->queue_tail_index + 1) % RMT_MAXIMUM_JOB_COUNT;
    job_queue->queue_size++;

    RmtMutexUnlock(&job_queue->queue_mutex);

    // signal all threads that there is work to do
    RmtThreadEventSignal(&job_queue->signal);

    // write the handle back if the pointer is not NULL
    if (out_handle != nullptr)
    {
        *out_handle = job_handle;
    }

    return kRmtOk;
}

// check if a job has completed
RmtErrorCode RmtJobQueueWaitForCompletion(RmtJobQueue* job_queue, RmtJobHandle handle)
{
    RMT_ASSERT_MESSAGE(job_queue, "Parameter jobQueue is NULL.");
    RMT_RETURN_ON_ERROR(job_queue, kRmtErrorInvalidPointer);

    do
    {
        const uint64_t completed_jobs = RmtThreadAtomicRead((volatile uint64_t*)&job_queue->queue_items[handle % RMT_MAXIMUM_JOB_COUNT].completed_count);

        if ((int32_t)completed_jobs == job_queue->queue_items[handle % RMT_MAXIMUM_JOB_COUNT].count)
        {
            break;
        }

        // sleep for 1ms to reduce spin-lock.
        RmtSleep(1);

    } while (true);

    return kRmtOk;
}
