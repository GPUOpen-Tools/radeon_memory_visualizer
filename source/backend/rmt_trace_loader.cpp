//=============================================================================
// Copyright (c) 2022-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Trace loader structures and functions.
//=============================================================================

#include "rmt_trace_loader.h"
#include "rmt_data_snapshot.h"

#ifdef _LINUX
#include <dirent.h>
#include <fstream>
#include <string>
#include <sys/file.h>
#include <sys/stat.h>
#include <thread>
#include <unistd.h>
#include <vector>
#endif  // #ifdef _LINUX

/// The one and only instance of the data set, which is initialized when loading in
/// a trace file.
RmtDataSet      data_set_ = {};  ///< The dataset read from file.
RmtDataTimeline timeline_ = {};  ///< The timeline.

RmtDataSet* RmtTraceLoaderGetDataSet()
{
    return &data_set_;
}

RmtDataTimeline* RmtTraceLoaderGetTimeline()
{
    return &timeline_;
}

bool RmtTraceLoaderDataSetValid()
{
    // The data set is considered valid if the file handle is non-null (for legacy traces) or the RDF trace flag is set.
    if ((data_set_.file_handle != nullptr) || (data_set_.flags.is_rdf_trace))
    {
        return true;
    }
    return false;
}

RmtErrorCode RmtTraceLoaderTraceLoad(const char* trace_file_name, RmtDataSetErrorReportFunc reporter_function)
{
    // Loading regular binary data.
    RmtErrorCode error_code = RmtDataSetInitialize(trace_file_name, &data_set_);
    if (error_code != kRmtOk)
    {
        memset(&data_set_, 0, sizeof(RmtDataSet));
        RmtTokenClearPayloadCaches();
        return error_code;
    }

    // Set the error reporter callback function.
    RmtDataSetSetErrorReporter(&data_set_, reporter_function);

    // Create the default timeline for the data set.
    error_code = RmtDataSetGenerateTimeline(&data_set_, kRmtDataTimelineTypeVirtualMemory, &timeline_);
    return error_code;
}

void RmtTraceLoaderClearTrace()
{
    if (RmtTraceLoaderDataSetValid())
    {
        // Clean up any cached snapshots.
        for (int32_t current_snapshot_point_index = 0; current_snapshot_point_index < data_set_.snapshot_count; ++current_snapshot_point_index)
        {
            RmtSnapshotPoint* current_snapshot_point = &data_set_.snapshots[current_snapshot_point_index];
            if (current_snapshot_point->cached_snapshot)
            {
                RmtDataSnapshotDestroy(current_snapshot_point->cached_snapshot);
                delete current_snapshot_point->cached_snapshot;
            }
        }

        RmtDataTimelineDestroy(&timeline_);
        RmtDataSetDestroy(&data_set_);
    }
    memset(&data_set_, 0, sizeof(RmtDataSet));
    RmtTokenClearPayloadCaches();
}

RmtSnapshotPoint* RmtTraceLoaderGetSnapshotPoint(int32_t index)
{
    return &data_set_.snapshots[index];
}

int32_t RmtTraceLoaderGetSnapshotCount()
{
    return data_set_.snapshot_count;
}

#ifdef _LINUX
bool IsFileInUse(const std::vector<pid_t>& process_ids, const char* file_path_to_check)
{
    bool result = false;
    char file_link[PATH_MAX];

    for (auto pid : process_ids)
    {
        std::string proc_file_dir = "/proc/";
        proc_file_dir += std::to_string(pid);
        proc_file_dir += "/fd";
        DIR* proc_dir = opendir(proc_file_dir.c_str());
        if (proc_dir != nullptr)
        {
            struct dirent* proc_dir_entry = nullptr;
            while ((proc_dir_entry = readdir(proc_dir)))
            {
                if (proc_dir_entry->d_name[0] == '.')
                {
                    continue;  // skip "." and ".." file names.
                }

                std::string proc_file_path;
                proc_file_path = proc_file_dir;
                proc_file_path += "/";
                proc_file_path += proc_dir_entry->d_name;
                int file_link_len = readlink(proc_file_path.c_str(), file_link, sizeof(file_link) - 1);
                if (file_link_len < 1)
                {
                    // Skip empty filenames.
                    continue;
                }

                file_link[file_link_len] = 0;
                if (strcmp(file_path_to_check, file_link) == 0)
                {
                    result = true;
                    break;
                }
            }

            if (result)
            {
                break;
            }
            closedir(proc_dir);
        }
    }

    return result;
}

bool GetProcessIdList(std::vector<pid_t>& process_ids)
{
    bool result = true;
    DIR* dir    = opendir("/proc");
    if (dir != nullptr)
    {
        struct dirent* entry = readdir(dir);
        if (entry != nullptr)
        {
            const pid_t current_pid = ::getpid();
            do
            {
                const std::string dir_name = entry->d_name;

                // Only include PID entries.
                if (dir_name.find_first_not_of("0123456789") == std::string::npos)
                {
                    const std::string status = "/proc/" + dir_name + "/status";
                    std::ifstream     input(status.c_str());

                    std::string name_line;
                    std::getline(input, name_line);
                    size_t name_pos = name_line.find_first_of('\t') + 1;
                    name_line       = name_line.substr(name_pos);

                    char* end_of_string;
                    pid_t pid = std::strtol(dir_name.c_str(), &end_of_string, 10);
                    if (pid != current_pid)
                    {
                        // Store the PID unless it matches the PID of this process.
                        process_ids.push_back(pid);
                    }
                    else
                    {
                        end_of_string[0] = '\0';
                    }
                }
            } while ((entry = readdir(dir)) != nullptr);
            closedir(dir);
        }
        else
        {
            result = false;
        }
    }

    return result;
}

bool RmtTraceLoaderIsTraceAlreadyInUse(const char* trace_file_path)
{
    bool               result = false;
    std::vector<pid_t> process_ids;
    if (GetProcessIdList(process_ids))
    {
        char* full_path = nullptr;
        full_path       = realpath(trace_file_path, nullptr);
        if (full_path != nullptr)
        {
            result = IsFileInUse(process_ids, full_path);
            free(full_path);
            full_path = nullptr;
        }
    }

    return result;
}
#endif  // #ifdef _LINUX
