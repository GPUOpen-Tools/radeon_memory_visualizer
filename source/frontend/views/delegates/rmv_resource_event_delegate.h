//=============================================================================
/// Copyright (c) 2019-2020 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief Header for the resource event delegate.
//=============================================================================

#ifndef RMV_VIEWS_DELEGATES_RMV_RESOURCE_EVENT_DELEGATE_H_
#define RMV_VIEWS_DELEGATES_RMV_RESOURCE_EVENT_DELEGATE_H_

#include <QStyledItemDelegate>

#include "models/snapshot/resource_details_model.h"
#include "views/snapshot/resource_event_icons.h"

/// Support for the resource event delegate. This does the custom painting
/// in the resource timeline table in the resource details view.
class RMVResourceEventDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    /// Default width and height of the icon sizeHint.
    /// They are the same value since it draws within a square; this includes
    /// a small padding around the actual icon.
    static const double kIconDefaultSizeHint;

    /// The icon size factor relative to the height of the available rect.
    static const double kIconSizeFactor;

    /// Constructor.
    /// \param parent The delegate's parent.
    /// \param model The model containing the resource details.
    explicit RMVResourceEventDelegate(QWidget* parent, rmv::ResourceDetailsModel* model);

    /// Destructor.
    ~RMVResourceEventDelegate();

    /// Overridden sizeHint method.
    /// \param option Style options related to this element.
    /// \param index The model index for this element.
    /// \return The desired size needed to paint this element.
    virtual QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const Q_DECL_OVERRIDE;

    /// Overridden delegate paint method.
    /// \param painter Pointer to the painter object.
    /// \param option A QStyleOptionViewItem object, which contains the bounding rectangle for this element.
    /// \param index The model index for this element.
    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const Q_DECL_OVERRIDE;

private:
    rmv::ResourceEventIcons    event_icons_;  ///< The icon painter helper object.
    rmv::ResourceDetailsModel* model_;        ///< The model containing the resource details information.
};

#endif  // RMV_VIEWS_DELEGATES_RMV_RESOURCE_EVENT_DELEGATE_H_
