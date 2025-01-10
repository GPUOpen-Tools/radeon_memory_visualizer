//=============================================================================
// Copyright (c) 2024-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief File containing tree structures and functions.
//=============================================================================

#ifndef RMT_BACKEND_RMT_TREE_H_
#define RMT_BACKEND_RMT_TREE_H_

#include <algorithm>
#include <iostream>
#include <limits>
#include <memory>
#include <queue>
#include <string>
#include <unordered_map>
#include <vector>

/// rmt_tree is used to precisely calculate overlapping ranges
/// for any given range interval. rmt_tree is self-balancing (with AVL) and
/// intended to reduce the range overlapping searching complexity.
namespace rmt_tree
{
    /// @brief Struct defining an interval.
    template <typename T1, typename T2>
    struct Interval
    {
        T1 start;  ///< Starting value of interval.
        T1 end;    ///< Ending value of interval.
        T2 index;  ///< Index of interval.
    };

    /// @brief Class defining a tree node.
    template <typename T1, typename T2>
    class IntervalTreeNode
    {
    public:
        /// @brief Constructor.
        inline IntervalTreeNode()
            : left_(nullptr)
            , right_(nullptr)
            , max_((std::numeric_limits<T1>::min)())
            , height_(1)
        {
        }

        /// @brief Constructor.
        ///
        /// @param [in] interval Current interval.
        inline IntervalTreeNode(const Interval<T1, T2>& interval)
            : left_(nullptr)
            , right_(nullptr)
            , interval_(interval)
            , max_(interval.end)
            , height_(1)
        {
        }

        std::shared_ptr<IntervalTreeNode> left_;      ///< Left child node.
        std::shared_ptr<IntervalTreeNode> right_;     ///< Right child node.
        Interval<T1, T2>                  interval_;  ///< Current interval.
        T1                                max_;       ///< Max interval ending value for current subtree.
        size_t                            height_;    ///< Tree height.
    };

    /// @brief Class defining the interval tree.
    template <typename T1, typename T2>
    class IntervalTree
    {
    public:
        /// @brief Constructor.
        inline IntervalTree()
            : root_(nullptr)
        {
        }

        /// @brief Destructor.
        inline ~IntervalTree()
        {
        }

        /// @brief Insert an interval into the tree.
        ///
        /// @param [in] interval Target interval.
        inline void Insert(const Interval<T1, T2>& interval)
        {
            InsertRecursive(root_, interval);
        }

        /// @brief Find all overlapping intervals for a given interval.
        ///
        /// @param [in] interval              Target interval.
        /// @param [in] overlapping_intervals Vector containing all overlapping intervals.
        inline void FindOverlappingIntervals(const Interval<T1, T2>& interval, std::vector<Interval<T1, T2>>& overlapping_intervals) const
        {
            FindOverlappingIntervalsRecursive(root_, interval, overlapping_intervals, false);
        }

        /// @brief Find all overlapping intervals for a given interval.
        ///
        /// @param [in] interval              Target interval.
        /// @param [in] overlapping_intervals Queue containing all overlapping intervals.
        template <typename CompT>
        inline void FindOverlappingIntervals(const Interval<T1, T2>&                                                      interval,
                                             std::priority_queue<Interval<T1, T2>, std::vector<Interval<T1, T2>>, CompT>& overlapping_intervals) const
        {
            FindOverlappingIntervalsRecursive(root_, interval, overlapping_intervals, false);
        }

        /// @brief Merge all overlapping intervals into non-overlapping intervals.
        ///
        /// @param [in] intervals             Queue of intervals.
        /// @param [in] overlapping_intervals Queue containing merged non-overlapping intervals.
        template <typename CompT>
        inline static void MergeAscendingOrderIntervals(std::priority_queue<Interval<T1, T2>, std::vector<Interval<T1, T2>>, CompT>& intervals,
                                                        std::vector<Interval<T1, T2>>&                                               merged_intervals)
        {
            if (!intervals.empty())
            {
                merged_intervals.push_back(intervals.top());
                intervals.pop();

                while (!intervals.empty())
                {
                    if (intervals.top().start > merged_intervals.back().end)
                    {
                        merged_intervals.push_back(intervals.top());
                    }
                    else
                    {
                        merged_intervals.back().end = std::max(intervals.top().end, merged_intervals.back().end);
                    }

                    intervals.pop();
                }
            }
        }

        /// @brief Find all overlapping intervals for a given interval.
        ///
        /// The returned intervals will be culled based on their overlapping parts
        /// with the given interval.
        ///
        /// @param [in] interval                     Target interval.
        /// @param [in] culled_overlapping_intervals Vector containing all culled overlapping intervals.
        inline void FindCulledOverlappingIntervals(const Interval<T1, T2>& interval, std::vector<Interval<T1, T2>>& culled_overlapping_intervals) const
        {
            FindOverlappingIntervalsRecursive(root_, interval, culled_overlapping_intervals, true);
        }

