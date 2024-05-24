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

#include "librarytreeitem.h"

#include <core/library/tracksort.h>

#include <QCollator>

namespace Fooyin {
LibraryTreeItem::LibraryTreeItem()
    : LibraryTreeItem{QStringLiteral(""), nullptr, -1}
{ }

LibraryTreeItem::LibraryTreeItem(QString title, LibraryTreeItem* parent, int level)
    : TreeItem{parent}
    , m_pending{false}
    , m_level{level}
    , m_key{QStringLiteral("0")}
    , m_title{std::move(title)}
{ }

bool LibraryTreeItem::pending() const
{
    return m_pending;
}

int LibraryTreeItem::level() const
{
    return m_level;
}

QString LibraryTreeItem::title() const
{
    return m_title;
}

TrackList LibraryTreeItem::tracks() const
{
    return m_tracks;
}

int LibraryTreeItem::trackCount() const
{
    return static_cast<int>(m_tracks.size());
}

QString LibraryTreeItem::key() const
{
    return m_key;
}

void LibraryTreeItem::setPending(bool pending)
{
    m_pending = pending;
}

void LibraryTreeItem::setTitle(const QString& title)
{
    m_title = title;
}

void LibraryTreeItem::setKey(const QString& key)
{
    m_key = key;
}

void LibraryTreeItem::addTrack(const Track& track)
{
    m_tracks.emplace_back(track);
}

void LibraryTreeItem::addTracks(const TrackList& tracks)
{
    std::ranges::copy(tracks, std::back_inserter(m_tracks));
}

void LibraryTreeItem::removeTrack(const Track& track)
{
    if(m_tracks.empty()) {
        return;
    }
    std::erase_if(m_tracks, [track](const Track& child) { return child.id() == track.id(); });
}

void LibraryTreeItem::replaceTrack(const Track& track)
{
    if(m_tracks.empty()) {
        return;
    }
    std::ranges::replace_if(
        m_tracks, [track](const Track& child) { return child.id() == track.id(); }, track);
}

void LibraryTreeItem::sortTracks()
{
    m_tracks = Sorting::sortTracks(m_tracks);
}

void LibraryTreeItem::sortChildren()
{
    std::vector<LibraryTreeItem*> sortedChildren{m_children};

    QCollator collator;
    collator.setNumericMode(true);

    std::sort(sortedChildren.begin(), sortedChildren.end(),
              [collator](const LibraryTreeItem* lhs, const LibraryTreeItem* rhs) {
                  if(lhs->m_level == -1) {
                      return true;
                  }
                  if(rhs->m_level == -1) {
                      return false;
                  }

                  const auto cmp = collator.compare(lhs->m_title, rhs->m_title);
                  if(cmp == 0) {
                      return false;
                  }
                  return cmp < 0;
              });
    m_children = sortedChildren;

    for(auto& child : m_children) {
        child->sortChildren();
        child->sortTracks();
    }
}
} // namespace Fooyin
