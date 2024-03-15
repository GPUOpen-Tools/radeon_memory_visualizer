//=============================================================================
// Copyright (c) 2019-2024 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Header for a resource timeline widget.
//=============================================================================

#ifndef RMV_VIEWS_CUSTOM_WIDGETS_RMV_RESOURCE_TIMELINE_H_
#define RMV_VIEWS_CUSTOM_WIDGETS_RMV_RESOURCE_TIMELINE_H_

#include <QWidget>

#include "models/snapshot/resource_details_model.h"
#include "views/snapshot/resource_event_icons.h"

/// @brief Support for the resource timeline graphics item widget.
class RMVResourceTimeline : public QWidget
{
    Q_OBJECT

public:
    /// @brief Constructor.
    ///
    /// @param [in] parent The parent widget.
    explicit RMVResourceTimeline(QWidget* parent);

    /// @brief Destructor.
    virtual ~RMVResourceTimeline();

    /// @brief Implementation of Qt's sizeHint for this widget.
    ///
    /// @return A default sizeHint since the size of this widget can grow to fit the space allowed by the layout.
    virtual QSize sizeHint() const Q_DECL_OVERRIDE;

    /// @brief Initialize the widget with non-default values.
    ///
    /// @param [in] model The model where the timeline data is stored.
    void Initialize(rmv::ResourceDetailsModel* model);

signals:
    /// @brief Indicate that the timeline was clicked on.
    ///
    /// @param [in] logical_position The logical position on the timeline clicked on.
    ///  The absolute position is converted to a logical position between 0 and 1.
    ///  A value of 0.5 would be half way along the timeline.
    /// @param [in] tolerance A factor around the logical_position still considered to
    ///  be valid. This should allow for the size of the icon. Tolerance is on the
    ///  same scale as the logical position.
    void TimelineSelected(double logical_position, double tolerance);

protected:
    /// @brief Implementation of Qt's paint for this widget.
    ///
    /// @param [in] paint_event The paint event.
    virtual void paintEvent(QPaintEvent* paint_event) Q_DECL_OVERRIDE;

    /// @brief Implementation of Qt's mouse press event for this widget.
    ///
    /// @param [in] event The mouse press event.
    virtual void mousePressEvent(QMouseEvent* event) Q_DECL_OVERRIDE;

private:
    rmv::ResourceDetailsModel* model_;        ///< Pointer to the model data.
    rmv::ResourceEventIcons    event_icons_;  ///< The icon painter helper object.
};

#endif  // RMV_VIEWS_CUSTOM_WIDGETS_RMV_RESOURCE_TIMELINE_H_
