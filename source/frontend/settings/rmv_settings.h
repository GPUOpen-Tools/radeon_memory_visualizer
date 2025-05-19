//=============================================================================
// Copyright (c) 2018-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Define the settings and information about recently opened traces.
//=============================================================================

#ifndef RMV_SETTINGS_RMV_SETTINGS_H_
#define RMV_SETTINGS_RMV_SETTINGS_H_

#include <QMap>
#include <QVector>

#include "qt_common/utils/color_palette.h"
#include "qt_common/utils/common_definitions.h"

#include "rmt_data_set.h"

#include "util/definitions.h"

/// @brief A struct for a setting key-value pair.
struct RMVSetting
{
    QString name;   ///< Name of the setting.
    QString value;  ///< Value of the setting.
};

/// @brief Enum of all settings.
enum RMVSettingID
{
    kSettingMainWindowGeometryData,
    kSettingMainWindowWidth,
    kSettingMainWindowHeight,
    kSettingMainWindowXpos,
    kSettingMainWindowYpos,

    kSettingLastFileOpenLocation,
    kSettingGeneralCheckForUpdatesOnStartup,
    kSettingGeneralTimeUnits,
    kSettingGeneralAllocUniquenessHeap,
    kSettingGeneralAllocUniquenessAllocation,
    kSettingGeneralAllocUniquenessOffset,
    kSettingGeneralDriverOverridesAllowNotifications,

    kSettingThemesAndColorsPalette,

    kSettingThemesAndColorsSnapshotViewed,
    kSettingThemesAndColorsSnapshotCompared,
    kSettingThemesAndColorsSnapshotLive,
    kSettingThemesAndColorsSnapshotGenerated,
    kSettingThemesAndColorsSnapshotVma,

    kSettingThemesAndColorsResourceDsBuffer,
    kSettingThemesAndColorsResourceRenderTarget,
    kSettingThemesAndColorsResourceTexture,
    kSettingThemesAndColorsResourceVertexBuffer,
    kSettingThemesAndColorsResourceIndexBuffer,
    kSettingThemesAndColorsResourceRayTracingBuffer,
    kSettingThemesAndColorsResourceUav,
    kSettingThemesAndColorsResourceShaderPipeline,
    kSettingThemesAndColorsResourceCommandBuffer,
    kSettingThemesAndColorsResourceHeap,
    kSettingThemesAndColorsResourceDescriptors,
    kSettingThemesAndColorsResourceBuffer,
    kSettingThemesAndColorsResourceGpuEvent,
    kSettingThemesAndColorsResourceFreeSpace,
    kSettingThemesAndColorsResourceInternal,

    kSettingThemesAndColorsColorThemeMode,

    kSettingThemesAndColorsDeltaIncrease,
    kSettingThemesAndColorsDeltaDecrease,
    kSettingThemesAndColorsDeltaNoChange,

    kSettingThemesAndColorsHeapLocal,
    kSettingThemesAndColorsHeapInvisible,
    kSettingThemesAndColorsHeapSystem,
    kSettingThemesAndColorsHeapUnspecified,

    kSettingThemesAndColorsCpuMapped,
    kSettingThemesAndColorsNotCpuMapped,

    kSettingThemesAndColorsInPreferredHeap,
    kSettingThemesAndColorsNotInPreferredHeap,

    kSettingThemesAndColorsAliased,
    kSettingThemesAndColorsNotAliased,

    kSettingThemesAndColorsResourceHistoryResourceEvent,
    kSettingThemesAndColorsResourceHistoryCpuMapUnmap,
    kSettingThemesAndColorsResourceHistoryResidencyUpdate,
    kSettingThemesAndColorsResourceHistoryPageTableUpdate,
    kSettingThemesAndColorsResourceHistoryHighlight,
    kSettingThemesAndColorsResourceHistorySnapshot,

    kSettingThemesAndColorsCommitTypeCommitted,
    kSettingThemesAndColorsCommitTypePlaced,
    kSettingThemesAndColorsCommitTypeVirtual,

    kSettingCount,
};

typedef QMap<RMVSettingID, RMVSetting> RMVSettingsMap;

namespace rmv
{
    /// @brief Support for the settings.
    class RMVSettings
    {
    public:
        /// @brief Get the single settings object.
        ///
        /// @return a reference to the RMVSettings.
        static RMVSettings& Get();

        /// @brief Constructor.
        RMVSettings();

