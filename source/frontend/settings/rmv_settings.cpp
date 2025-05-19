//=============================================================================
// Copyright (c) 2018-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Implementation for the settings.
//=============================================================================

#include "settings/rmv_settings.h"

#include <chrono>
#include <ctime>

#include <QFile>

#include "rmt_assert.h"

#include "settings/settings_reader.h"
#include "settings/settings_writer.h"
#include "util/constants.h"
#include "util/rmv_util.h"

namespace rmv
{
    // Single instance of the RMVSettings.
    static RMVSettings rmv_settings;

    RMVSettings& RMVSettings::Get()
    {
        return rmv_settings;
    }

    RMVSettings::RMVSettings()
        : color_palette_(nullptr)
        , override_units_(false)
    {
        InitDefaultSettings();
    }

    RMVSettings::~RMVSettings()
    {
        delete color_palette_;
    }

    void RMVSettings::AddRecentFile(const RecentFileData& recent_file)
    {
        recent_files_.push_back(recent_file);
    }

    void RMVSettings::TraceLoaded(const QString& trace_file_name, const RmtDataSet* data_set, bool remove_from_list)
    {
        RecentFileData trace_file;

        // Make sure there's a valid trace loaded.
        if (data_set != nullptr)
        {
            trace_file.path = trace_file_name;
            trace_file.keywords.clear();

            trace_file.created = QString::number(data_set->create_time);

            std::chrono::system_clock::time_point tp  = std::chrono::system_clock::now();
            std::time_t                           now = std::chrono::system_clock::to_time_t(tp);
            trace_file.accessed                       = QString::number(now);

            // If the file loaded is from the recent files list, remove it.
            RemoveRecentFile(trace_file_name);

            // Add the loaded file to the top of the recent file list.
            recent_files_.insert(recent_files_.begin(), trace_file);
        }

        if (remove_from_list)
        {
            // Trace failed to load, so remove it from the recent file list.
            RemoveRecentFile(trace_file_name);
        }
    }

    void RMVSettings::RemoveRecentFile(const char* file_name)
    {
        const int num_recent_files = this->recent_files_.size();
        for (int loop = 0; loop < num_recent_files; loop++)
        {
            if (file_name != nullptr && recent_files_[loop].path.compare(file_name) == 0)
            {
                recent_files_.remove(loop);
                break;
            }
        }
    }

    bool RMVSettings::DoesFileExistInRecentList(const char* file_path)
    {
        const int num_recent_files = recent_files_.size();
        for (int loop = 0; loop < num_recent_files; loop++)
        {
            if (file_path != nullptr && recent_files_[loop].path.compare(file_path) == 0)
            {
                return true;
            }
        }

        return false;
    }

    void RMVSettings::RemoveRecentFile(const QString& trace_name)
    {
        const int num_recent_files = this->recent_files_.size();
        for (int loop = 0; loop < num_recent_files; loop++)
        {
            if (recent_files_[loop].path.compare(trace_name) == 0)
            {
                recent_files_.remove(loop);
                break;
            }
        }
    }

    void RMVSettings::AddPotentialSetting(const QString& name, const QString& value)
    {
        for (RMVSettingsMap::iterator i = default_settings_.begin(); i != default_settings_.end(); ++i)
        {
            if (i.value().name.compare(name) == 0)
            {
                AddActiveSetting(i.key(), {name, value});
                break;
            }
        }
    }

    QString RMVSettings::GetSettingsFileLocation() const
    {
        QString xml_file = "";

        // Get file location
        xml_file = rmv_util::GetFileLocation();

        // Add the file name
        xml_file.append("/RmvSettings.xml");

        return xml_file;
    }

