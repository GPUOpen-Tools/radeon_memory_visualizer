//=============================================================================
// Copyright (c) 2018-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Declaration for the log file writer.
//=============================================================================

#ifndef RMV_UTIL_LOG_FILE_WRITER_H_
#define RMV_UTIL_LOG_FILE_WRITER_H_

#include <QMutex>
#include <QString>

namespace rmv
{
    /// @brief Log file writer class definition.
    class LogFileWriter
    {
    public:
        /// @brief Log levels used by the logger from most severe to least severe.
        enum LogLevel
        {
            kError,
            kWarning,
            kInfo,
            kDebug,
        };

        /// @brief LogFileWriter instance get function.
        ///
        /// @return a reference to the LogFileWriter instance.
        static LogFileWriter& Get();

        /// @brief Write a string to the log file.
        ///
        /// @param [in] log_level The log level to use for this message.
        /// @param [in] log_message The message to write to the log file.
        void WriteLog(LogLevel log_level, const char* log_message, ...);

        /// @brief Get the location of the log file.
        ///
        /// @return The location of the log file.
        QString GetLogFileLocation();

    private:
        /// @brief Constructor.
        explicit LogFileWriter();

        /// @brief Destructor.
        ~LogFileWriter();

        /// @brief Write the log out to the log file.
        ///
        /// @param [in] log_message The message to write to the log file.
        void WriteLogMessage(const char* log_message);

        QMutex   mutex_;      ///< The mutex to write the log.
        LogLevel log_level_;  ///< The current log level.
    };
}  // namespace rmv

#endif  // RMV_UTIL_LOG_FILE_WRITER_H_
