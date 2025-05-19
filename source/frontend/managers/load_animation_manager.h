//==============================================================================
// Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Class definition for the file loading animation manager.
///
/// This class is responsible for managing the file load animation when loading
/// a trace or data-mining the trace file (ie getting resource details or
/// generating the timeline).
///
//==============================================================================

#ifndef RMV_MANAGERS_LOAD_ANIMATION_MANAGER_H_
#define RMV_MANAGERS_LOAD_ANIMATION_MANAGER_H_

#include <QMenu>
#include <QObject>

#include "qt_common/custom_widgets/tab_widget.h"

#include "views/custom_widgets/rmv_cancellable_loading_widget.h"

namespace rmv
{
    /// @brief This class handles the trace loading animation.
    class LoadAnimationManager : public QObject
    {
        Q_OBJECT

    public:
        /// @brief Constructor.
        ///
        /// @param [in] parent The parent widget.
        explicit LoadAnimationManager(QObject* parent = nullptr);

        /// @brief Destructor.
        virtual ~LoadAnimationManager();

        /// @brief Accessor for singleton instance.
        ///
        /// @return The singleton instance.
        static LoadAnimationManager& Get();

        /// @brief Initialize the animation manager.
        ///
        /// @param [in] tab_widget The tab widget from the main window.
        void Initialize(TabWidget* tab_widget);

        /// @brief Start the loading animation.
        ///
        /// Called when an animation needs to be loaded onto a window.
        ///
        /// @param [in] parent        The parent window.
        /// @param [in] height_offset The offset from the top of the parent widget.
        /// @param [in] can_cancel    If true, a cancel button is added allowing the user to abort.
        void StartAnimation(QWidget* parent, int height_offset, const bool can_cancel = false);

        /// @brief Start the loading animation.
        ///
        /// Called when an animation needs to be loaded onto a window.
        ///
        /// The tab widget is used as the parent window.
        void StartAnimation();

        /// @brief Stop the loading animation.
        ///
        /// Called when trace file has loaded.
        void StopAnimation();

        /// @brief Resize the loading animation.
        ///
        /// Called when the main window is resized.
        /// Make sure that the load animation is also resized.
        void ResizeAnimation();

    signals:
        /// @brief Indicates that the animation was cancelled by the user.
        void AnimationCancelled();

    private:
        /// @brief Resize the loading animation.
        ///
        /// @param [in] parent        The parent window.
        /// @param [in] height_offset The offset from the top of the parent widget.
        void Resize(QWidget* parent, int height_offset);

        TabWidget*                   tab_widget_;           ///< The tab widget from the main window.
        QMenu*                       file_menu_;            ///< The file menu widget from the main window.
        RmvCancellableLoadingWidget* file_load_animation_;  ///< Widget to show animation.
    };
}  // namespace rmv

#endif  // RMV_MANAGERS_LOAD_ANIMATION_MANAGER_H_