    bool RMVSettings::LoadSettings()
    {
        // Begin by applying the defaults
        for (RMVSettingsMap::iterator i = default_settings_.begin(); i != default_settings_.end(); ++i)
        {
            AddPotentialSetting(i.value().name, i.value().value);
        }

        QFile file(GetSettingsFileLocation());

        bool read_settings_file = file.open(QFile::ReadOnly | QFile::Text);

        // Override the defaults
        if (read_settings_file)
        {
            rmv::SettingsReader xml_reader(&RMVSettings::Get());

            read_settings_file = xml_reader.Read(&file);

            file.close();

            // Make sure the XML parse worked
            Q_ASSERT(read_settings_file == true);
        }

        // If there is not file or if the parsing of an existing file failed, save a new file
        if (!read_settings_file)
        {
            SaveSettings();

            read_settings_file = false;
        }

        SetColorPalette(ColorPalette(active_settings_[kSettingThemesAndColorsPalette].value));

        return read_settings_file;
    }

    void RMVSettings::SaveSettings() const
    {
        QFile file(GetSettingsFileLocation());
        bool  success = file.open(QFile::WriteOnly | QFile::Text);

        if (success)
        {
            rmv::SettingsWriter xml_writer(&RMVSettings::Get());
            success = xml_writer.Write(&file);
            file.close();

            RMT_ASSERT(success == true);
        }
    }

