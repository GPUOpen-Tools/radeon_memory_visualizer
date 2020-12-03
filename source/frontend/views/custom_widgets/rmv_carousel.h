//=============================================================================
/// Copyright (c) 2018-2020 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Header for RMV's carousel
//=============================================================================

#ifndef RMV_VIEWS_CUSTOM_WIDGETS_RMV_CAROUSEL_H_
#define RMV_VIEWS_CUSTOM_WIDGETS_RMV_CAROUSEL_H_

#include <QWidget>
#include <QGraphicsScene>

#include "rmt_data_set.h"

#include "models/carousel_model.h"
#include "views/custom_widgets/rmv_carousel_item.h"
#include "views/custom_widgets/rmv_carousel_nav_button.h"

/// Container class for a carousel.
class RMVCarousel : public QWidget
{
    Q_OBJECT

public:
    /// Constructor.
    /// \param config The configuration parameters.
    explicit RMVCarousel(const RMVCarouselConfig& config);

    /// Destructor.
    virtual ~RMVCarousel();

    /// Overridden window resize event.
    /// \param width The new width.
    /// \param height The new height.
    void ResizeEvent(int width, int height);

    /// Get the graphics scene for the carousel.
    /// \return The carousel graphics scene.
    QGraphicsScene* Scene();

    /// Clear out the data.
    void ClearData();

    /// Update model for a single snapshot.
    void UpdateModel();

    /// Update model for 2 compared snapshots.
    /// \param base_snapshot The first (base) snapshot.
    /// \param diff_snapshot The second snapshot to compare against the first.
    void UpdateModel(RmtDataSnapshot* base_snapshot, RmtDataSnapshot* diff_snapshot);

private slots:
    /// Move the carousel.
    /// \param left_direction If true, move left, otherwise move right.
    void MoveCarousel(bool left_direction);

private:
    /// set the UI data for the individual carousel items.
    /// \param carousel_data The information needed by the carousel.
    void SetData(const RMVCarouselData& carousel_data);

    /// Template function to create a new carousel item of a certain type.
    /// Also saves the base pointer to an array.
    template <class CarouselItemType>
    CarouselItemType* CreateCarouselItem(RMVCarouselConfig& config)
    {
        CarouselItemType* item = new CarouselItemType(config);
        scene_->addItem(item);
        carousel_items_.push_back(item);
        return item;
    }

    /// Refresh the carousel.
    void Update();

    RMVCarouselConfig         config_;            ///< The initial configuration parameters for the carousel.
    QGraphicsScene*           scene_;             ///< Pointer to the graphics scene.
    RMVCarouselNavButton*     left_nav_button_;   ///< The left button graphic.
    RMVCarouselNavButton*     right_nav_button_;  ///< The right button graphic.
    QVector<RMVCarouselItem*> carousel_items_;    ///< The list of carousel items.
    QGraphicsTextItem*        info_text_;         ///< The info text showing the current carousel index.
    int32_t                   carousel_index_;    ///< The current carousel index.
    CarouselModel*            model_;             ///< The carousel model which interfaces with the backend.
};

#endif  // RMV_VIEWS_CUSTOM_WIDGETS_RMV_CAROUSEL_H_
