/*
 * Fooyin
 * Copyright © 2023, Luke Taylor <LukeT1@proton.me>
 *
 * Fooyin is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Fooyin is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Fooyin.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "tageditoritem.h"

#include <utils/helpers.h>

constexpr auto CharLimit = 2000;

namespace {
bool withinCharLimit(const QStringList& strings)
{
    const int currentLength = std::accumulate(strings.cbegin(), strings.cend(), 0,
                                              [](int sum, const QString& str) { return sum + str.length(); });
    return currentLength <= CharLimit;
}
} // namespace

namespace Fooyin::TagEditor {
TagEditorItem::TagEditorItem()
    : TagEditorItem{QStringLiteral(""), nullptr, true}
{ }

TagEditorItem::TagEditorItem(QString title, TagEditorItem* parent, bool isDefault)
    : TreeStatusItem{parent}
    , m_isDefault{isDefault}
    , m_name{std::move(title)}
    , m_trackCount{0}
{ }

QString TagEditorItem::name() const
{
    return m_name;
}

QString TagEditorItem::value() const
{
    if(m_value.isEmpty()) {
        QStringList nonEmptyValues{m_values};
        nonEmptyValues.removeAll(QStringLiteral(""));
        m_value = nonEmptyValues.join(QStringLiteral("; "));
    }

    return m_value;
}

bool TagEditorItem::isDefault() const
{
    return m_isDefault;
}

int TagEditorItem::trackCount() const
{
    return m_trackCount;
}

void TagEditorItem::addTrackValue(const QString& value)
{
    if(m_values.contains(value)) {
        return;
    }

    if(m_trackCount == 0 || withinCharLimit(m_values)) {
        m_values.append(value);
        m_values.sort();
    }

    m_trackCount++;
}

void TagEditorItem::addTrackValue(const QStringList& values)
{
    for(const auto& trackValue : values) {
        if(m_values.contains(trackValue)) {
            continue;
        }

        if(m_trackCount == 0 || withinCharLimit(m_values)) {
            m_values.append(trackValue);
            m_values.sort();
        }
    }

    m_trackCount++;
}

void TagEditorItem::setValue(const QStringList& values)
{
    m_values = values;
    m_value.clear();
}

void TagEditorItem::setTitle(const QString& title)
{
    m_name = title;
}
} // namespace Fooyin::TagEditor
