//=============================================================================
/// Copyright (c) 2018-2020 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Implementation of rmv_util which holds useful utility functions.
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

#include "rmt_assert.h"

#include "settings/rmv_settings.h"
#include "util/version.h"

// Get the brightness of a given color. Adds weighting values to the color
// components to compute a color brightness. HSV won't work that well here
// as the brightness for 2 given hues may be different for identical
// saturation and value.
// Uses a standard luminance formula.
// \param background_color The color to calculate the brightness of.
// \return The brightness (0 is black, 255 is white).
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

    // make sure the folder exists. If not, create it
    std::string dir = file_location.toStdString();
    if (QDir(dir.c_str()).exists() == false)
    {
        QDir qdir;
        qdir.mkpath(dir.c_str());
    }

    return file_location;
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
        out = RMVSettings::Get().GetColorSnapshotViewed();
        break;
    case kSnapshotStateCompared:
        out = RMVSettings::Get().GetColorSnapshotCompared();
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
        out = RMVSettings::Get().GetColorDeltaIncrease();
        break;
    case kDeltaChangeDecrease:
        out = RMVSettings::Get().GetColorDeltaDecrease();
        break;
    case kDeltaChangeNone:
        out = RMVSettings::Get().GetColorDeltaNoChange();
        break;
    case kDeltaChangeCount:
        out = Qt::black;
        break;
    default:
        break;
    }

    return out;
}
