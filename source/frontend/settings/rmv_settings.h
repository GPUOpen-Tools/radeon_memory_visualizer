//=============================================================================
/// Copyright (c) 2018-2020 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief Define Rmv settings and information about recently opened traces.
//=============================================================================

#ifndef RMV_SETTINGS_RMV_SETTINGS_H_
#define RMV_SETTINGS_RMV_SETTINGS_H_

#include <QVector>
#include <QMap>

#include "qt_common/utils/color_palette.h"
#include "qt_common/utils/common_definitions.h"

#include "rmt_data_set.h"

#include "util/definitions.h"

/// A struct for a setting key-value pair.
struct RMVSetting
{
    QString name;   ///< Name of the setting.
    QString value;  ///< Value of the setting.
};

/// Enum of all settings.
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
    kSettingThemesAndColorsResourceUav,
    kSettingThemesAndColorsResourceShaderPipeline,
    kSettingThemesAndColorsResourceCommandBuffer,
    kSettingThemesAndColorsResourceHeap,
    kSettingThemesAndColorsResourceDescriptors,
    kSettingThemesAndColorsResourceBuffer,
    kSettingThemesAndColorsResourceGpuEvent,
    kSettingThemesAndColorsResourceFreeSpace,
    kSettingThemesAndColorsResourceInternal,

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

/// Support for Rmv's settings.
class RMVSettings
{
public:
    /// Get the single settings object.
    /// \return a reference to the RMVSettings.
    static RMVSettings& Get();

    /// Constructor.
    RMVSettings();

    /// Destructor.
    ~RMVSettings();

    /// Get file path to RMV settings. Find the 'Temp' folder on the local OS
    /// and create an RMV subfolder (on linux, create .RMV folder).
    /// \return The name and location of the xml file.
    QString GetSettingsFileLocation() const;

    /// Apply default settings and then override them if found on disk.
    /// \return true if settings were read from file, and false otherwise.
    bool LoadSettings();

    /// Save the settings (and list of recent files) to disk.
    void SaveSettings() const;

    /// Add a setting to our active map if it is recognized.
    /// \param name The name of the setting to add.
    /// \param value The value of the setting.
    void AddPotentialSetting(const QString& name, const QString& value);

    /// Add a recent file to the settings.
    /// \param recent_file The recent file to add.
    void AddRecentFile(const RecentFileData& recent_file);

    /// Update the recent files list. Called when loading a new trace file. If the file
    /// already exists in the recent files list, bump it to the top. If it doesn't exist
    /// then add it to the list.
    /// \param trace_file_name The trace file name to add/remove.
    /// \param data_set The data set.
    /// \param remove_from_list If true, remove the trace file from the list.
    void TraceLoaded(const char* trace_file_name, const RmtDataSet* data_set, bool remove_from_list);

    /// Remove a file from the recent files list.
    /// \param trace_name The name of the file to remove.
    void RemoveRecentFile(const QString& trace_name);

    /// Get a reference to the settings.
    /// \return A reference to the settings.
    QMap<RMVSettingID, RMVSetting>& Settings();

    /// Get a reference to the recent files list.
    /// \return A reference to the recent files list.
    const QVector<RecentFileData>& RecentFiles();

    /// Get a setting as a string value.
    /// \param setting_id The identifier for this setting.
    /// \return The string value for the setting specified.
    QString GetStringValue(const RMVSettingID setting_id) const;

    /// Set a setting as an string value.
    /// \param setting_id The identifier for this setting.
    /// \param value The new value of the setting.
    void SetStringValue(RMVSettingID setting_id, const QString& value);

    /// Get window width from the settings.
    /// \return Main window width.
    int GetWindowWidth() const;

    /// Get window height from the settings.
    /// \return Main window height.
    int GetWindowHeight() const;

    /// Get window X screen position from the settings.
    /// \return Main window x position.
    int GetWindowXPos() const;

    /// Get window Y screen position from the settings.
    /// \return Main window y position.
    int GetWindowYPos() const;

    /// Get the time units override setting.
    /// \return If the override is enabled, true is returned.  Otherwise false is
    /// returned.
    bool IsUnitsOverrideEnabled() const;

    /// Get timing units from the settings.  If the time units override flag is
    /// set, then return clock cycles as the unit.
    /// \return A TimeUnitType value.
    TimeUnitType GetUnits() const;

