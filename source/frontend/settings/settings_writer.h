//=============================================================================
/// Copyright (c) 2018-2020 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Header for RMV XML settings writer.
//=============================================================================

#ifndef RMV_SETTINGS_SETTINGS_WRITER_H_
#define RMV_SETTINGS_SETTINGS_WRITER_H_

#include <QIODevice>
#include <QXmlStreamWriter>

#include "settings/rmv_settings.h"

namespace rmv
{
    /// Support for RMV XML settings writer.
    class SettingsWriter
    {
    public:
        /// Constructor.
        /// \param pSettings Output settings class.
        explicit SettingsWriter(RMVSettings* settings);

        /// Destructor.
        ~SettingsWriter();

        /// Begin writing RMV XML file and make sure it's valid.
        /// \param device The XML file represented by a Qt IO device.
        bool Write(QIODevice* device);

    private:
        /// Detect global settings and recently used files section.
        void WriteSettingsAndRecents();

        /// Detect setting list.
        void WriteSettings();

        /// Write individual settings.
        /// \param setting The RMVSetting structure to write out.
        void WriteSetting(const RMVSetting& setting);

        /// Detect recently opened file list.
        void WriteRecentFiles();

        /// Write individual recently opened files.
        /// \param recentFile The name of the file to write.
        void WriteRecentFile(const RecentFileData& recent_file);

        QXmlStreamWriter writer_;    ///< Qt's XML stream.
        RMVSettings*     settings_;  ///< Belongs to the caller, not this class.
    };
}  // namespace rmv

#endif  // RMV_SETTINGS_SETTINGS_WRITER_H_
