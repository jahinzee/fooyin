/*
 * Fooyin
 * Copyright 2022, Luke Taylor <LukeT1@proton.me>
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

namespace Core::Constants {

namespace Icons {
    constexpr auto Fooyin = "://icons/sc-fooyin.svg";
    constexpr auto Play = "://icons/play.svg";
    constexpr auto Pause = "://icons/pause.svg";
    constexpr auto Stop = "://icons/stop.svg";
    constexpr auto Prev = "://icons/prev.svg";
    constexpr auto Next = "://icons/next.svg";
    constexpr auto Repeat = "://icons/repeat-once.svg";
    constexpr auto RepeatAll = "://icons/repeat.svg";
    constexpr auto Shuffle = "://icons/shuffle.svg";
    constexpr auto VolumeMax = "://icons/volume-max.svg";
    constexpr auto VolumeMed = "://icons/volume-med.svg";
    constexpr auto VolumeLow = "://icons/volume-low.svg";
    constexpr auto VolumeMin = "://icons/volume-min.svg";
    constexpr auto VolumeMute = "://icons/volume-mute.svg";

    constexpr auto Quit = "://icons/quit.svg";
    constexpr auto Settings = "://icons/settings.svg";
    constexpr auto RescanLibrary = "://icons/reload.svg";
    constexpr auto LayoutEditing = "://icons/layout-editing.svg";
    constexpr auto QuickSetup = "://icons/quick-setup.svg";
}; // namespace Icons

constexpr auto MenuBar = "Fooyin.MenuBar";

namespace Menus {
    constexpr auto File = "Fooyin.Menu.File";
    constexpr auto Edit = "Fooyin.Menu.Edit";
    constexpr auto View = "Fooyin.Menu.View";
    constexpr auto Playback = "Fooyin.Menu.Playback";
    constexpr auto Library = "Fooyin.Menu.Library";
    constexpr auto Help = "Fooyin.Menu.Help";
}; // namespace Menus

namespace Groups {
    constexpr auto One = "Fooyin.Group.One";
    constexpr auto Two = "Fooyin.Group.Two";
    constexpr auto Three = "Fooyin.Group.Three";

    constexpr auto File = "Fooyin.Group.File";
    constexpr auto Edit = "Fooyin.Group.Edit";
    constexpr auto View = "Fooyin.Group.View";
    constexpr auto Playback = "Fooyin.Group.Playback";
    constexpr auto Library = "Fooyin.Group.Library";
    constexpr auto Help = "Fooyin.Group.Help";
}; // namespace Groups

namespace Actions {
    constexpr auto Exit = "Fooyin.Action.Exit";
    constexpr auto Settings = "Fooyin.Action.Settings";
    constexpr auto LayoutEditing = "Fooyin.Action.LayoutEditing";
    constexpr auto Rescan = "Fooyin.Action.Rescan";
    constexpr auto PlaylistCreate = "Fooyin.Action.Playlist.Create";
}; // namespace Actions

namespace ContextMenus {
    constexpr auto Playlist = "Fooyin.Menu.Playlist";
    constexpr auto PlaylistAddTo = "Fooyin.Menu.Playlist.AddTo";
}; // namespace ContextMenus

namespace ContextMenuGroups {
    constexpr auto Playlist = "Fooyin.Group.Playlist";
    constexpr auto PlaylistAddTo = "Fooyin.Group.Playlist.AddTo";
}; // namespace ContextMenuGroups

} // namespace Core::Constants