    void RMVSettings::InitDefaultSettings()
    {
        override_units_ = false;

        default_settings_[kSettingMainWindowGeometryData]                   = {"WindowGeometryData", ""};
        default_settings_[kSettingMainWindowWidth]                          = {"WindowWidth", "0"};
        default_settings_[kSettingMainWindowHeight]                         = {"WindowHeight", "0"};
        default_settings_[kSettingMainWindowXpos]                           = {"WindowXPos", "100"};
        default_settings_[kSettingMainWindowYpos]                           = {"WindowYPos", "100"};
        default_settings_[kSettingLastFileOpenLocation]                     = {"LastFileOpenLocation", ""};
        default_settings_[kSettingGeneralCheckForUpdatesOnStartup]          = {"CheckForUpdatesOnStartup", "False"};
        default_settings_[kSettingGeneralTimeUnits]                         = {"TimeUnits", rmv::text::kSettingsUnitsSeconds};
        default_settings_[kSettingGeneralDriverOverridesAllowNotifications] = {"DriverOverridesAllowNotifications", "True"};

        default_settings_[kSettingThemesAndColorsPalette] = {"ColorPalette",
                                                             "#FFFFBA02,#FFFF8B00,#FFF76210,#FFE17F35,#FFDA3B01,#FFEF6950,#FFD03438,#FFFF4343,"
                                                             "#FFFF6062,#FFE81123,#FFEA015D,#FFC40052,#FFFF0080,#FFFF97FF,#FFFF4CFF,#FFDC00DD,"
                                                             "#FF0278D8,#FF0063B1,#FF8E8CD7,#FF6B69D6,#FF7F00FF,#FF754CA8,#FFAF47C2,#FF871797,"
                                                             "#FFC3C3C3,#FF2D7C9A,#FF01B7C5,#FF038288,#FF00B394,#FF018675,#FF00CC69,#FF10883E"};

        default_settings_[kSettingThemesAndColorsSnapshotViewed]    = {"SnapshotViewedColor", "16"};
        default_settings_[kSettingThemesAndColorsSnapshotCompared]  = {"SnapshotComparedColor", "1"};
        default_settings_[kSettingThemesAndColorsSnapshotLive]      = {"SnapshotLiveColor", "9"};
        default_settings_[kSettingThemesAndColorsSnapshotGenerated] = {"SnapshotGeneratedColor", "14"};
        default_settings_[kSettingThemesAndColorsSnapshotVma]       = {"SnapshotVmaColor", "15"};

        default_settings_[kSettingThemesAndColorsResourceDsBuffer]         = {"ResourceDSBufferColor", "28"};
        default_settings_[kSettingThemesAndColorsResourceRenderTarget]     = {"ResourceRenderTargetBufferColor", "8"};
        default_settings_[kSettingThemesAndColorsResourceTexture]          = {"ResourceTextureBufferColor", "3"};
        default_settings_[kSettingThemesAndColorsResourceVertexBuffer]     = {"ResourceVertexBufferColor", "0"};
        default_settings_[kSettingThemesAndColorsResourceIndexBuffer]      = {"ResourceIndexBufferColor", "16"};
        default_settings_[kSettingThemesAndColorsResourceRayTracingBuffer] = {"ResourceRayTracingBufferColor", "26"};
        default_settings_[kSettingThemesAndColorsResourceUav]              = {"ResourceUAVColor", "21"};
        default_settings_[kSettingThemesAndColorsResourceShaderPipeline]   = {"ResourceShaderPipelineColor", "18"};
        default_settings_[kSettingThemesAndColorsResourceCommandBuffer]    = {"ResourceCommandBufferColor", "13"};
        default_settings_[kSettingThemesAndColorsResourceHeap]             = {"ResourceHeapColor", "30"};
        default_settings_[kSettingThemesAndColorsResourceDescriptors]      = {"ResourceDescriptorsColor", "9"};
        default_settings_[kSettingThemesAndColorsResourceBuffer]           = {"ResourceBufferColor", "22"};
        default_settings_[kSettingThemesAndColorsResourceGpuEvent]         = {"ResourceGPUEventColor", "19"};
        default_settings_[kSettingThemesAndColorsResourceFreeSpace]        = {"ResourceFreeSpaceColor", "24"};
        default_settings_[kSettingThemesAndColorsResourceInternal]         = {"ResourceInternalColor", "31"};

        default_settings_[kSettingThemesAndColorsColorThemeMode] = {"ColorThemeMode", "2"};

        default_settings_[kSettingThemesAndColorsDeltaIncrease] = {"DeltaIncreaseColor", "31"};
        default_settings_[kSettingThemesAndColorsDeltaDecrease] = {"DeltaDecreaseColor", "9"};
        default_settings_[kSettingThemesAndColorsDeltaNoChange] = {"DeltaNoChangeColor", "24"};

        default_settings_[kSettingThemesAndColorsHeapLocal]       = {"HeapLocal", "17"};
        default_settings_[kSettingThemesAndColorsHeapInvisible]   = {"HeapInvisible", "18"};
        default_settings_[kSettingThemesAndColorsHeapSystem]      = {"HeapSystem", "7"};
        default_settings_[kSettingThemesAndColorsHeapUnspecified] = {"HeapUnspecified", "24"};

        default_settings_[kSettingThemesAndColorsCpuMapped]    = {"CPUMapped", "7"};
        default_settings_[kSettingThemesAndColorsNotCpuMapped] = {"NotCPUMapped", "24"};

        default_settings_[kSettingThemesAndColorsInPreferredHeap]    = {"InPreferredHeap", "24"};
        default_settings_[kSettingThemesAndColorsNotInPreferredHeap] = {"NotInPreferredHeap", "7"};

        default_settings_[kSettingThemesAndColorsAliased]    = {"Aliased", "7"};
        default_settings_[kSettingThemesAndColorsNotAliased] = {"NotAliased", "24"};

        default_settings_[kSettingThemesAndColorsResourceHistoryResourceEvent]   = {"ResourceHistoryResourceEvent", "1"};
        default_settings_[kSettingThemesAndColorsResourceHistoryCpuMapUnmap]     = {"ResourceHistoryCpuMapping", "16"};
        default_settings_[kSettingThemesAndColorsResourceHistoryResidencyUpdate] = {"ResourceHistoryResidency", "31"};
        default_settings_[kSettingThemesAndColorsResourceHistoryPageTableUpdate] = {"ResourceHistoryPageTable", "0"};
        default_settings_[kSettingThemesAndColorsResourceHistoryHighlight]       = {"ResourceHistoryHighlight", "13"};
        default_settings_[kSettingThemesAndColorsResourceHistorySnapshot]        = {"ResourceHistorySnapshot", "9"};

        default_settings_[kSettingThemesAndColorsCommitTypeCommitted] = {"CommitTypeCommitted", "31"};
        default_settings_[kSettingThemesAndColorsCommitTypePlaced]    = {"CommitTypePlaced", "17"};
        default_settings_[kSettingThemesAndColorsCommitTypeVirtual]   = {"CommitTypeVirtual", "1"};

        color_palette_ = new ColorPalette(default_settings_[kSettingThemesAndColorsPalette].value);
    }

