//=============================================================================
// Copyright (c) 2018-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Header for the XML settings reader.
//=============================================================================

#ifndef RMV_SETTINGS_SETTINGS_READER_H_
#define RMV_SETTINGS_SETTINGS_READER_H_

#include <QIODevice>
#include <QXmlStreamReader>

#include "settings/rmv_settings.h"

namespace rmv
{
    /// @brief Support for the XML settings reader.
    class SettingsReader
    {
    public:
        /// @brief Constructor.
        ///
        /// @param [in] settings Output settings class.
        explicit SettingsReader(RMVSettings* settings);

        /// @brief Destructor.
        ~SettingsReader();

        /// @brief Begin reading the XML file and make sure it's valid.
        ///
        /// @param [in] device The XML file represented by a Qt IO device.
        bool Read(QIODevice* device);

    private:
        /// @brief Read global settings and recently used files section.
        void ReadSettingsAndRecents();

        /// @brief Read settings list.
        void ReadSettings();

        /// @brief Read individual settings.
        void ReadSetting();

        /// @brief Read recently opened file list.
        void ReadRecentFiles();

        /// @brief Read individual recently opened files.
        void ReadRecentFile();

        QXmlStreamReader reader_;    ///< Qt's XML stream.
        RMVSettings*     settings_;  ///< Belongs to the caller, not this class.
    };
}  // namespace rmv

#endif  // RMV_SETTINGS_SETTINGS_READER_H_