        /// @brief Helper function to print the tree via pre-order traversal.
        inline void PrintPreOrder()
        {
            PrintPreOrderDfs(root_);

            std::cout << "\n";
        }

    protected:
        /// @brief Helper function to traverse the tree in pre-order traversal.
        ///
        /// @param [in] current_node The current tree node.
        inline void PrintPreOrderDfs(const std::shared_ptr<IntervalTreeNode<T1, T2>>& current_node)
        {
            if (current_node == nullptr)
            {
                return;
            }

            std::cout << "[" << current_node->interval_.start << "," << current_node->interval_.end << "] ";

            PrintPreOrderDfs(current_node->left_);
            PrintPreOrderDfs(current_node->right_);
        }

        /// @brief Get the tree node height difference between its left and right child node.
        ///
        /// @param [in] current_node The current tree node.
        inline int32_t GetNodeHeightDifference(const std::shared_ptr<IntervalTreeNode<T1, T2>>& current_node)
        {
            if (current_node != nullptr)
            {
                int32_t height_diff = static_cast<int32_t>(GetNodeHeight(current_node->left_)) - static_cast<int32_t>(GetNodeHeight(current_node->right_));
                return height_diff;
            }
            else
            {
                return 0;
            }
        }

        /// @brief Get the node height.
        ///
        /// @param [in] current_node The current tree node.
        inline size_t GetNodeHeight(const std::shared_ptr<IntervalTreeNode<T1, T2>>& current_node)
        {
            if (current_node != nullptr)
            {
                return current_node->height_;
            }
            else
            {
                return 0;
            }
        }

        /// @brief Get the max interval ending value for current subtree.
        ///
        /// @param [in] current_node The current tree node.
        inline T1 GetNodeMax(const std::shared_ptr<IntervalTreeNode<T1, T2>>& current_node)
        {
            if (current_node != nullptr)
            {
                return current_node->max_;
            }
            else
            {
                return (std::numeric_limits<T1>::min)();
            }
        }

        /// @brief Right rotation of the current tree node.
        ///
        /// @param [in] current_node The current tree node.
        inline void RightRotate(std::shared_ptr<IntervalTreeNode<T1, T2>>& current_node)
        {
            auto current_left       = current_node->left_;
            auto current_left_right = current_left->right_;

            current_left->right_ = current_node;
            current_node->left_  = current_left_right;

            current_node->height_ = 1 + std::max(GetNodeHeight(current_node->left_), GetNodeHeight(current_node->right_));
            current_left->height_ = 1 + std::max(GetNodeHeight(current_left->left_), GetNodeHeight(current_left->right_));

            current_node->max_ = std::max(current_node->interval_.end, GetNodeMax(current_node->left_));
            current_node->max_ = std::max(current_node->max_, GetNodeMax(current_node->right_));
            current_left->max_ = std::max(current_left->interval_.end, GetNodeMax(current_left->left_));
            current_left->max_ = std::max(current_left->max_, GetNodeMax(current_left->right_));

            current_node = current_left;
        }

        /// @brief Left rotation of the current tree node.
        ///
        /// @param [in] current_node The current tree node.
        inline void LeftRotate(std::shared_ptr<IntervalTreeNode<T1, T2>>& current_node)
        {
            auto current_right      = current_node->right_;
            auto current_right_left = current_right->left_;

            current_right->left_ = current_node;
            current_node->right_ = current_right_left;

            current_node->height_  = 1 + std::max(GetNodeHeight(current_node->left_), GetNodeHeight(current_node->right_));
            current_right->height_ = 1 + std::max(GetNodeHeight(current_right->left_), GetNodeHeight(current_right->right_));

            current_node->max_  = std::max(current_node->interval_.end, GetNodeMax(current_node->left_));
            current_node->max_  = std::max(current_node->max_, GetNodeMax(current_node->right_));
            current_right->max_ = std::max(current_right->interval_.end, GetNodeMax(current_right->left_));
            current_right->max_ = std::max(current_right->max_, GetNodeMax(current_right->right_));

            current_node = current_right;
        }