    void RMVSettings::AddActiveSetting(RMVSettingID setting_id, const RMVSetting& setting)
    {
        active_settings_[setting_id] = setting;
    }

    const QMap<RMVSettingID, RMVSetting>& RMVSettings::Settings() const
    {
        return active_settings_;
    }

    const QVector<RecentFileData>& RMVSettings::RecentFiles() const
    {
        return recent_files_;
    }

    QString RMVSettings::GetStringValue(const RMVSettingID setting_id) const
    {
        return active_settings_[setting_id].value;
    }

    bool RMVSettings::GetBoolValue(RMVSettingID setting_id) const
    {
        return (active_settings_[setting_id].value.compare("True") == 0) ? true : false;
    }

    int RMVSettings::GetIntValue(RMVSettingID setting_id) const
    {
        return active_settings_[setting_id].value.toInt();
    }

    void RMVSettings::SetStringValue(RMVSettingID setting_id, const QString& value)
    {
        AddPotentialSetting(default_settings_[setting_id].name, value);
    }

    void RMVSettings::SetToDefaultValue(RMVSettingID setting_id)
    {
        active_settings_[setting_id].value = default_settings_[setting_id].value;
    }

    void RMVSettings::SetBoolValue(RMVSettingID setting_id, const bool value)
    {
        if (value)
        {
            AddPotentialSetting(default_settings_[setting_id].name, "True");
        }
        else
        {
            AddPotentialSetting(default_settings_[setting_id].name, "False");
        }
    }

    void RMVSettings::SetIntValue(RMVSettingID setting_id, const int value)
    {
        AddPotentialSetting(default_settings_[setting_id].name, QString::number(value));
    }

    bool RMVSettings::IsUnitsOverrideEnabled() const
    {
        return override_units_;
    }

    TimeUnitType RMVSettings::GetUnits() const
    {
        const QString value = active_settings_[kSettingGeneralTimeUnits].value;

        if (!override_units_)
        {
            if (value.compare(rmv::text::kSettingsUnitsClocks) == 0)
            {
                return kTimeUnitTypeClk;
            }
            else if (value.compare(rmv::text::kSettingsUnitsMilliseconds) == 0)
            {
                return kTimeUnitTypeMillisecond;
            }
            else if (value.compare(rmv::text::kSettingsUnitsSeconds) == 0)
            {
                return kTimeUnitTypeSecond;
            }
            else if (value.compare(rmv::text::kSettingsUnitsMinutes) == 0)
            {
                return kTimeUnitTypeMinute;
            }
            return kTimeUnitTypeHour;
        }
        else
        {
            return kTimeUnitTypeClk;
        }
    }

    int RMVSettings::GetWindowWidth() const
    {
        return GetIntValue(kSettingMainWindowWidth);
    }

    int RMVSettings::GetWindowHeight() const
    {
        return GetIntValue(kSettingMainWindowHeight);
    }

    int RMVSettings::GetWindowXPos() const
    {
        return GetIntValue(kSettingMainWindowXpos);
    }

    int RMVSettings::GetWindowYPos() const
    {
        return GetIntValue(kSettingMainWindowYpos);
    }

    QString& RMVSettings::GetLastFileOpenLocation()
    {
        return active_settings_[kSettingLastFileOpenLocation].value;
    }

    void RMVSettings::SetUnitsOverrideEnable(bool enable)
    {
        override_units_ = enable;
    }

