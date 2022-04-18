//=============================================================================
// Copyright (c) 2019-2022 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Header of the name delegate for the snapshot table.
//=============================================================================

#ifndef RMV_VIEWS_DELEGATES_RMV_SNAPSHOT_NAME_DELEGATE_H_
#define RMV_VIEWS_DELEGATES_RMV_SNAPSHOT_NAME_DELEGATE_H_

#include <QStyledItemDelegate>

/// Support for the snapshot name delegate.
class RMVSnapshotNameDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    /// @brief Constructor.
    ///
    /// @param [in] parent The delegate's parent.
    explicit RMVSnapshotNameDelegate(QWidget* parent = nullptr);

    /// @brief Destructor.
    virtual ~RMVSnapshotNameDelegate();

    /// @brief Override to handle creating an editor widget.
    ///
    /// @param [in] parent The delegate's parent.
    /// @param [in] option The style properties for the editor widget.
    /// @param [in] index  The model index requesting an edit widget.
    /// 
    /// @return The edit widget.
    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const Q_DECL_OVERRIDE;

    /// @brief Override to update the model with data from the edit widget.
    ///
    /// @param [in] editor The edit widget.
    /// @param [in] model The model to be updated.
    /// @param [in] index The index of the cell in the model to be updated.
    virtual void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const Q_DECL_OVERRIDE;
};

#endif  // RMV_VIEWS_DELEGATES_RMV_SNAPSHOT_NAME_DELEGATE_H_
