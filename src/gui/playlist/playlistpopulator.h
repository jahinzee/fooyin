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

#pragma once

#include "playlistcolumn.h"
#include "playlistitem.h"
#include "playlistitemmodels.h"

#include <core/trackfwd.h>
#include <utils/id.h>
#include <utils/worker.h>

namespace Fooyin {
class PlayerController;
struct PlaylistPreset;

using ItemList        = std::vector<PlaylistItem>;
using TrackItemMap    = std::unordered_map<Track, PlaylistItem, Track::TrackHash>;
using ItemKeyMap      = std::unordered_map<QString, PlaylistItem>;
using ContainerKeyMap = std::unordered_map<QString, PlaylistContainerItem*>;
using NodeKeyMap      = std::unordered_map<QString, std::vector<QString>>;
using TrackIdNodeMap  = std::unordered_map<int, std::vector<QString>>;
using IndexGroupMap   = std::map<int, std::vector<QString>>;

struct PendingData
{
    Id playlistId;
    ItemKeyMap items;
    NodeKeyMap nodes;
    std::vector<QString> containerOrder;
    TrackIdNodeMap trackParents;

    QString parent;
    int row{-1};

    IndexGroupMap indexNodes;

    void clear()
    {
        playlistId = {};
        items.clear();
        nodes.clear();
        containerOrder.clear();
        trackParents.clear();
        row = -1;
        indexNodes.clear();
    }
};

class PlaylistPopulator : public Worker
{
    Q_OBJECT

public:
    explicit PlaylistPopulator(PlayerController* playerController, QObject* parent = nullptr);
    ~PlaylistPopulator() override;

    void run(const Id& playlistId, const PlaylistPreset& preset, const PlaylistColumnList& columns,
             const TrackList& tracks);
    void runTracks(const Id& playlistId, const PlaylistPreset& preset, const PlaylistColumnList& columns,
                   const std::map<int, TrackList>& tracks);
    void updateTracks(const Id& playlistId, const PlaylistPreset& preset, const PlaylistColumnList& columns,
                      const TrackItemMap& tracks);
    void updateHeaders(const ItemList& headers);

signals:
    void populated(PendingData data);
    void populatedTrackGroup(PendingData data);
    void tracksUpdated(ItemList tracks);
    void headersUpdated(ItemKeyMap headers);

private:
    struct Private;
    std::unique_ptr<Private> p;
};
} // namespace Fooyin
