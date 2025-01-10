//=============================================================================
// Copyright (c) 2018-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Header for the carousel.
//=============================================================================

#ifndef RMV_VIEWS_CUSTOM_WIDGETS_RMV_CAROUSEL_H_
#define RMV_VIEWS_CUSTOM_WIDGETS_RMV_CAROUSEL_H_

#include <QWidget>
#include <QGraphicsScene>

#include "models/carousel_model.h"
#include "views/custom_widgets/rmv_carousel_item.h"
#include "views/custom_widgets/rmv_carousel_nav_button.h"

/// @brief Container class for a carousel.
class RMVCarousel : public QWidget
{
    Q_OBJECT

public:
    /// @brief Constructor.
    ///
    /// @param [in] config The configuration parameters.
    explicit RMVCarousel(const RMVCarouselConfig& config);

    /// @brief Destructor.
    virtual ~RMVCarousel();

    /// @brief Overridden window resize event.
    ///
    /// @param [in] width  The new width.
    /// @param [in] height The new height.
    void ResizeEvent(int width, int height);

    /// @brief Get the graphics scene for the carousel.
    ///
    /// @return The carousel graphics scene.
    QGraphicsScene* Scene() const;

    /// @brief Clear out the data.
    void ClearData();

    /// @brief Update model for a single snapshot.
    void UpdateModel();

    /// @brief Update model for 2 compared snapshots.
    ///
    /// @param [in] base_snapshot The first (base) snapshot.
    /// @param [in] diff_snapshot The second snapshot to compare against the first.
    void UpdateModel(RmtDataSnapshot* base_snapshot, RmtDataSnapshot* diff_snapshot);

private slots:
    /// @brief Move the carousel.
    ///
    /// @param [in] left_direction If true, move left, otherwise move right.
    void MoveCarousel(bool left_direction);

private:
    /// @brief Set the UI data for the individual carousel items.
    ///
    /// @param [in] carousel_data The information needed by the carousel.
    void SetData(const rmv::RMVCarouselData& carousel_data);

    /// @brief Template function to create a new carousel item of a certain type.
    ///
    /// Also saves the base pointer to an array.
    ///
    /// @param [in] config The carousel item configuration.
    ///
    /// @return The new carousel item created.
    template <class CarouselItemType>
    CarouselItemType* CreateCarouselItem(const RMVCarouselConfig& config)
    {
        CarouselItemType* item = new CarouselItemType(config);
        scene_->addItem(item);
        carousel_items_.push_back(item);
        return item;
    }

    /// @brief Refresh the carousel.
    void Update();

    RMVCarouselConfig         config_;            ///< The initial configuration parameters for the carousel.
    QGraphicsScene*           scene_;             ///< Pointer to the graphics scene.
    RMVCarouselNavButton*     left_nav_button_;   ///< The left button graphic.
    RMVCarouselNavButton*     right_nav_button_;  ///< The right button graphic.
    QVector<RMVCarouselItem*> carousel_items_;    ///< The list of carousel items.
    QGraphicsTextItem*        info_text_;         ///< The info text showing the current carousel index.
    int32_t                   carousel_index_;    ///< The current carousel index.
    rmv::CarouselModel*       model_;             ///< The carousel model which interfaces with the backend.
};

#endif  // RMV_VIEWS_CUSTOM_WIDGETS_RMV_CAROUSEL_H_
