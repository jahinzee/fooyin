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

#include <utils/actions/widgetcontext.h>

namespace Fooyin {
Context::Context(const Id& id)
{
    append(id);
}

Context::Context(const IdList& ids)
{
    std::ranges::for_each(ids, [this](const Id& id) { append(id); });
}

bool Context::operator==(const Context& context) const
{
    return m_ids == context.m_ids;
}

int Context::size() const
{
    return static_cast<int>(m_ids.size());
}

bool Context::empty() const
{
    return m_ids.empty();
}

bool Context::contains(const Id& id) const
{
    return std::ranges::find(m_ids, id) != m_ids.cend();
}

IdList::const_iterator Context::begin() const
{
    return m_ids.cbegin();
}

IdList::const_iterator Context::end() const
{
    return m_ids.cend();
}

void Context::append(const Id& id)
{
    m_ids.push_back(id);
}

void Context::append(const Context& context)
{
    std::ranges::copy(context.m_ids, std::back_inserter(m_ids));
}

void Context::prepend(const Id& id)
{
    m_ids.insert(m_ids.begin(), id);
}

void Context::erase(const Id& id)
{
    std::erase(m_ids, id);
}

WidgetContext::WidgetContext(QWidget* widget, QObject* parent)
    : WidgetContext{widget, {}, parent}
{ }

WidgetContext::WidgetContext(QWidget* widget, Context context, QObject* parent)
    : QObject{parent}
    , m_widget{widget}
    , m_context{std::move(context)}
    , m_isEnabled{true}
{ }

Context WidgetContext::context() const
{
    return m_context;
}

QWidget* WidgetContext::widget() const
{
    return m_widget;
}

bool WidgetContext::isEnabled() const
{
    return m_isEnabled;
}

void WidgetContext::setEnabled(bool enabled)
{
    if(std::exchange(m_isEnabled, enabled) != enabled) {
        emit isEnabledChanged();
    }
}
} // namespace Fooyin
