//=============================================================================
/// Copyright (c) 2019-2020 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief Header for a heap overview memory bar
//=============================================================================

#ifndef RMV_VIEWS_CUSTOM_WIDGETS_RMV_HEAP_OVERVIEW_MEMORY_BAR_H_
#define RMV_VIEWS_CUSTOM_WIDGETS_RMV_HEAP_OVERVIEW_MEMORY_BAR_H_

#include <QWidget>

#include "rmt_data_snapshot.h"

/// Support for the heap overview memory bar widget.
class RMVHeapOverviewMemoryBar : public QWidget
{
public:
    /// Constructor.
    /// \param parent Pointer to the parent widget.
    explicit RMVHeapOverviewMemoryBar(QWidget* parent = nullptr);

    /// Destructor.
    virtual ~RMVHeapOverviewMemoryBar();

    /// Provides a desired sizeHint that allows the text and bar to be visible.
    /// \return a size hint.
    virtual QSize sizeHint() const Q_DECL_OVERRIDE;

    /// Provides a minimum sizeHint that ensures that the text should always be visible.
    /// \return a minimum size hint.
    virtual QSize minimumSizeHint() const Q_DECL_OVERRIDE;

    /// Set the parameters for the memory bar. The first 3 size parameters' units don't matter, so
    /// long as they are all consistent (can be bytes, KB or pixels).
    /// The bar can be visualized in 3 sections:
    /// |xxxxxxxxxxxooooo     |
    /// The '|' respresent the total extent of the bar (max_size, below)
    /// The 'x's represent the normal bar data (size, below)
    /// The 'o's represent any extra data shown after the normal data (extra_data, below)
    /// \param size The size of the bar showing data.
    /// \param extra_size The size of the bar showing extra data. This is a value corresponding to just
    ///  the extra data, not the length of the bar from the start.
    /// \param max_size The maximum size of the bar.
    /// \param has_subscription Does this bar need to take into account memory subscription?
    ///  If so it will be colored based on its subscription status, otherwise it will be gray.
    /// \param subscription_status The current subscription status showing if the memory is oversubscribed or not.
    void SetParameters(uint64_t size, uint64_t extra_size, uint64_t max_size, bool has_subscription, RmtSegmentSubscriptionStatus subscription_status);

protected:
    /// Implementation of Qt's paint event.
    /// \param event The paint event.
    virtual void paintEvent(QPaintEvent* event) Q_DECL_OVERRIDE;

    /// Capture a resize event.
    /// \param event The resize event.
    virtual void resizeEvent(QResizeEvent* event) Q_DECL_OVERRIDE;

private:
    uint64_t                     size_;                 ///< Size of bar, in bytes.
    uint64_t                     extra_size_;           ///< Size of the hashed bit at the end.
    uint64_t                     max_size_;             ///< Max size of bar. Used to scale all bars.
    bool                         has_subscription_;     ///< Does this bar need subscription coloring.
    RmtSegmentSubscriptionStatus subscription_status_;  ///< Subscription(none, over, under, near). Will determine bar color.
};

#endif  // RMV_VIEWS_CUSTOM_WIDGETS_RMV_HEAP_OVERVIEW_MEMORY_BAR_H_