        /// @brief Destructor.
        ~RMVSettings();

        /// @brief Get file path to the settings.
        ///
        /// Find the 'Temp' folder on the local OS and create a subfolder (on linux, create .RMV folder).
        ///
        /// @return The name and location of the xml file.
        QString GetSettingsFileLocation() const;

        /// @brief Apply default settings and then override them if found on disk.
        ///
        /// @return true if settings were read from file, and false otherwise.
        bool LoadSettings();

        /// @brief Save the settings (and list of recent files) to disk.
        void SaveSettings() const;

        /// @brief Add a setting to our active map if it is recognized.
        ///
        /// @param [in] name  The name of the setting to add.
        /// @param [in] value The value of the setting.
        void AddPotentialSetting(const QString& name, const QString& value);

        /// @brief Add a recent file to the settings.
        ///
        /// @param [in] recent_file The recent file to add.
        void AddRecentFile(const RecentFileData& recent_file);

        /// @brief Update the recent files list.
        ///
        /// Called when loading a new trace file. If the file already exists in the recent files list,
        /// bump it to the top. If it doesn't exist then add it to the list.
        ///
        /// @param [in] trace_file_name  The trace file name to add/remove.
        /// @param [in] data_set         The data set.
        /// @param [in] remove_from_list If true, remove the trace file from the list.
        void TraceLoaded(const QString& trace_file_name, const RmtDataSet* data_set, bool remove_from_list);

        /// @brief Remove a file from the recent files list.
        ///
        /// @param [in] trace_name The name of the file to remove.
        void RemoveRecentFile(const QString& trace_name);

        /// @brief Determines if a file path exists in recent files list.
        ///
        /// @return If file is found, returns true.  Otherwise returns false.
        bool DoesFileExistInRecentList(const char* file_path);

        /// @brief Get a reference to the settings.
        ///
        /// @return A reference to the settings.
        const QMap<RMVSettingID, RMVSetting>& Settings() const;

        /// @brief Get a reference to the recent files list.
        ///
        /// @return A reference to the recent files list.
        const QVector<RecentFileData>& RecentFiles() const;

        /// @brief Get a setting as a string value.
        ///
        /// @param [in] setting_id The identifier for this setting.
        ///
        /// @return The string value for the setting specified.
        QString GetStringValue(const RMVSettingID setting_id) const;

        /// @brief Set a setting as an string value.
        ///
        /// @param [in] setting_id The identifier for this setting.
        /// @param [in] value      The new value of the setting.
        void SetStringValue(RMVSettingID setting_id, const QString& value);

        /// @brief Get window width from the settings.
        ///
        /// @return Main window width.
        int GetWindowWidth() const;

        /// @brief Get window height from the settings.
        ///
        /// @return Main window height.
        int GetWindowHeight() const;

        /// @brief Get window X screen position from the settings.
        ///
        /// @return Main window x position.
        int GetWindowXPos() const;

        /// @brief Get window Y screen position from the settings.
        ///
        /// @return Main window y position.
        int GetWindowYPos() const;

        /// @brief Get the time units override setting.
        ///
        /// @return If the override is enabled, true is returned.  Otherwise false is returned.
        bool IsUnitsOverrideEnabled() const;

        /// @brief Get timing units from the settings.
        ///
        /// If the time units override flag is set, then return clock cycles as the unit.
        ///
        /// @return A TimeUnitType value.
        TimeUnitType GetUnits() const;

        /// @brief Get last file open location from the settings.
        ///
        /// @return Path to last opened file dir.
        QString& GetLastFileOpenLocation();

        /// @brief Sets the size of the window (width and height) in the settings.
        ///
        /// @param [in] width  The new width.
        /// @param [in] height The new height.
        void SetWindowSize(const int width, const int height);

        /// @brief Sets the position of the window on the screen in the settings.
        ///
        /// @param [in] x_pos The new X position.
        /// @param [in] y_pos The new Y Position.
        void SetWindowPos(const int x_pos, const int y_pos);

        /// @brief Set last file open location in the settings.
        ///
        /// @param [in] last_file_open_location  path + filename.
        void SetLastFileOpenLocation(const QString& last_file_open_location);

        /// @brief Set the value of kSettingGeneralCheckForUpdatesOnStartup in the settings.
        ///
        /// @param [in] value The new value of kSettingGeneralCheckForUpdatesOnStartup.
        void SetCheckForUpdatesOnStartup(const bool value);

