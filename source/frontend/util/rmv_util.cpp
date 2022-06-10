//=============================================================================
// Copyright (c) 2018-2021 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Implementation of rmv_util which holds useful utility functions.
//=============================================================================

#include "util/rmv_util.h"

#ifdef _WIN32
#include <Windows.h>
#include <Shlobj.h>
#else
#include <unistd.h>
#include <sys/stat.h>
#include <pwd.h>

#include <linux/safe_crt.h>
#endif

#include <QDir>
#include <cmath>

#include "rmt_assert.h"

#include "settings/rmv_settings.h"
#include "util/version.h"

/// @brief Get the brightness of a given color.
///
/// Adds weighting values to the color components to compute a color brightness. HSV won't work that well here
/// as the brightness for 2 given hues may be different for identical saturation and value.
/// Uses a standard luminance formula.
///
/// @param [in] background_color The color to calculate the brightness of.
///
/// @return The brightness (0 is black, 255 is white).
static int GetColorBrightness(const QColor& background_color)
{
    double r          = background_color.red();
    double g          = background_color.green();
    double b          = background_color.blue();
    double brightness = (0.3 * r) + (0.59 * g) + 0.11 * b;

    return static_cast<int>(brightness);
}

QColor rmv_util::GetTextColorForBackground(const QColor& background_color, bool has_white_background)
{
    int brightness = GetColorBrightness(background_color);
    if (brightness > 128)
    {
        return Qt::black;
    }
    else
    {
        if (has_white_background)
        {
            return Qt::lightGray;
        }
        return Qt::white;
    }
}

QString rmv_util::GetFileLocation()
{
    QString file_location = "";

#ifdef _WIN32
    LPWSTR  wsz_path = nullptr;
    HRESULT hr       = SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, nullptr, &wsz_path);
    Q_UNUSED(hr);
    Q_ASSERT(hr == S_OK);

    file_location = QString::fromUtf16((const ushort*)wsz_path);
    file_location.append("/" + rmv::kRmvExecutableBaseFilename);

#else

    struct passwd* pw = getpwuid(getuid());
    if (pw != nullptr)
    {
        const char* homedir = pw->pw_dir;
        file_location       = homedir;
    }
    file_location.append("/." + rmv::kRmvExecutableBaseFilename);
#endif

    // Make sure the folder exists. If not, create it.
    std::string dir = file_location.toStdString();
    if (QDir(dir.c_str()).exists() == false)
    {
        QDir qdir;
        qdir.mkpath(dir.c_str());
    }

    return file_location;
}

bool rmv_util::TraceValidToLoad(const QString& trace_path)
{
    bool may_load = false;

    QFileInfo trace_file(trace_path);
    if (trace_file.exists() && trace_file.isFile())
    {
        const QString extension = trace_path.mid(trace_path.lastIndexOf("."), trace_path.length());

        if (extension.compare(rmv::text::kTraceFileExtension, Qt::CaseInsensitive) == 0)
        {
            may_load = true;
        }
    }

    return may_load;
}

QColor rmv_util::GetSnapshotStateColor(SnapshotState state)
{
    QColor out = Qt::black;

    switch (state)
    {
    case kSnapshotStateNone:
        out = Qt::black;
        break;
    case kSnapshotStateViewed:
        out = rmv::RMVSettings::Get().GetColorSnapshotViewed();
        break;
    case kSnapshotStateCompared:
        out = rmv::RMVSettings::Get().GetColorSnapshotCompared();
        break;
    case kSnapshotStateCount:
        out = Qt::black;
        break;
    default:
        break;
    }

    return out;
}

QColor rmv_util::GetDeltaChangeColor(DeltaChange delta)
{
    QColor out = Qt::black;

    switch (delta)
    {
    case kDeltaChangeIncrease:
        out = rmv::RMVSettings::Get().GetColorDeltaIncrease();
        break;
    case kDeltaChangeDecrease:
        out = rmv::RMVSettings::Get().GetColorDeltaDecrease();
        break;
    case kDeltaChangeNone:
        out = rmv::RMVSettings::Get().GetColorDeltaNoChange();
        break;
    case kDeltaChangeCount:
        out = Qt::black;
        break;
    default:
        break;
    }

    return out;
}

void rmv_util::BuildResourceSizeThresholds(std::vector<uint64_t>& resource_sizes, uint64_t* resource_thresholds)
{
    if (resource_sizes.size() == 0)
    {
        return;
    }

    std::stable_sort(resource_sizes.begin(), resource_sizes.end());

    float step_size = (resource_sizes.size() - 1) / static_cast<float>(rmv::kSizeSliderRange);
    float index     = 0.0F;
    for (int loop = 0; loop <= rmv::kSizeSliderRange; loop++)
    {
        resource_thresholds[loop] = resource_sizes[static_cast<int>(round(index))];
        index += step_size;
    }
}