    void RMVSettings::SetUnits(const TimeUnitType units)
    {
        switch (units)
        {
        case kTimeUnitTypeClk:
            AddPotentialSetting(default_settings_[kSettingGeneralTimeUnits].name, rmv::text::kSettingsUnitsClocks);
            break;

        case kTimeUnitTypeMillisecond:
            AddPotentialSetting(default_settings_[kSettingGeneralTimeUnits].name, rmv::text::kSettingsUnitsMilliseconds);
            break;

        case kTimeUnitTypeSecond:
            AddPotentialSetting(default_settings_[kSettingGeneralTimeUnits].name, rmv::text::kSettingsUnitsSeconds);
            break;

        case kTimeUnitTypeMinute:
            AddPotentialSetting(default_settings_[kSettingGeneralTimeUnits].name, rmv::text::kSettingsUnitsMinutes);
            break;

        case kTimeUnitTypeHour:
        default:
            AddPotentialSetting(default_settings_[kSettingGeneralTimeUnits].name, rmv::text::kSettingsUnitsHours);
            break;
        }
        SaveSettings();
    }

    void RMVSettings::SetLastFileOpenLocation(const QString& last_file_open_location)
    {
        AddPotentialSetting("LastFileOpenLocation", last_file_open_location);
        SaveSettings();
    }

    void RMVSettings::SetWindowSize(const int width, const int height)
    {
        AddPotentialSetting(default_settings_[kSettingMainWindowWidth].name, QString::number(width));
        AddPotentialSetting(default_settings_[kSettingMainWindowHeight].name, QString::number(height));
        SaveSettings();
    }

    void RMVSettings::SetWindowPos(const int x_pos, const int y_pos)
    {
        AddPotentialSetting(default_settings_[kSettingMainWindowXpos].name, QString::number(x_pos));
        AddPotentialSetting(default_settings_[kSettingMainWindowYpos].name, QString::number(y_pos));
        SaveSettings();
    }

    void RMVSettings::SetCheckForUpdatesOnStartup(const bool value)
    {
        SetBoolValue(kSettingGeneralCheckForUpdatesOnStartup, value);
        SaveSettings();
    }

    void RMVSettings::SetDriverOverridesAllowNotifications(const bool value)
    {
        SetBoolValue(kSettingGeneralDriverOverridesAllowNotifications, value);
        SaveSettings();
    }

    void RMVSettings::SetCheckBoxStatus(const RMVSettingID setting_id, const bool value)
    {
        SetBoolValue(setting_id, value);
        SaveSettings();
    }

    bool RMVSettings::GetCheckBoxStatus(const RMVSettingID setting_id) const
    {
        return GetBoolValue(setting_id);
    }

    bool RMVSettings::GetCheckForUpdatesOnStartup()
    {
        return GetBoolValue(kSettingGeneralCheckForUpdatesOnStartup);
    }

    bool RMVSettings::GetDriverOverridesAllowNotifications()
    {
        return GetBoolValue(kSettingGeneralDriverOverridesAllowNotifications);
    }

    int RMVSettings::GetColorTheme()
    {
        return GetIntValue(kSettingThemesAndColorsColorThemeMode);
    }

    void RMVSettings::SetColorTheme(int value)
    {
        SetIntValue(kSettingThemesAndColorsColorThemeMode, value);
        SaveSettings();
    }

    const ColorPalette& RMVSettings::GetColorPalette() const
    {
        return *color_palette_;
    }

    int RMVSettings::GetPaletteId(RMVSettingID setting_id)
    {
        return GetIntValue(setting_id);
    }

    void RMVSettings::SetPaletteId(RMVSettingID setting_id, const int value)
    {
        SetIntValue(setting_id, value);
        SaveSettings();
    }

    void RMVSettings::CachePalette()
    {
        if (color_palette_)
        {
            delete color_palette_;
            color_palette_ = new ColorPalette(active_settings_[kSettingThemesAndColorsPalette].value);
        }
    }

