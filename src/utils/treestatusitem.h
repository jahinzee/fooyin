/*
 * Fooyin
 * Copyright 2022-2023, Luke Taylor <LukeT1@proton.me>
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

#pragma once

#include <utils/treeitem.h>

namespace Fy::Utils {
template <class Item>
class TreeStatusItem : public TreeItem<Item>
{
public:
    enum ItemStatus
    {
        None    = 0,
        Added   = 1,
        Removed = 2,
        Changed = 3
    };

    explicit TreeStatusItem(Item* parent)
        : TreeItem<Item>{parent}
    { }

    [[nodiscard]] ItemStatus status() const
    {
        return m_status;
    };

    void setStatus(ItemStatus status)
    {
        m_status = status;
    };

private:
    ItemStatus m_status{None};
};
} // namespace Fy::Utils