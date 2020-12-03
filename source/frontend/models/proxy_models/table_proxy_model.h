//=============================================================================
/// Copyright (c) 2018-2020 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Header for a proxy filter that processes multiple columns.
//=============================================================================

#ifndef RMV_MODELS_PROXY_MODELS_TABLE_PROXY_MODEL_H_
#define RMV_MODELS_PROXY_MODELS_TABLE_PROXY_MODEL_H_

#include <set>
#include <QSortFilterProxyModel>

namespace rmv
{
    /// Class to filter out and sort a table.
    class TableProxyModel : public QSortFilterProxyModel
    {
        Q_OBJECT

    public:
        /// Constructor.
        /// \param parent The parent widget.
        explicit TableProxyModel(QObject* parent = nullptr);

        /// Destructor.
        virtual ~TableProxyModel();

        /// Specify which columns should be sorted/filtered.
        /// \param columns list of columns.
        void SetFilterKeyColumns(const QList<qint32>& columns);

        /// Specify string to use as search filter.
        /// \param filter the search filter.
        void SetSearchFilter(const QString& filter);

        /// Specify range to use as size filter.
        /// \param min the min size.
        /// \param max the max size.
        void SetSizeFilter(uint64_t min, uint64_t max);

        /// Get content from proxy model.
        /// \param row The row where the data is located.
        /// \param column The column where the data is located.
        /// \return The contents at row, column.
        qulonglong GetData(int row, int column);

        /// Find a model index corresponding to the passed in data.
        /// \param lookup The value to find.
        /// \param column The column to search.
        /// \return the model index containing the data.
        QModelIndex FindModelIndex(qulonglong lookup, int column) const;

    protected:
        /// Methods that must be implemented by derived classes.
        virtual bool filterAcceptsRow(int row, const QModelIndex& source_parent) const = 0;
        virtual bool lessThan(const QModelIndex& left, const QModelIndex& right) const = 0;

        /// Filter the size slider.
        /// \param row The row to apply the size filter to.
        /// \param column The columnto apply the size filter to.
        /// \param source_parent The parent model index in the source model.
        /// \return true if the table item at row,column is to be shown, false if not.
        bool FilterSizeSlider(int row, int column, const QModelIndex& source_parent) const;

        /// Filter the search string.
        /// \param row The row to apply the size filter to.
        /// \param source_parent The parent model index in the source model.
        /// \return true if the table item at row,column is to be shown, false if not.
        bool FilterSearchString(int row, const QModelIndex& source_parent) const;

        /// Extract a uint64_t from a model index.
        /// \param index the index.
        /// \return the uint64 value stored at the model index.
        uint64_t GetIndexValue(const QModelIndex& index) const;

        std::set<qint32> column_filters_;  ///< Holds which columns are being filtered.
        QString          search_filter_;   ///< The current search string.
        uint64_t         min_size_;        ///< The minimum size of the size filter.
        uint64_t         max_size_;        ///< The maximum size of the size filter.
    };
}  // namespace rmv

#endif  // RMV_MODELS_PROXY_MODELS_TABLE_PROXY_MODEL_H_