    void RMVSettings::SetColorPalette(const ColorPalette& value)
    {
        active_settings_[kSettingThemesAndColorsPalette].value = value.GetString();
        CachePalette();
        SaveSettings();
    }

    void RMVSettings::RestoreDefaultColors()
    {
        SetToDefaultValue(kSettingThemesAndColorsSnapshotViewed);
        SetToDefaultValue(kSettingThemesAndColorsSnapshotCompared);
        SetToDefaultValue(kSettingThemesAndColorsSnapshotLive);
        SetToDefaultValue(kSettingThemesAndColorsSnapshotGenerated);
        SetToDefaultValue(kSettingThemesAndColorsSnapshotVma);
        SetToDefaultValue(kSettingThemesAndColorsResourceDsBuffer);
        SetToDefaultValue(kSettingThemesAndColorsResourceRenderTarget);
        SetToDefaultValue(kSettingThemesAndColorsResourceTexture);
        SetToDefaultValue(kSettingThemesAndColorsResourceVertexBuffer);
        SetToDefaultValue(kSettingThemesAndColorsResourceIndexBuffer);
        SetToDefaultValue(kSettingThemesAndColorsResourceRayTracingBuffer);
        SetToDefaultValue(kSettingThemesAndColorsResourceUav);
        SetToDefaultValue(kSettingThemesAndColorsResourceShaderPipeline);
        SetToDefaultValue(kSettingThemesAndColorsResourceCommandBuffer);
        SetToDefaultValue(kSettingThemesAndColorsResourceHeap);
        SetToDefaultValue(kSettingThemesAndColorsResourceDescriptors);
        SetToDefaultValue(kSettingThemesAndColorsResourceBuffer);
        SetToDefaultValue(kSettingThemesAndColorsResourceGpuEvent);
        SetToDefaultValue(kSettingThemesAndColorsResourceFreeSpace);
        SetToDefaultValue(kSettingThemesAndColorsResourceInternal);
        SetToDefaultValue(kSettingThemesAndColorsDeltaIncrease);
        SetToDefaultValue(kSettingThemesAndColorsDeltaDecrease);
        SetToDefaultValue(kSettingThemesAndColorsDeltaNoChange);
        SetToDefaultValue(kSettingThemesAndColorsHeapLocal);
        SetToDefaultValue(kSettingThemesAndColorsHeapInvisible);
        SetToDefaultValue(kSettingThemesAndColorsHeapSystem);
        SetToDefaultValue(kSettingThemesAndColorsHeapUnspecified);
        SetToDefaultValue(kSettingThemesAndColorsCpuMapped);
        SetToDefaultValue(kSettingThemesAndColorsNotCpuMapped);
        SetToDefaultValue(kSettingThemesAndColorsInPreferredHeap);
        SetToDefaultValue(kSettingThemesAndColorsNotInPreferredHeap);
        SetToDefaultValue(kSettingThemesAndColorsAliased);
        SetToDefaultValue(kSettingThemesAndColorsNotAliased);
        SetToDefaultValue(kSettingThemesAndColorsResourceHistoryResourceEvent);
        SetToDefaultValue(kSettingThemesAndColorsResourceHistoryCpuMapUnmap);
        SetToDefaultValue(kSettingThemesAndColorsResourceHistoryResidencyUpdate);
        SetToDefaultValue(kSettingThemesAndColorsResourceHistoryPageTableUpdate);
        SetToDefaultValue(kSettingThemesAndColorsResourceHistoryHighlight);
        SetToDefaultValue(kSettingThemesAndColorsResourceHistorySnapshot);
        SetToDefaultValue(kSettingThemesAndColorsCommitTypeCommitted);
        SetToDefaultValue(kSettingThemesAndColorsCommitTypePlaced);
        SetToDefaultValue(kSettingThemesAndColorsCommitTypeVirtual);
        SetToDefaultValue(kSettingGeneralDriverOverridesAllowNotifications);
        SaveSettings();
    }