        /// @brief Insert an interval into the tree.
        ///
        /// @param [in] current_node The current tree node.
        /// @param [in] interval     Target interval.
        inline void InsertRecursive(std::shared_ptr<IntervalTreeNode<T1, T2>>& current_node, const Interval<T1, T2>& interval)
        {
            if (current_node == nullptr)
            {
                current_node.reset(new IntervalTreeNode<T1, T2>(interval));
            }
            else
            {
                T1 start = interval.start;

                if (start < current_node->interval_.start)
                {
                    InsertRecursive(current_node->left_, interval);
                }
                else
                {
                    InsertRecursive(current_node->right_, interval);
                }

                if (current_node->max_ < interval.end)
                {
                    current_node->max_ = interval.end;
                }

                current_node->height_ = 1 + std::max(GetNodeHeight(current_node->left_), GetNodeHeight(current_node->right_));

                int32_t height_diff = GetNodeHeightDifference(current_node);

                // Balance the tree with AVL balancing style.
                // Left-left case.
                if (height_diff > 1 && start < current_node->left_->interval_.start)
                {
                    RightRotate(current_node);
                }
                // Left-right case.
                else if (height_diff > 1 && start >= current_node->left_->interval_.start)
                {
                    LeftRotate(current_node->left_);
                    RightRotate(current_node);
                }
                // Right-right case.
                else if (height_diff < -1 && start >= current_node->right_->interval_.start)
                {
                    LeftRotate(current_node);
                }
                // Right-left case.
                else if (height_diff < -1 && start < current_node->right_->interval_.start)
                {
                    RightRotate(current_node->right_);
                    LeftRotate(current_node);
                }
            }
        }

        /// @brief Find all overlapping intervals for a given interval recursively.
        ///
        /// If culled is needed, the returned intervals will be culled based on
        /// their overlapping parts with the given interval.
        ///
        /// @param [in] current_node          The current tree node.
        /// @param [in] interval              Target interval.
        /// @param [in] overlapping_intervals Vector containing all overlapping intervals, the output intervals are culled if is_culled is set to true.
        /// @param [in] is_culled             Flag indicating if the output intervals need to be culled.
        inline void FindOverlappingIntervalsRecursive(const std::shared_ptr<IntervalTreeNode<T1, T2>>& current_node,
                                                      const Interval<T1, T2>&                          interval,
                                                      std::vector<Interval<T1, T2>>&                   overlapping_intervals,
                                                      bool                                             is_culled) const
        {
            if (current_node == nullptr)
            {
                return;
            }

            if (!(current_node->interval_.start > interval.end || current_node->interval_.end < interval.start))
            {
                if (is_culled)
                {
                    T1 culled_start = std::max(interval.start, current_node->interval_.start);
                    T1 culled_end   = std::min(interval.end, current_node->interval_.end);

                    overlapping_intervals.push_back({culled_start, culled_end, current_node->interval_.index});
                }
                else
                {
                    overlapping_intervals.push_back(current_node->interval_);
                }
            }

            if (current_node->left_ != nullptr && (current_node->left_->max_ >= interval.start))
            {
                FindOverlappingIntervalsRecursive(current_node->left_, interval, overlapping_intervals, is_culled);
            }

            FindOverlappingIntervalsRecursive(current_node->right_, interval, overlapping_intervals, is_culled);
        }

        /// @brief Find all overlapping intervals for a given interval recursively.
        ///
        /// If culled is needed, the returned intervals will be culled based on
        /// their overlapping parts with the given interval.
        ///
        /// @param [in] current_node          The current tree node.
        /// @param [in] interval              Target interval.
        /// @param [in] overlapping_intervals Queue containing all overlapping intervals, the output intervals are culled if is_culled is set to true.
        /// @param [in] is_culled             Flag indicating if the output intervals need to be culled.
        template <typename CompT>
        inline void FindOverlappingIntervalsRecursive(const std::shared_ptr<IntervalTreeNode<T1, T2>>&                             current_node,
                                                      const Interval<T1, T2>&                                                      interval,
                                                      std::priority_queue<Interval<T1, T2>, std::vector<Interval<T1, T2>>, CompT>& overlapping_intervals,
                                                      bool                                                                         is_culled) const
        {
            if (current_node == nullptr)
            {
                return;
            }

            if (!(current_node->interval_.start > interval.end || current_node->interval_.end < interval.start))
            {
                if (is_culled)
                {
                    T1 culled_start = std::max(interval.start, current_node->interval_.start);
                    T1 culled_end   = std::min(interval.end, current_node->interval_.end);

                    overlapping_intervals.push({culled_start, culled_end, current_node->interval_.index});
                }
                else
                {
                    overlapping_intervals.push(current_node->interval_);
                }
            }

            if (current_node->left_ != nullptr && (current_node->left_->max_ >= interval.start))
            {
                FindOverlappingIntervalsRecursive(current_node->left_, interval, overlapping_intervals, is_culled);
            }

            FindOverlappingIntervalsRecursive(current_node->right_, interval, overlapping_intervals, is_culled);
        }

        std::shared_ptr<IntervalTreeNode<T1, T2>> root_;  ///< Root node of tree.
    };
}  // namespace rmt_tree

#endif  // RMT_BACKEND_RMT_TREE_H_
