//=============================================================================
// Copyright (c) 2018-2024 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Header for the XML settings writer.
//=============================================================================

#ifndef RMV_SETTINGS_SETTINGS_WRITER_H_
#define RMV_SETTINGS_SETTINGS_WRITER_H_

#include <QIODevice>
#include <QXmlStreamWriter>

#include "settings/rmv_settings.h"

namespace rmv
{
    /// @brief Support for the XML settings writer.
    class SettingsWriter
    {
    public:
        /// @brief Constructor.
        ///
        /// @param [in] settings Output settings class.
        explicit SettingsWriter(RMVSettings* settings);

        /// @brief Destructor.
        ~SettingsWriter();

        /// @brief Begin writing the XML file and make sure it's valid.
        ///
        /// @param [in] device The XML file represented by a Qt IO device.
        bool Write(QIODevice* device);

    private:
        /// @brief Write global settings and recently used files section.
        void WriteSettingsAndRecents();

        /// @brief Write settings list.
        void WriteSettings();

        /// @brief Write individual settings.
        ///
        /// @param [in] setting The RMVSetting structure to write out.
        void WriteSetting(const RMVSetting& setting);

        /// @brief Write recently opened file list.
        void WriteRecentFiles();

        /// @brief Write individual recently opened files.
        ///
        /// @param [in] recent_file The name of the file to write.
        void WriteRecentFile(const RecentFileData& recent_file);

        QXmlStreamWriter writer_;    ///< Qt's XML stream.
        RMVSettings*     settings_;  ///< Belongs to the caller, not this class.
    };
}  // namespace rmv

#endif  // RMV_SETTINGS_SETTINGS_WRITER_H_
