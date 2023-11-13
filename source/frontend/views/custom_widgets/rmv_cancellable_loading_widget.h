//=============================================================================
/// Copyright (c) 2023 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief Header for a loading widget that has a cancel button
//=============================================================================

#ifndef RMV_VIEWS_CUSTOM_WIDGETS_RMV_CANCELLABLE_LOADING_WIDGET_H_
#define RMV_VIEWS_CUSTOM_WIDGETS_RMV_CANCELLABLE_LOADING_WIDGET_H_

#include <QWidget>

#include "qt_common/custom_widgets/file_loading_widget.h"
#include "qt_common/custom_widgets/scaled_push_button.h"

/// Class to handle the loading animation with cancel button.
class RmvCancellableLoadingWidget : public FileLoadingWidget
{
    Q_OBJECT

public:
    /// @brief Constructor.
    ///
    /// @param [in] parent     The animation widget's parent.
    /// @param [in] can_cancel If true, indicates that the user can cancel the operation by clicking a cancel button.
    RmvCancellableLoadingWidget(QWidget* parent = nullptr, const bool can_cancel = true);

    /// @brief Virtual destructor.
    virtual ~RmvCancellableLoadingWidget();

signals:
    /// @brief Notifies when the user clicks the cancel button.
    void CancelClicked();

private slots:
    /// @brief Handle when the user clicks the cancel button.
    ///
    /// @param [in] checked Not used.
    void HandleCancelClicked(bool checked = false);

protected:
    /// @brief Overridden resizeEvent handler.  Adjusts position of cancel button.
    ///
    /// @param [in] event The resize event.
    virtual void resizeEvent(QResizeEvent* event) Q_DECL_OVERRIDE;

private:
    ScaledPushButton* cancel_button_;  ///< The cancel button.
};

#endif  // RMV_VIEWS_CUSTOM_WIDGETS_RMV_CANCELLABLE_LOADING_WIDGET_H_
