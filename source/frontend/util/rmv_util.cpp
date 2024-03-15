//=============================================================================
// Copyright (c) 2018-2024 Advanced Micro Devices, Inc. All rights reserved.
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

static const int32_t kThresholdStepOffset =
    7;  ///< Used to adjust the unscaled step value when calculating scaled threshold value.

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

    file_location = QString::fromUtf16((const char16_t*)wsz_path);
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

        if ((extension.compare(kRMVTraceFileExtension, Qt::CaseInsensitive) == 0) || (extension.compare(kRGDTraceFileExtension, Qt::CaseInsensitive) == 0))
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

uint64_t rmv_util::CalculateSizeThresholdFromStepValue(const uint32_t step_value, const uint32_t max_steps)
{
    if (step_value == 0)
    {
        return 0;
    }

    if (step_value >= max_steps)
    {
        return UINT64_MAX;
    }

    // Calculate a threshold value ranging from 256 to 1073741824 (1 GB).  The lowest step value is 1.
    // The threshold offset increases the first step to 8 so that 2 raised to the power of 8 results in a value of 256.
    return pow(2, step_value + kThresholdStepOffset);
}

QString rmv_util::GetVirtualAllocationName(const RmtVirtualAllocation* virtual_allocation)
{
    QString allocation_name;
    if (virtual_allocation != nullptr)
    {
        if (virtual_allocation->name != nullptr)
        {
            allocation_name = QString("'%1'").arg(virtual_allocation->name);
        }
        else
        {
            allocation_name = QString("0x") + QString::number(virtual_allocation->base_address, 16);
        }
    }
    else
    {
        allocation_name = "Orphaned";
    }

    return allocation_name;
}
