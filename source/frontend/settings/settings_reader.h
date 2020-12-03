//=============================================================================
/// Copyright (c) 2018-2020 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Header for RMV XML settings reader.
//=============================================================================

#ifndef RMV_SETTINGS_SETTINGS_READER_H_
#define RMV_SETTINGS_SETTINGS_READER_H_

#include <QIODevice>
#include <QXmlStreamReader>

#include "settings/rmv_settings.h"

namespace rmv
{
    /// Support for RMV XML settings reader.
    class SettingsReader
    {
    public:
        /// Constructor.
        /// \param settings Output settings class.
        explicit SettingsReader(RMVSettings* settings);

        /// Destructor.
        ~SettingsReader();

        /// Begin reading RMV XML file and make sure it's valid.
        /// \param device The XML file represented by a Qt IO device.
        bool Read(QIODevice* device);

    private:
        /// Detect global settings and recently used files section.
        void ReadSettingsAndRecents();

        /// Detect setting list.
        void ReadSettings();

        /// Read individual settings.
        void ReadSetting();

        /// Detect recently opened file list.
        void ReadRecentFiles();

        /// Read individual recently opened files.
        void ReadRecentFile();

        QXmlStreamReader reader_;    ///< Qt's XML stream.
        RMVSettings*     settings_;  ///< Belongs to the caller, not this class.
    };
}  // namespace rmv

#endif  // RMV_SETTINGS_SETTINGS_READER_H_