        /// @brief Set the value of kSettingDriverOverridesAllowNotifications in the settings.
        ///
        /// @param [in] value The new value of kSettingDriverOverridesAllowNotifications.
        void SetDriverOverridesAllowNotifications(const bool value);

        /// @brief Allows units to be displayed as clock cycles for traces with invalid clock frequencies.
        ///
        /// @param [in] enable When set to true, units are displayed as clock cycles regardless
        ///  of the user's time unit preference.  When set to false, the last saved time
        ///  units preference is used.
        void SetUnitsOverrideEnable(bool enable);

        /// @brief Set the timing units in the settings.
        ///
        /// @param [in] units The new value of the timing units.
        void SetUnits(const TimeUnitType units);

        /// @brief Get the value of kSettingGeneralCheckForUpdatesOnStartup in the settings.
        ///
        /// @return The value of kSettingGeneralCheckForUpdatesOnStartup.
        bool GetCheckForUpdatesOnStartup();

        /// @brief Get the value of kSetOverridesAllowNotifications in the settings.
        ///
        /// @return The value of kSettingDriverOverridesAllowNotifications.
        bool GetDriverOverridesAllowNotifications();

        /// @brief Gets the current color theme mode.
        ///
        /// @return the value of the current color theme.
        int GetColorTheme();

        /// @brief Sets the color theme mode.
        ///
        /// @param [in] the value of the color theme.
        void SetColorTheme(int value);

        /// @brief Get the color palette from the settings.
        ///
        /// @return The current color palette.
        const ColorPalette& GetColorPalette() const;

        /// @brief Get the value of a palette id from the settings.
        ///
        /// @param [in] setting_id The id of the palette setting to query.
        ///
        /// @return The palette id value for the setting_id.
        int GetPaletteId(RMVSettingID setting_id);

        /// @brief Set the value of a palette id in the settings.
        ///
        /// @param [in] setting_id The id of the setting to change.
        /// @param [in] value      The new palette id value of this item.
        void SetPaletteId(RMVSettingID setting_id, const int value);

        /// @brief Cache the color palette.
        ///
        /// Creating a temporary ColorPalette object with a palette string for each palette query can be time consuming.
        void CachePalette();

        /// @brief Set the color palette.
        ///
        /// @param [in] value The new color palette value.
        void SetColorPalette(const ColorPalette& value);

        /// @brief Restore all color settings to their default value.
        void RestoreDefaultColors();

        /// @brief Restore all palette settings to their default value.
        void RestoreDefaultPalette();

        /// @brief Get a setting as a QColor object.
        ///
        /// @param [in] setting_id The identifier for this setting.
        ///
        /// @return The color value for the setting specified.
        QColor GetColorValue(RMVSettingID setting_id) const;

        /// @brief Get kSettingThemesAndColorsSnapshotViewed from the settings.
        ///
        /// @return The color value of this snapshot.
        QColor GetColorSnapshotViewed() const;

        /// @brief Get kSettingThemesAndColorsSnapshotComapred from the settings.
        ///
        /// @return The color value of this snapshot.
        QColor GetColorSnapshotCompared() const;

        /// @brief Get kSettingThemesAndColorsSnapshotLive from the settings.
        ///
        /// @return The color value of this snapshot.
        QColor GetColorSnapshotLive() const;

        /// @brief Get kSettingThemesAndColorsSnapshotGenerated from the settings.
        ///
        /// @return The color value of this snapshot.
        QColor GetColorSnapshotGenerated() const;

        /// @brief Get kSettingThemesAndColorsSnapshotVma from the settings.
        ///
        /// @return The color value of this snapshot.
        QColor GetColorSnapshotVMA() const;

        /// @brief Get kSettingThemesAndColorsResourceDsBuffer from the settings.
        ///
        /// @return The color value of this resource.
        QColor GetColorResourceDepthStencil() const;

        /// @brief Get kSettingThemesAndColorsResourceRenderTarget from the settings.
        ///
        /// @return The color value of this resource.
        QColor GetColorResourceRenderTarget() const;

        /// @brief Get kSettingThemesAndColorsResourceTexture from the settings.
        ///
        /// @return The color value of this resource.
        QColor GetColorResourceTexture() const;

        /// @brief Get kSettingThemesAndColorsResourceVertexBuffer from the settings.
        ///
        /// @return The color value of this resource.
        QColor GetColorResourceVertexBuffer() const;