    /// Get last file open location from the settings.
    /// \return Path to last opened file dir.
    QString& GetLastFileOpenLocation();

    /// Sets the size of the window (width and height) in the settings.
    /// \param width The new width.
    /// \param height The new height.
    void SetWindowSize(const int width, const int height);

    /// Sets the position of the window on the screen in the settings.
    /// \param x_pos The new X position.
    /// \param y_pos The new Y Position.
    void SetWindowPos(const int x_pos, const int y_pos);

    /// Set last file open location in the settings.
    /// \param last_file_open_location  path + filename.
    void SetLastFileOpenLocation(const QString& last_file_open_location);

    /// Set the value of kSettingGeneralCheckForUpdatesOnStartup in the settings.
    /// \param value The new value of kSettingGeneralCheckForUpdatesOnStartup.
    void SetCheckForUpdatesOnStartup(const bool value);

    /// Allows units to be displayed as clock cycles for traces with invalid clock frequencies.
    /// \param enable When set to true, units are displayed as clock cycles regardless
    /// of the user's time unit preference.  When set to false, the last saved time
    /// units preference is used.
    void SetUnitsOverrideEnable(bool enable);

    /// Set the timing units in the settings.
    /// \param units The new value of the timing units.
    void SetUnits(const TimeUnitType units);

    /// Set the value of kSettingGeneralAllocUniquenessHeap in the settings.
    /// \param value The new value of kSettingGeneralAllocUniquenessHeap.
    void SetAllocUniqunessHeap(const bool value);

    /// Set the value of kSettingGeneralAllocUniquenessAllocation in the settings.
    /// \param value The new value of kSettingGeneralAllocUniquenessAllocation.
    void SetAllocUniqunessAllocation(const bool value);

    /// Set the value of kSettingGeneralAllocUniquenessOffset in the settings.
    /// \param value The new value of kSettingGeneralAllocUniquenessOffset.
    void SetAllocUniqunessOffset(const bool value);

    /// Get the value of kSettingGeneralCheckForUpdatesOnStartup in the settings.
    /// \return The value of kSettingGeneralCheckForUpdatesOnStartup.
    bool GetCheckForUpdatesOnStartup();

    /// Get the value of kSettingGeneralAllocUniquenessHeap in the settings.
    /// \return The value of kSettingGeneralAllocUniquenessHeap.
    bool GetAllocUniqunessHeap();

    /// Get the value of kSettingGeneralAllocUniquenessAllocation in the settings.
    /// \return The value of kSettingGeneralAllocUniquenessAllocation.
    bool GetAllocUniqunessAllocation();

    /// Get the value of kSettingGeneralAllocUniquenessOffset in the settings.
    /// \return The value of kSettingGeneralAllocUniquenessOffset.
    bool GetAllocUniqunessOffset();

    /// Get the color palette from the settings.
    /// \return The current color palette.
    const ColorPalette& GetColorPalette() const;

    /// Get the value of a palette id from the settings.
    /// \param setting_id The id of the setting to query.
    /// \return The palette id value of this item.
    int GetPaletteId(RMVSettingID setting_id);

    /// Set the value of a palette id in the settings.
    /// \param setting_id The id of the setting to change.
    /// \param value The new palette id value of this item.
    void SetPaletteId(RMVSettingID setting_id, const int value);

    /// Cache the color palette. Creating a temporary ColorPalette object with a
    /// palette string for each palette query can be time consuming.
    void CachePalette();

    /// Set the color palette.
    /// \param value The new color palette value.
    void SetColorPalette(const ColorPalette& value);

    /// Restore all color settings to their default value.
    void RestoreDefaultColors();

    /// Restore all palette settings to their default value.
    void RestoreDefaultPalette();

    /// Get a setting as a QColor object.
    /// \param setting_id The identifier for this setting.
    /// \return The color value for the setting specified.
    QColor GetColorValue(RMVSettingID setting_id) const;

    /// Get kSettingThemesAndColorsSnapshotViewed from the settings.
    /// \return The color value of this snapshot.
    QColor GetColorSnapshotViewed() const;

    /// Get kSettingThemesAndColorsSnapshotComapred from the settings.
    /// \return The color value of this snapshot.
    QColor GetColorSnapshotCompared() const;

    /// Get kSettingThemesAndColorsSnapshotLive from the settings.
    /// \return The color value of this snapshot.
    QColor GetColorSnapshotLive() const;