    void RMVSettings::RestoreDefaultPalette()
    {
        SetToDefaultValue(kSettingThemesAndColorsPalette);
        CachePalette();
        SaveSettings();
    }

    QColor RMVSettings::GetColorValue(RMVSettingID setting_id) const
    {
        const ColorPalette& palette    = GetColorPalette();
        int                 palette_id = GetIntValue(setting_id);

        return palette.GetColor(palette_id);
    }

    QColor RMVSettings::GetColorSnapshotViewed() const
    {
        return GetColorValue(kSettingThemesAndColorsSnapshotViewed);
    }

    QColor RMVSettings::GetColorSnapshotCompared() const
    {
        return GetColorValue(kSettingThemesAndColorsSnapshotCompared);
    }

    QColor RMVSettings::GetColorSnapshotLive() const
    {
        return GetColorValue(kSettingThemesAndColorsSnapshotLive);
    }

    QColor RMVSettings::GetColorSnapshotGenerated() const
    {
        return GetColorValue(kSettingThemesAndColorsSnapshotGenerated);
    }

    QColor RMVSettings::GetColorSnapshotVMA() const
    {
        return GetColorValue(kSettingThemesAndColorsSnapshotVma);
    }

    QColor RMVSettings::GetColorResourceDepthStencil() const
    {
        return GetColorValue(kSettingThemesAndColorsResourceDsBuffer);
    }

    QColor RMVSettings::GetColorResourceRenderTarget() const
    {
        return GetColorValue(kSettingThemesAndColorsResourceRenderTarget);
    }

    QColor RMVSettings::GetColorResourceTexture() const
    {
        return GetColorValue(kSettingThemesAndColorsResourceTexture);
    }

    QColor RMVSettings::GetColorResourceVertexBuffer() const
    {
        return GetColorValue(kSettingThemesAndColorsResourceVertexBuffer);
    }

    QColor RMVSettings::GetColorResourceIndexBuffer() const
    {
        return GetColorValue(kSettingThemesAndColorsResourceIndexBuffer);
    }

    QColor RMVSettings::GetColorResourceRayTracingBuffer() const
    {
        return GetColorValue(kSettingThemesAndColorsResourceRayTracingBuffer);
    }

    QColor RMVSettings::GetColorResourceUAV() const
    {
        return GetColorValue(kSettingThemesAndColorsResourceUav);
    }

    QColor RMVSettings::GetColorResourceShaderPipeline() const
    {
        return GetColorValue(kSettingThemesAndColorsResourceShaderPipeline);
    }

    QColor RMVSettings::GetColorResourceCommandBuffer() const
    {
        return GetColorValue(kSettingThemesAndColorsResourceCommandBuffer);
    }

    QColor RMVSettings::GetColorResourceHeap() const
    {
        return GetColorValue(kSettingThemesAndColorsResourceHeap);
    }

    QColor RMVSettings::GetColorResourceDescriptors() const
    {
        return GetColorValue(kSettingThemesAndColorsResourceDescriptors);
    }

    QColor RMVSettings::GetColorResourceBuffer() const
    {
        return GetColorValue(kSettingThemesAndColorsResourceBuffer);
    }

    QColor RMVSettings::GetColorResourceGPUEvent() const
    {
        return GetColorValue(kSettingThemesAndColorsResourceGpuEvent);
    }

    QColor RMVSettings::GetColorResourceFreeSpace() const
    {
        return GetColorValue(kSettingThemesAndColorsResourceFreeSpace);
    }

    QColor RMVSettings::GetColorResourceInternal() const
    {
        return GetColorValue(kSettingThemesAndColorsResourceInternal);
    }

    QColor RMVSettings::GetColorDeltaIncrease() const
    {
        return GetColorValue(kSettingThemesAndColorsDeltaIncrease);
    }

    QColor RMVSettings::GetColorDeltaDecrease() const
    {
        return GetColorValue(kSettingThemesAndColorsDeltaDecrease);
    }

