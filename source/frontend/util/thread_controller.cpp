//=============================================================================
// Copyright (c) 2019-2021 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Implementation of a thread controller.
//=============================================================================

#include "util/thread_controller.h"

#include "rmt_util.h"

#include "managers/load_animation_manager.h"

namespace rmv
{
    BackgroundTask::BackgroundTask()
    {
    }

    BackgroundTask::~BackgroundTask()
    {
    }

    void BackgroundTask::Start()
    {
        ThreadFunc();
        emit WorkerFinished();
    }

    ThreadController::ThreadController(QWidget* parent, BackgroundTask* background_task)
        : background_task_(background_task)
        , finished_(false)
    {
        // Start the loading animation.
        if (parent == nullptr)
        {
            rmv::LoadAnimationManager::Get().StartAnimation();
        }
        else
        {
            rmv::LoadAnimationManager::Get().StartAnimation(parent, 0);
        }

        // Create the thread. It will be setup to be deleted below when the thread has
        // finished (it emits a QThread::Finished signal).
        thread_ = new QThread();
        background_task_->moveToThread(thread_);

        connect(thread_, &QThread::started, background_task_, &BackgroundTask::Start);
        connect(background_task_, &BackgroundTask::WorkerFinished, thread_, &QThread::quit);
        connect(background_task_, &BackgroundTask::WorkerFinished, this, &ThreadController::WorkerFinished);

        // set up signal connections to automatically delete thread_ and background_task_ when work is done:
        connect(background_task_, &BackgroundTask::WorkerFinished, background_task_, &BackgroundTask::deleteLater);
        connect(thread_, &QThread::finished, thread_, &QThread::deleteLater);

        thread_->start();
    }

    ThreadController::~ThreadController()
    {
    }

    void ThreadController::WorkerFinished()
    {
        rmv::LoadAnimationManager::Get().StopAnimation();
        finished_ = true;
        emit ThreadFinished();
    }

    bool ThreadController::Finished() const
    {
        return finished_;
    }

}  // namespace rmv
