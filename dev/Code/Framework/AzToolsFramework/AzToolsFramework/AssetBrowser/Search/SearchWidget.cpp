/*
* All or portions of this file Copyright (c) Amazon.com, Inc. or its affiliates or
* its licensors.
*
* For complete copyright and license terms please see the LICENSE at the root of this
* distribution (the "License"). All use of this software is governed by the License,
* or, if provided, by the license below or the license accompanying this file. Do not
* remove or modify any license notices. This file is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*
*/

#include <AzToolsFramework/AssetBrowser/Search/SearchWidget.h>

#include <AzCore/Asset/AssetTypeInfoBus.h>
#include <AzCore/std/containers/vector.h>

#include <QLineEdit>
#include <QPushButton>

namespace AzToolsFramework
{
    namespace AssetBrowser
    {
        namespace
        {
            AzQtComponents::SearchTypeFilterList buildTypesFilterList()
            {
                AzQtComponents::SearchTypeFilterList filters;

                EBusAggregateUniqueResults<QString> groups;
                AZ::AssetTypeInfoBus::BroadcastResult(groups, &AZ::AssetTypeInfo::GetGroup);

                // Group "Other" should be in the end of the list, and "Hidden" should not be on the list at all
                for (const QString& group : groups.values)
                {
                    if (group != "Hidden")
                    {
                        EBusAggregateAssetTypesIfBelongsToGroup types(group);
                        AZ::AssetTypeInfoBus::BroadcastResult(types, &AZ::AssetTypeInfo::GetAssetType);

                        if (!types.values.empty())
                        {
                            AssetGroupFilter* groupFilter = new AssetGroupFilter();
                            groupFilter->SetAssetGroup(group);

                            AzQtComponents::SearchTypeFilter stFilter;
                            stFilter.displayName = group;
                            stFilter.metadata = QVariant::fromValue(FilterConstType(groupFilter));

                            filters.push_back(stFilter);
                        }
                    }
                }

                std::sort(filters.begin(), filters.end(),
                          [](const AzQtComponents::SearchTypeFilter& a, const AzQtComponents::SearchTypeFilter& b)
                {
                    const int categoryResult = QString::compare(a.category, b.category, Qt::CaseInsensitive);

                    if (categoryResult != 0)
                    {
                        return categoryResult < 0;
                    }
                    else if (a.displayName == QStringLiteral("Other"))
                    {
                        return false;
                    }
                    else if (b.displayName == QStringLiteral("Other"))
                    {
                        return true;
                    }

                    return QString::compare(a.displayName, b.displayName, Qt::CaseInsensitive) < 0;
                });

                return filters;
            }
        }

        SearchWidget::SearchWidget(QWidget* parent)
            : AzQtComponents::FilteredSearchWidget(parent)
            , m_filter(new CompositeFilter(CompositeFilter::LogicOperatorType::AND))
            , m_stringFilter(new CompositeFilter(CompositeFilter::LogicOperatorType::AND))
            , m_typesFilter(new CompositeFilter(CompositeFilter::LogicOperatorType::OR))
        {
            m_filter->SetFilterPropagation(AssetBrowserEntryFilter::PropagateDirection::Down);

            m_stringFilter->SetFilterPropagation(AssetBrowserEntryFilter::PropagateDirection::Up);
            m_stringFilter->SetTag("String");

            m_typesFilter->SetFilterPropagation(AssetBrowserEntryFilter::PropagateDirection::Down);
            m_typesFilter->SetTag("AssetTypes");

            connect(this, &AzQtComponents::FilteredSearchWidget::TextFilterChanged, this,
                    [this](const QString& text)
            {
                if (!filterLineEdit()->isHidden())
                {
                    m_stringFilter->RemoveAllFilters();

                    auto stringList = text.split(' ', QString::SkipEmptyParts);
                    for (auto& str : stringList)
                    {
                        auto stringFilter = new StringFilter();
                        stringFilter->SetFilterString(str);
                        m_stringFilter->AddFilter(FilterConstType(stringFilter));
                    }
                }
            });
            connect(this, &AzQtComponents::FilteredSearchWidget::TypeFilterChanged, this,
                    [this](const AzQtComponents::SearchTypeFilterList& filters)
            {
                if (!filterTypePushButton()->isHidden())
                {
                    m_typesFilter->RemoveAllFilters();

                    if (filters.isEmpty())
                    {
                        m_typesFilter->SetEmptyResult(true);
                    }
                    else
                    {
                        for (auto it = filters.constBegin(), end = filters.constEnd(); it != end; ++it)
                        {
                            m_typesFilter->AddFilter((*it).metadata.value<FilterConstType>());
                        }
                    }
                }
            });
        }

        void SearchWidget::Setup(bool stringFilter, bool assetTypeFilter)
        {
            ClearTextFilter();
            ClearTypeFilter();

            m_filter->RemoveAllFilters();

            SetTextFilterVisible(stringFilter);
            SetTypeFilterVisible(assetTypeFilter);

            if (stringFilter)
            {
                m_filter->AddFilter(m_stringFilter);
            }

            // do not show assets in Hidden group
            auto hiddenGroupFilter = new AssetGroupFilter();
            hiddenGroupFilter->SetAssetGroup("Hidden");
            auto inverseFilter = new InverseFilter();
            inverseFilter->SetFilter(FilterConstType(hiddenGroupFilter));
            m_filter->AddFilter(FilterConstType(inverseFilter));

            // hide irrelevant
            auto cleanerProductsFilter = new CleanerProductsFilter();
            m_filter->AddFilter(FilterConstType(cleanerProductsFilter));

            if (assetTypeFilter)
            {
                m_filter->AddFilter(FilterConstType(m_typesFilter));
                SetTypeFilters(buildTypesFilterList());
            }
        }

        QSharedPointer<CompositeFilter> SearchWidget::GetFilter() const
        {
            return m_filter;
        }

    } // namespace AssetBrowser
} // namespace AzToolsFramework
#include <AssetBrowser/Search/SearchWidget.moc>
#include <AssetBrowser/Search/rcc_search.h>
