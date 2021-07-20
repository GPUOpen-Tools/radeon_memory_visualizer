//=============================================================================
// Copyright (c) 2020-2021 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Implementation of a number of string utilities.
//=============================================================================

#include "string_util.h"

#include <QtMath>
#include <QTextStream>

#include "qt_common/utils/qt_util.h"

QString rmv::string_util::ToUpperCase(const QString& string)
{
    QString out;

    for (int i = 0; i < string.length(); i++)
    {
        char c = string.at(i).toLatin1();
        out.append(c >= 'a' && c <= 'z' ? c - 32 : c);
    }

    return out;
}

QString rmv::string_util::Convert128BitHashToString(uint64_t upper_bits, uint64_t lower_bits)
{
    QString out = "";

    if (upper_bits == 0 && lower_bits == 0)
    {
        out = "N/A";
    }
    else
    {
        if (upper_bits == 0)
        {
            out = "0x" + QtCommon::QtUtils::HashToStr(lower_bits);
        }
        else
        {
            out = "0x" + QtCommon::QtUtils::HashToStr(upper_bits) + QtCommon::QtUtils::HashToStr(lower_bits);
        }
    }

    return out;
}

QString rmv::string_util::LocalizedValue(int64_t value)
{
    QString     str = "";
    QTextStream out(&str);
    out.setRealNumberNotation(QTextStream::FixedNotation);
    out.setLocale(QLocale::English);
    out << value;
    return str;
}

QString rmv::string_util::LocalizedValuePrecise(double value)
{
    QString     str = "";
    QTextStream out(&str);
    out.setRealNumberPrecision(2);
    out.setRealNumberNotation(QTextStream::FixedNotation);
    out.setLocale(QLocale::English);
    out << value;
    return str;
}

QString rmv::string_util::LocalizedValueMemory(double value, bool base_10, bool use_round)
{
    double multiple;
    if (base_10)
    {
        multiple = 1000;
    }
    else
    {
        multiple = 1024;
    }

    double scaled_size = value;

    int postfix_index = 0;
    while (fabs(scaled_size) >= multiple)
    {
        scaled_size /= multiple;
        postfix_index++;
    }

    if (use_round)
    {
        scaled_size = round(scaled_size);
    }

    static const QString kBinarySizePostfix[] = {" bytes", " KiB", " MiB", " GiB", " TiB", " PiB"};
    static const QString kBase10SizePostfix[] = {" bytes", " KB", " MB", " GB", " TB", " PB"};

    // If index is too large, it's probably down to bad data so display as bytes in this case.
    if (postfix_index >= 6)
    {
        postfix_index = 0;
        scaled_size   = value;
    }

    // Display value string to 2 decimal places if not bytes. No fractional part for bytes.
    QString value_string;
    if (postfix_index != 0)
    {
        value_string = LocalizedValuePrecise(scaled_size);
    }
    else
    {
        value_string = LocalizedValue(scaled_size);
    }

    if (base_10)
    {
        return value_string + kBase10SizePostfix[postfix_index];
    }
    else
    {
        return value_string + kBinarySizePostfix[postfix_index];
    }
}

QString rmv::string_util::LocalizedValueAddress(RmtGpuAddress address)
{
    QString str = "0x" + QString::number(address, 16);
    return str;
}

QString rmv::string_util::LocalizedValueBytes(int64_t value)
{
    return rmv::string_util::LocalizedValue(value) + " bytes";
}

QString rmv::string_util::GetResourceUsageString(RmtResourceUsageType usage_type)
{
    QString out = "";

    switch (usage_type)
    {
    case kRmtResourceUsageTypeDepthStencil:
        out = "Depth stencil buffer";
        break;
    case kRmtResourceUsageTypeRenderTarget:
        out = "Render target";
        break;
    case kRmtResourceUsageTypeTexture:
        out = "Texture";
        break;
    case kRmtResourceUsageTypeVertexBuffer:
        out = "Vertex buffer";
        break;
    case kRmtResourceUsageTypeIndexBuffer:
        out = "Index buffer";
        break;
    case kRmtResourceUsageTypeUav:
        out = "UAV";
        break;
    case kRmtResourceUsageTypeShaderPipeline:
        out = "Shader pipeline";
        break;
    case kRmtResourceUsageTypeCommandBuffer:
        out = "Command buffer";
        break;
    case kRmtResourceUsageTypeHeap:
        out = "Heap";
        break;
    case kRmtResourceUsageTypeDescriptors:
        out = "Descriptors";
        break;
    case kRmtResourceUsageTypeBuffer:
        out = "Multi-use buffer";
        break;
    case kRmtResourceUsageTypeFree:
        out = "Unbound";
        break;
    case kRmtResourceUsageTypeGpuEvent:
        out = "Event";
        break;
    case kRmtResourceUsageTypeInternal:
        out = "Internal";
        break;

    default:
        out = "Unknown";
        break;
    }

    return out;
}