    QColor RMVSettings::GetColorDeltaNoChange() const
    {
        return GetColorValue(kSettingThemesAndColorsDeltaNoChange);
    }

    QColor RMVSettings::GetColorHeapLocal() const
    {
        return GetColorValue(kSettingThemesAndColorsHeapLocal);
    }

    QColor RMVSettings::GetColorHeapInvisible() const
    {
        return GetColorValue(kSettingThemesAndColorsHeapInvisible);
    }

    QColor RMVSettings::GetColorHeapSystem() const
    {
        return GetColorValue(kSettingThemesAndColorsHeapSystem);
    }

    QColor RMVSettings::GetColorHeapUnspecified() const
    {
        return GetColorValue(kSettingThemesAndColorsHeapUnspecified);
    }

    QColor RMVSettings::GetColorCPUMapped() const
    {
        return GetColorValue(kSettingThemesAndColorsCpuMapped);
    }

    QColor RMVSettings::GetColorNotCPUMapped() const
    {
        return GetColorValue(kSettingThemesAndColorsNotCpuMapped);
    }

    QColor RMVSettings::GetColorInPreferredHeap() const
    {
        return GetColorValue(kSettingThemesAndColorsInPreferredHeap);
    }

    QColor RMVSettings::GetColorNotInPreferredHeap() const
    {
        return GetColorValue(kSettingThemesAndColorsNotInPreferredHeap);
    }

    QColor RMVSettings::GetColorAliased() const
    {
        return GetColorValue(kSettingThemesAndColorsAliased);
    }

    QColor RMVSettings::GetColorNotAliased() const
    {
        return GetColorValue(kSettingThemesAndColorsNotAliased);
    }

    QColor RMVSettings::GetColorResourceHistoryResourceEvent() const
    {
        return GetColorValue(kSettingThemesAndColorsResourceHistoryResourceEvent);
    }

    QColor RMVSettings::GetColorResourceHistoryCpuMapping() const
    {
        return GetColorValue(kSettingThemesAndColorsResourceHistoryCpuMapUnmap);
    }

    QColor RMVSettings::GetColorResourceHistoryResidencyUpdate() const
    {
        return GetColorValue(kSettingThemesAndColorsResourceHistoryResidencyUpdate);
    }

    QColor RMVSettings::GetColorResourceHistoryPageTableUpdate() const
    {
        return GetColorValue(kSettingThemesAndColorsResourceHistoryPageTableUpdate);
    }

    QColor RMVSettings::GetColorResourceHistoryHighlight() const
    {
        return GetColorValue(kSettingThemesAndColorsResourceHistoryHighlight);
    }

    QColor RMVSettings::GetColorResourceHistorySnapshot() const
    {
        return GetColorValue(kSettingThemesAndColorsResourceHistorySnapshot);
    }

    QColor RMVSettings::GetColorCommitTypeCommitted() const
    {
        return GetColorValue(kSettingThemesAndColorsCommitTypeCommitted);
    }

    QColor RMVSettings::GetColorCommitTypePlaced() const
    {
        return GetColorValue(kSettingThemesAndColorsCommitTypePlaced);
    }

    QColor RMVSettings::GetColorCommitTypeVirtual() const
    {
        return GetColorValue(kSettingThemesAndColorsCommitTypeVirtual);
    }

    void RMVSettings::CycleTimeUnits()
    {
        TimeUnitType units = GetUnits();
        switch (units)
        {
        case kTimeUnitTypeClk:
            units = kTimeUnitTypeMillisecond;
            break;
        case kTimeUnitTypeMillisecond:
            units = kTimeUnitTypeSecond;
            break;
        case kTimeUnitTypeSecond:
            units = kTimeUnitTypeMinute;
            break;
        case kTimeUnitTypeMinute:
            units = kTimeUnitTypeHour;
            break;
        case kTimeUnitTypeHour:
        default:
            units = kTimeUnitTypeClk;
            break;
        }

        SetUnits(units);
        SaveSettings();
    }

}  // namespace rmv