        /// @brief Get kSettingThemesAndColorsResourceIndexBuffer from the settings.
        ///
        /// @return The color value of this resource.
        QColor GetColorResourceIndexBuffer() const;

        /// @brief Get kSettingThemesAndColorsResourceRayTracingBuffer from the settings.
        ///
        /// @return The color value of this resource.
        QColor GetColorResourceRayTracingBuffer() const;

        /// @brief Get kSettingThemesAndColorsResourceUav from the settings.
        ///
        /// @return The color value of this resource.
        QColor GetColorResourceUAV() const;

        /// @brief Get kSettingThemesAndColorsResourceShaderPipeline from the settings.
        ///
        /// @return The color value of this resource.
        QColor GetColorResourceShaderPipeline() const;

        /// @brief Get kSettingThemesAndColorsResourceCommandBuffer from the settings.
        ///
        /// @return The color value of this resource.
        QColor GetColorResourceCommandBuffer() const;

        /// @brief Get kSettingThemesAndColorsResourceHeap from the settings.
        ///
        /// @return The color value of this resource.
        QColor GetColorResourceHeap() const;

        /// @brief Get kSettingThemesAndColorsResourceDescriptors from the settings.
        ///
        /// @return The color value of this resource.
        QColor GetColorResourceDescriptors() const;

        /// @brief Get kSettingThemesAndColorsResourceBuffer from the settings.
        ///
        /// @return The color value of this resource.
        QColor GetColorResourceBuffer() const;

        /// @brief Get kSettingThemesAndColorsResourceGpuEvent from the settings.
        ///
        /// @return The color value of this resource.
        QColor GetColorResourceGPUEvent() const;

        /// @brief Get kSettingThemesAndColorsResourceFreeSpace from the settings.
        ///
        /// @return The color value of this resource.
        QColor GetColorResourceFreeSpace() const;

        /// @brief Get kSettingThemesAndColorsResourceInternal from the settings.
        ///
        /// @return The color value of this resource.
        QColor GetColorResourceInternal() const;

        /// @brief Get kSettingThemesAndColorsDeltaIncrease from the settings.
        ///
        /// @return The color value of this delta type.
        QColor GetColorDeltaIncrease() const;

        /// @brief Get kSettingThemesAndColorsDeltaDecrease from the settings.
        ///
        /// @return The color value of this delta type.
        QColor GetColorDeltaDecrease() const;

        /// @brief Get kSettingThemesAndColorsDeltaNoChange from the settings.
        ///
        /// @return The color value of this delta type.
        QColor GetColorDeltaNoChange() const;

        /// @brief Get kSettingThemesAndColorsHeapLocal from the settings.
        ///
        /// @return The color value of this heap.
        QColor GetColorHeapLocal() const;

        /// @brief Get kSettingThemesAndColorsHeapInvisible from the settings.
        ///
        /// @return The color value of this heap.
        QColor GetColorHeapInvisible() const;

        /// @brief Get kSettingThemesAndColorsHeapSystem from the settings.
        ///
        /// @return The color value of this heap.
        QColor GetColorHeapSystem() const;

        /// @brief Get kSettingThemesAndColorsHeapUnspecified from the settings.
        ///
        /// @return The color value of this heap.
        QColor GetColorHeapUnspecified() const;

        /// @brief Get kSettingThemesAndColorsCpuMapped from the settings.
        ///
        /// @return The color value of this heap.
        QColor GetColorCPUMapped() const;

        /// @brief Get kSettingThemesAndColorsNotCpuMapped from the settings.
        ///
        /// @return The color value of this heap.
        QColor GetColorNotCPUMapped() const;

        /// @brief Get kSettingThemesAndColorsInPreferredHeap from the settings.
        ///
        /// @return The color value of this heap.
        QColor GetColorInPreferredHeap() const;

        /// @brief Get kSettingThemesAndColorsNotInPreferredHeap from the settings.
        ///
        /// @return The color value of this heap.
        QColor GetColorNotInPreferredHeap() const;

        /// @brief Get kSettingThemesAndColorsAliased from the settings.
        ///
        /// @return The color value of this heap.
        QColor GetColorAliased() const;

        /// @brief Get kSettingThemesAndColorsNotAliased from the settings.
        ///
        /// @return The color value of this heap.
        QColor GetColorNotAliased() const;

        /// @brief Get kSettingThemesAndColorsResourceHistoryResourceEvent from the settings.
        ///
        /// @return The color value of this resource history event.
        QColor GetColorResourceHistoryResourceEvent() const;