    /// Get kSettingThemesAndColorsSnapshotGenerated from the settings.
    /// \return The color value of this snapshot.
    QColor GetColorSnapshotGenerated() const;

    /// Get kSettingThemesAndColorsSnapshotVma from the settings.
    /// \return The color value of this snapshot.
    QColor GetColorSnapshotVMA() const;

    /// Get kSettingThemesAndColorsResourceDsBuffer from the settings.
    /// \return The color value of this resource.
    QColor GetColorResourceDepthStencil() const;

    /// Get kSettingThemesAndColorsResourceRenderTarget from the settings.
    /// \return The color value of this resource.
    QColor GetColorResourceRenderTarget() const;

    /// Get kSettingThemesAndColorsResourceTexture from the settings.
    /// \return The color value of this resource.
    QColor GetColorResourceTexture() const;

    /// Get kSettingThemesAndColorsResourceVertexBuffer from the settings.
    /// \return The color value of this resource.
    QColor GetColorResourceVertexBuffer() const;

    /// Get kSettingThemesAndColorsResourceIndexBuffer from the settings.
    /// \return The color value of this resource.
    QColor GetColorResourceIndexBuffer() const;

    /// Get kSettingThemesAndColorsResourceUav from the settings.
    /// \return The color value of this resource.
    QColor GetColorResourceUAV() const;

    /// Get kSettingThemesAndColorsResourceShaderPipeline from the settings.
    /// \return The color value of this resource.
    QColor GetColorResourceShaderPipeline() const;

    /// Get kSettingThemesAndColorsResourceCommandBuffer from the settings.
    /// \return The color value of this resource.
    QColor GetColorResourceCommandBuffer() const;

    /// Get kSettingThemesAndColorsResourceHeap from the settings.
    /// \return The color value of this resource.
    QColor GetColorResourceHeap() const;

    /// Get kSettingThemesAndColorsResourceDescriptors from the settings.
    /// \return The color value of this resource.
    QColor GetColorResourceDescriptors() const;

    /// Get kSettingThemesAndColorsResourceBuffer from the settings.
    /// \return The color value of this heresourceap.
    QColor GetColorResourceBuffer() const;

    /// Get kSettingThemesAndColorsResourceGpuEvent from the settings.
    /// \return The color value of this resource.
    QColor GetColorResourceGPUEvent() const;

    /// Get kSettingThemesAndColorsResourceFreeSpace from the settings.
    /// \return The color value of this resource.
    QColor GetColorResourceFreeSpace() const;

    /// Get kSettingThemesAndColorsResourceInternal from the settings.
    /// \return The color value of this resource.
    QColor GetColorResourceInternal() const;

    /// Get kSettingThemesAndColorsDeltaIncrease from the settings.
    /// \return The color value of this delta type.
    QColor GetColorDeltaIncrease() const;

    /// Get kSettingThemesAndColorsDeltaDecrease from the settings.
    /// \return The color value of this delta type.
    QColor GetColorDeltaDecrease() const;

    /// Get kSettingThemesAndColorsDeltaNoChange from the settings.
    /// \return The color value of this delta type.
    QColor GetColorDeltaNoChange() const;

    /// Get kSettingThemesAndColorsHeapLocal from the settings.
    /// \return The color value of this heap.
    QColor GetColorHeapLocal() const;

    /// Get kSettingThemesAndColorsHeapInvisible from the settings.
    /// \return The color value of this heap.
    QColor GetColorHeapInvisible() const;

    /// Get kSettingThemesAndColorsHeapSystem from the settings.
    /// \return The color value of this heap.
    QColor GetColorHeapSystem() const;

    /// Get kSettingThemesAndColorsHeapUnspecified from the settings.
    /// \return The color value of this heap.
    QColor GetColorHeapUnspecified() const;

    /// Get kSettingThemesAndColorsCpuMapped from the settings.
    /// \return The color value of this heap.
    QColor GetColorCPUMapped() const;

    /// Get kSettingThemesAndColorsNotCpuMapped from the settings.
    /// \return The color value of this heap.
    QColor GetColorNotCPUMapped() const;

    /// Get kSettingThemesAndColorsInPreferredHeap from the settings.
    /// \return The color value of this heap.
    QColor GetColorInPreferredHeap() const;

