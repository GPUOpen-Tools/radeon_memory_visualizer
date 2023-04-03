//=============================================================================
// Copyright (c) 2019-2023 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Implementation of the name delegate for the snapshot table.
//=============================================================================

#include "rmv_snapshot_name_delegate.h"

#include <QLineEdit>

#include "models/timeline/snapshot_item_model.h"
#include "rmv_regular_expression_validator.h"

// Regular Expression to filter non-printable characters and limit the snapshot name hlength to 32 characters.
static const QString kSnapshotNameRegEx("[ -~]{1,32}");

// Tooltip message displayed when user enters and invalid character for a snapshot name.
static const QString kInvalidSnapshotNameMessage("Snapshot names must contain only printable characters and must be 32 characters or less.");

RMVSnapshotNameDelegate::RMVSnapshotNameDelegate(QWidget* parent)
    : QStyledItemDelegate(parent)
{
}

RMVSnapshotNameDelegate::~RMVSnapshotNameDelegate()
{
}

QWidget* RMVSnapshotNameDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);

    QLineEdit* line_edit = new QLineEdit(parent);
    if (line_edit != nullptr)
    {
        RMVRegularExpressionValidator* validator =
            new RMVRegularExpressionValidator((QRegularExpression(kSnapshotNameRegEx, QRegularExpression::CaseInsensitiveOption)), line_edit);
        validator->SetInvalidInputMessage(kInvalidSnapshotNameMessage);
        line_edit->setValidator(validator);
    }

    return line_edit;
}

void RMVSnapshotNameDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
    QLineEdit* line_edit = qobject_cast<QLineEdit*>(editor);
    if (line_edit != nullptr)
    {
        QString value = line_edit->text().trimmed();
        model->setData(index, value, Qt::EditRole);
    }
}