        /// @brief Get kSettingThemesAndColorsResourceHistoryCpuMapUnmap from the settings.
        ///
        /// @return The color value of this resource history event.
        QColor GetColorResourceHistoryCpuMapping() const;

        /// @brief Get kSettingThemesAndColorsResourceHistoryResidencyUpdate from the settings.
        ///
        /// @return The color value of this resource history event.
        QColor GetColorResourceHistoryResidencyUpdate() const;

        /// @brief Get kSettingThemesAndColorsResourceHistoryPageTableUpdate from the settings.
        ///
        /// @return The color value of this resource history event.
        QColor GetColorResourceHistoryPageTableUpdate() const;

        /// @brief Get kSettingThemesAndColorsResourceHistoryHighlight from the settings.
        ///
        /// @return The color value of this resource history event.
        QColor GetColorResourceHistoryHighlight() const;

        /// @brief Get kSettingThemesAndColorsResourceHistorySnapshot from the settings.
        ///
        /// @return The color value of this resource history event.
        QColor GetColorResourceHistorySnapshot() const;

        /// @brief Get kSettingThemesAndColorsCommitTypeCommitted from the settings.
        ///
        /// @return The color value of this commit type.
        QColor GetColorCommitTypeCommitted() const;

        /// @brief Get kSettingThemesAndColorsCommitTypePlaced from the settings.
        ///
        /// @return The color value of this commit type.
        QColor GetColorCommitTypePlaced() const;

        /// @brief Get kSettingThemesAndColorsCommitTypeVirtual from the settings.
        ///
        /// @return The color value of this commit type.
        QColor GetColorCommitTypeVirtual() const;

        /// @brief Cycle through the available time units.
        void CycleTimeUnits();

    private:
        /// @brief Set the value of checkbox's state in the settings.
        ///
        /// @param [in] setting_id The setting id of checkbox.
        /// @param [in] value      The new value of checkbox state.
        void SetCheckBoxStatus(const RMVSettingID setting_id, const bool value);

        /// @brief Get checkbox state from the settings.
        ///
        /// @param [in] setting_id The setting id of checkbox.
        ///
        /// @return The value of checkbox state.
        bool GetCheckBoxStatus(const RMVSettingID setting_id) const;

        /// @brief Initialize our table with default settings.
        void InitDefaultSettings();

        /// @brief Store an active setting.
        ///
        /// @param [in] id      The identifier for this setting.
        /// @param [in] setting The setting containing name and value.
        void AddActiveSetting(RMVSettingID id, const RMVSetting& setting);

        /// @brief Get a setting as a boolean value.
        ///
        /// @param [in] setting_id The identifier for this setting.
        ///
        /// @return The boolean value for the setting specified.
        bool GetBoolValue(RMVSettingID setting_id) const;

        /// @brief Get a setting as an integer value.
        ///
        /// @param [in] setting_id The identifier for this setting.
        ///
        /// @return The integer value for the setting specified.
        int GetIntValue(RMVSettingID setting_id) const;

        /// @brief Set a setting as a boolean value.
        ///
        /// @param [in] setting_id The identifier for this setting.
        /// @param [in] value      The new value of the setting.
        void SetBoolValue(RMVSettingID setting_id, const bool value);

        /// @brief Set a setting as an integer value.
        ///
        /// @param [in] setting_id The identifier for this setting.
        /// @param [in] value      The new value of the setting.
        void SetIntValue(RMVSettingID setting_id, const int value);

        /// @brief Restore a setting to its default value.
        ///
        /// @param [in] setting_id The identifier for this setting.
        void SetToDefaultValue(RMVSettingID setting_id);

        /// @brief Remove a file from the recent files list.
        ///
        /// @param [in] file_name The name of the file to remove.
        void RemoveRecentFile(const char* file_name);

        QVector<RecentFileData> recent_files_;      ///< Vector of recently opened files.
        RMVSettingsMap          active_settings_;   ///< Map containing active settings.
        RMVSettingsMap          default_settings_;  ///< Map containing default settings.
        ColorPalette*           color_palette_;     ///< The currently cached color palette.
        bool                    override_units_;    ///< Force time units to be in clock cycles (for traces with invalid CPU frequencies).
    };
}  // namespace rmv

#endif  // RMV_SETTINGS_RMV_SETTINGS_H_