    /// Get kSettingThemesAndColorsNotInPreferredHeap from the settings.
    /// \return The color value of this heap.
    QColor GetColorNotInPreferredHeap() const;

    /// Get kSettingThemesAndColorsAliased from the settings.
    /// \return The color value of this heap.
    QColor GetColorAliased() const;

    /// Get kSettingThemesAndColorsNotAliased from the settings.
    /// \return The color value of this heap.
    QColor GetColorNotAliased() const;

    /// Get kSettingThemesAndColorsResourceHistoryResourceEvent from the settings.
    /// \return The color value of this resource history event.
    QColor GetColorResourceHistoryResourceEvent() const;

    /// Get kSettingThemesAndColorsResourceHistoryCpuMapUnmap from the settings.
    /// \return The color value of this resource history event.
    QColor GetColorResourceHistoryCpuMapping() const;

    /// Get kSettingThemesAndColorsResourceHistoryResidencyUpdate from the settings.
    /// \return The color value of this resource history event.
    QColor GetColorResourceHistoryResidencyUpdate() const;

    /// Get kSettingThemesAndColorsResourceHistoryPageTableUpdate from the settings.
    /// \return The color value of this resource history event.
    QColor GetColorResourceHistoryPageTableUpdate() const;

    /// Get kSettingThemesAndColorsResourceHistoryHighlight from the settings.
    /// \return The color value of this resource history event.
    QColor GetColorResourceHistoryHighlight() const;

    /// Get kSettingThemesAndColorsResourceHistorySnapshot from the settings.
    /// \return The color value of this resource history event.
    QColor GetColorResourceHistorySnapshot() const;

    /// Get kSettingThemesAndColorsCommitTypeCommitted from the settings.
    /// \return The color value of this commit type.
    QColor GetColorCommitTypeCommitted() const;

    /// Get kSettingThemesAndColorsCommitTypePlaced from the settings.
    /// \return The color value of this commit type.
    QColor GetColorCommitTypePlaced() const;

    /// Get kSettingThemesAndColorsCommitTypeVirtual from the settings.
    /// \return The color value of this commit type.
    QColor GetColorCommitTypeVirtual() const;

private:
    /// Set the value of checkbox's state in the settings.
    /// \param setting_id The setting id of checkbox.
    /// \param value The new value of checkbox state.
    void SetCheckBoxStatus(const RMVSettingID setting_id, const bool value);

    /// Get checkbox state from the settings.
    /// \param setting_id The setting id of checkbox.
    /// \return The value of checkbox state.
    bool GetCheckBoxStatus(const RMVSettingID setting_id) const;

    /// Initialize our table with default settings.
    void InitDefaultSettings();

    /// Store an active setting.
    /// \param setting_id The identifier for this setting.
    /// \param setting The setting containing name and value.
    void AddActiveSetting(RMVSettingID id, const RMVSetting& setting);

    /// Get a setting as a boolean value.
    /// \param setting_id The identifier for this setting.
    /// \return The boolean value for the setting specified.
    bool GetBoolValue(RMVSettingID setting_id) const;

    /// Get a setting as an integer value.
    /// \param setting_id The identifier for this setting.
    /// \return The integer value for the setting specified.
    int GetIntValue(RMVSettingID setting_id) const;

    /// Set a setting as a boolean value.
    /// \param setting_id The identifier for this setting.
    /// \param value The new value of the setting.
    void SetBoolValue(RMVSettingID setting_id, const bool value);

    /// Set a setting as an integer value.
    /// \param setting_id The identifier for this setting.
    /// \param value The new value of the setting.
    void SetIntValue(RMVSettingID setting_id, const int value);

    /// Restore a setting to its default value.
    /// \param setting_id The identifier for this setting.
    void SetToDefaultValue(RMVSettingID setting_id);

    /// Remove a file from the recent files list.
    /// \param file_name The name of the file to remove.
    void RemoveRecentFile(const char* file_name);

    QVector<RecentFileData> recent_files_;      ///< Vector of recently opened files.
    RMVSettingsMap          active_settings_;   ///< Map containing active settings.
    RMVSettingsMap          default_settings_;  ///< Map containing default settings.
    ColorPalette*           color_palette_;     ///< The currently cached color palette.
    bool                    override_units_;    ///< Force time units to be in clock cycles (for traces with invalid CPU frequencies).
};

#endif  // RMV_SETTINGS_RMV_SETTINGS_H_
