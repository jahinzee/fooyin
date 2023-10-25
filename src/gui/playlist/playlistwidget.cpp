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

#include "playlistwidget.h"

#include "gui/guisettings.h"
#include "gui/trackselectioncontroller.h"
#include "playlistcontroller.h"
#include "playlistdelegate.h"
#include "playlistmodel.h"
#include "playlistview.h"
#include "presetregistry.h"

#include <core/library/librarymanager.h>
#include <core/library/musiclibrary.h>
#include <core/library/sortingregistry.h>
#include <core/library/tracksort.h>
#include <core/player/playermanager.h>
#include <core/playlist/playlistmanager.h>
#include <utils/actions/actioncontainer.h>
#include <utils/async.h>
#include <utils/headerview.h>
#include <utils/settings/settingsdialogcontroller.h>

#include <QActionGroup>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QKeyEvent>
#include <QMenu>
#include <QScrollBar>

#include <QCoro/QCoroCore>

namespace {
void expandTree(QTreeView* view, QAbstractItemModel* model, const QModelIndex& parent, int first, int last)
{
    while(first <= last) {
        const QModelIndex child = model->index(first, 0, parent);
        view->expand(child);
        ++first;
    }
}
} // namespace

namespace Fy::Gui::Widgets::Playlist {
class PlaylistWidgetPrivate
{
public:
    PlaylistWidgetPrivate(PlaylistWidget* self, Core::Player::PlayerManager* playerManager,
                          PlaylistController* playlistController, TrackSelectionController* selectionController,
                          Utils::SettingsManager* settings);

    void onPresetChanged(const PlaylistPreset& preset);
    void changePreset(const PlaylistPreset& preset);

    void changePlaylist(const Core::Playlist::Playlist& playlist) const;

    void resetTree() const;

    [[nodiscard]] bool isHeaderHidden() const;
    [[nodiscard]] bool isScrollbarHidden() const;

    void setHeaderHidden(bool showHeader) const;
    void setScrollbarHidden(bool showScrollBar) const;

    void selectionChanged();

    void customHeaderMenuRequested(QPoint pos);

    void changeState(Core::Player::PlayState state) const;
    void playlistTracksChanged() const;

    void doubleClicked(const QModelIndex& index) const;

    void findCurrent() const;

    void switchContextMenu(QPoint pos);
    QCoro::Task<void> changeSort(QString script) const;
    void addSortMenu(QMenu* parent);

    PlaylistWidget* self;

    Core::Player::PlayerManager* playerManager;
    TrackSelectionController* selectionController;
    Utils::SettingsManager* settings;
    Utils::SettingsDialogController* settingsDialog;

    PlaylistController* controller;

    QHBoxLayout* layout;
    PlaylistModel* model;
    PlaylistView* playlistView;
    Utils::HeaderView* header;
    bool changingSelection{false};

    PlaylistPreset currentPreset;
};

PlaylistWidgetPrivate::PlaylistWidgetPrivate(PlaylistWidget* self, Core::Player::PlayerManager* playerManager,
                                             PlaylistController* playlistController,
                                             TrackSelectionController* selectionController,
                                             Utils::SettingsManager* settings)
    : self{self}
    , playerManager{playerManager}
    , selectionController{selectionController}
    , settings{settings}
    , settingsDialog{settings->settingsDialog()}
    , controller{playlistController}
    , layout{new QHBoxLayout(self)}
    , model{new PlaylistModel(playerManager, settings, self)}
    , playlistView{new PlaylistView(self)}
    , header{new Utils::HeaderView(Qt::Horizontal, self)}
{
    layout->setContentsMargins(0, 0, 0, 0);

    header->setStretchLastSection(true);
    playlistView->setHeader(header);
    header->setContextMenuPolicy(Qt::CustomContextMenu);

    playlistView->setModel(model);
    playlistView->setItemDelegate(new PlaylistDelegate(self));

    layout->addWidget(playlistView);

    setHeaderHidden(settings->value<Settings::PlaylistHeader>());
    setScrollbarHidden(settings->value<Settings::PlaylistScrollBar>());

    QObject::connect(playerManager, &Core::Player::PlayerManager::currentTrackChanged, model,
                     &PlaylistModel::changeTrackState);

    QObject::connect(model, &QAbstractItemModel::modelAboutToBeReset, playlistView, &QAbstractItemView::clearSelection);

    changePreset(controller->presetRegistry()->itemByName(settings->value<Settings::CurrentPreset>()));
}

void PlaylistWidgetPrivate::onPresetChanged(const PlaylistPreset& preset)
{
    if(currentPreset.id == preset.id) {
        changePreset(preset);
    }
}

void PlaylistWidgetPrivate::changePreset(const PlaylistPreset& preset)
{
    currentPreset = preset;
    model->changePreset(currentPreset);
    if(auto playlist = controller->currentPlaylist()) {
        model->reset(*playlist);
    }
}

void PlaylistWidgetPrivate::changePlaylist(const Core::Playlist::Playlist& playlist) const
{
    model->reset(playlist);
}

void PlaylistWidgetPrivate::resetTree() const
{
    playlistView->expandAll();
    playlistView->scrollToTop();
}

bool PlaylistWidgetPrivate::isHeaderHidden() const
{
    return playlistView->isHeaderHidden();
}

bool PlaylistWidgetPrivate::isScrollbarHidden() const
{
    return playlistView->verticalScrollBarPolicy() == Qt::ScrollBarAlwaysOff;
}

void PlaylistWidgetPrivate::setHeaderHidden(bool showHeader) const
{
    playlistView->setHeaderHidden(!showHeader);
}

void PlaylistWidgetPrivate::setScrollbarHidden(bool showScrollBar) const
{
    playlistView->setVerticalScrollBarPolicy(!showScrollBar ? Qt::ScrollBarAlwaysOff : Qt::ScrollBarAsNeeded);
}

void PlaylistWidgetPrivate::selectionChanged()
{
    if(changingSelection) {
        return;
    }

    changingSelection = true;

    QItemSelectionModel* selectionModel = playlistView->selectionModel();

    QModelIndexList indexes = selectionModel->selectedIndexes();
    QItemSelection itemsToSelect;
    QItemSelection itemsToDeselect;

    Core::TrackList tracks;

    while(!indexes.empty()) {
        const QModelIndex& index = indexes.front();
        indexes.pop_front();

        if(!index.isValid()) {
            continue;
        }

        const auto type = index.data(PlaylistItem::Type).toInt();

        if(type != PlaylistItem::Track) {
            itemsToDeselect.emplace_back(index);

            const QItemSelection children{model->index(0, 0, index),
                                          model->index(model->rowCount(index) - 1, 0, index)};
            const auto childIndexes = children.indexes();

            auto childrenToSelect = std::views::filter(
                childIndexes, [&](const QModelIndex& childIndex) { return !selectionModel->isSelected(childIndex); });

            std::ranges::copy(childrenToSelect, std::back_inserter(indexes));
        }
        else {
            itemsToSelect.emplace_back(index);
            tracks.push_back(index.data(PlaylistItem::Role::ItemData).value<Core::Track>());
        }
    }
    selectionModel->select(itemsToDeselect, QItemSelectionModel::Deselect);
    selectionModel->select(itemsToSelect, QItemSelectionModel::Select);

    if(!tracks.empty()) {
        selectionController->changeSelectedTracks(tracks);
    }
    changingSelection = false;
}

void PlaylistWidgetPrivate::customHeaderMenuRequested(QPoint pos)
{
    auto* menu = new QMenu(self);
    menu->setAttribute(Qt::WA_DeleteOnClose);

    auto* presetsMenu = new QMenu(PlaylistWidget::tr("Presets"), menu);

    const auto& presets = controller->presetRegistry()->items();

    for(const auto& [index, preset] : presets) {
        const QString name = preset.name;
        auto* switchPreset = new QAction(name, presetsMenu);
        if(preset == currentPreset) {
            presetsMenu->setDefaultAction(switchPreset);
        }
        QObject::connect(switchPreset, &QAction::triggered, self,
                         [this, name]() { settings->set<Settings::CurrentPreset>(name); });
        presetsMenu->addAction(switchPreset);
        //            }
    }
    menu->addMenu(presetsMenu);

    menu->popup(self->mapToGlobal(pos));
}

void PlaylistWidgetPrivate::changeState(Core::Player::PlayState state) const
{
    switch(state) {
        case(Core::Player::PlayState::Playing):
            model->changeTrackState();
            findCurrent();
            break;
        case(Core::Player::PlayState::Stopped):
        case(Core::Player::PlayState::Paused):
            model->changeTrackState();
            break;
    }
}

void PlaylistWidgetPrivate::playlistTracksChanged() const
{
    QModelIndexList indexes{{}};

    Core::TrackList tracks;

    while(!indexes.empty()) {
        const QModelIndex& index = indexes.front();
        indexes.pop_front();

        const auto type = index.data(PlaylistItem::Type).toInt();

        if(type != PlaylistItem::Track) {
            const QItemSelection children{model->index(0, 0, index),
                                          model->index(model->rowCount(index) - 1, 0, index)};
            const auto childIndexes = children.indexes();
            std::ranges::copy(childIndexes, std::back_inserter(indexes));
        }
        else {
            tracks.push_back(index.data(PlaylistItem::Role::ItemData).value<Core::Track>());
        }
    }
    if(auto playlist = controller->currentPlaylist()) {
        controller->playlistHandler()->replacePlaylistTracks(playlist->id(), tracks);
        if(auto updatedPlaylist = controller->currentPlaylist()) {
            model->updateHeader(*updatedPlaylist);
        }
    }
}

void PlaylistWidgetPrivate::doubleClicked(const QModelIndex& index) const
{
    const auto type = index.data(PlaylistItem::Type);

    if(type != PlaylistItem::Track) {
        selectionController->executeAction(TrackAction::Play);
    }
    else {
        const auto track = index.data(PlaylistItem::Role::ItemData).value<Core::Track>();
        controller->startPlayback(track);
    }
    model->changeTrackState();
    playlistView->clearSelection();
}

void PlaylistWidgetPrivate::findCurrent() const
{
    const Core::Track track = playerManager->currentTrack();
    //        const QModelIndex index = model->indexForTrack(track);
    //        if(index.isValid()) {
    //            playlistView->scrollTo(index);
    //            playlistView->setCurrentIndex(index);
    //        }
}

void PlaylistWidgetPrivate::switchContextMenu(QPoint pos)
{
    auto* menu = new QMenu(self);
    menu->setAttribute(Qt::WA_DeleteOnClose);

    //        const auto currentPlaylist = playlistController->currentPlaylist();
    const auto& playlists = controller->playlists();

    for(const auto& playlist : playlists) {
        auto* switchPl = new QAction(playlist.name(), menu);
        const int id   = playlist.id();
        QObject::connect(switchPl, &QAction::triggered, self, [this, id]() { controller->changeCurrentPlaylist(id); });
        menu->addAction(switchPl);
    }
    menu->popup(self->mapToGlobal(pos));
}

QCoro::Task<void> PlaylistWidgetPrivate::changeSort(QString script) const
{
    /* if(playlistView->selectionModel()->hasSelection()) { }
     else*/
    if(auto playlist = controller->currentPlaylist()) {
        const auto sortedTracks = co_await Utils::asyncExec(
            [&script, &playlist]() { return Core::Library::Sorting::calcSortTracks(script, playlist->tracks()); });
        playlist->replaceTracks(sortedTracks);
        controller->playlistHandler()->replacePlaylistTracks(playlist->id(), sortedTracks);
        changePlaylist(*playlist);
    }
    co_return;
}

void PlaylistWidgetPrivate::addSortMenu(QMenu* parent)
{
    auto* sortMenu = new QMenu(PlaylistWidget::tr("Sort"), parent);

    const auto& groups = controller->sortRegistry()->items();
    for(const auto& [index, script] : groups) {
        auto* switchSort = new QAction(script.name, sortMenu);
        QObject::connect(switchSort, &QAction::triggered, self, [this, script]() { changeSort(script.script); });
        sortMenu->addAction(switchSort);
    }
    parent->addMenu(sortMenu);
}

PlaylistWidget::PlaylistWidget(Core::Player::PlayerManager* playerManager, PlaylistController* playlistController,
                               TrackSelectionController* selectionController, Utils::SettingsManager* settings,
                               QWidget* parent)
    : FyWidget{parent}
    , p{std::make_unique<PlaylistWidgetPrivate>(this, playerManager, playlistController, selectionController, settings)}
{
    setObjectName("Playlist");

    QObject::connect(p->playlistView->header(), &QHeaderView::customContextMenuRequested, this,
                     [this](QPoint pos) { p->customHeaderMenuRequested(pos); });
    QObject::connect(p->playlistView->selectionModel(), &QItemSelectionModel::selectionChanged, this,
                     [this]() { p->selectionChanged(); });

    QObject::connect(p->playlistView, &PlaylistView::playlistChanged, this, [this]() { p->playlistTracksChanged(); });
    QObject::connect(p->playlistView, &QAbstractItemView::doubleClicked, this,
                     [this](const QModelIndex& index) { p->doubleClicked(index); });
    QObject::connect(playerManager, &Core::Player::PlayerManager::playStateChanged, this,
                     [this](Core::Player::PlayState state) { p->changeState(state); });

    QObject::connect(p->model, &QAbstractItemModel::rowsInserted, this,
                     [this](const QModelIndex& parent, int first, int last) {
                         expandTree(p->playlistView, p->model, parent, first, last);
                     });
    QObject::connect(p->header, &Utils::HeaderView::leftClicked, this,
                     [this](int /*section*/, QPoint pos) { p->switchContextMenu(pos); });

    QObject::connect(p->model, &QAbstractItemModel::modelReset, this, [this]() { p->resetTree(); });

    QObject::connect(playlistController, &PlaylistController::currentPlaylistChanged, this,
                     [this](const Core::Playlist::Playlist& playlist) { p->changePlaylist(playlist); });
    QObject::connect(playlistController, &PlaylistController::refreshPlaylist, this,
                     [this](const Core::Playlist::Playlist& playlist) { p->changePlaylist(playlist); });

    QObject::connect(p->controller->presetRegistry(), &PresetRegistry::presetChanged, this,
                     [this](const PlaylistPreset& preset) { p->onPresetChanged(preset); });

    settings->subscribe<Settings::PlaylistHeader>(this, [this](bool showHeader) { p->setHeaderHidden(showHeader); });
    settings->subscribe<Settings::PlaylistScrollBar>(
        this, [this](bool showScrollbar) { p->setScrollbarHidden(showScrollbar); });
    settings->subscribe<Settings::CurrentPreset>(this, [this](const QString& presetName) {
        const auto preset = p->controller->presetRegistry()->itemByName(presetName);
        p->changePreset(preset);
    });
}

PlaylistWidget::~PlaylistWidget() = default;

QString PlaylistWidget::name() const
{
    return QStringLiteral("Playlist");
}

void PlaylistWidget::contextMenuEvent(QContextMenuEvent* event)
{
    auto* menu = new QMenu(this);
    menu->setAttribute(Qt::WA_DeleteOnClose);

    auto* removeRows = new QAction(tr("Remove"), menu);
    QObject::connect(removeRows, &QAction::triggered, this, [this]() {
        QModelIndexList trackSelection;
        const auto selected = p->playlistView->selectionModel()->selectedIndexes();
        std::ranges::copy_if(std::as_const(selected), std::back_inserter(trackSelection), [](const QModelIndex& index) {
            return index.isValid() && index.data(PlaylistItem::Type).toInt() == PlaylistItem::Track;
        });

        p->model->removeTracks(trackSelection);
        p->playlistTracksChanged();
    });
    menu->addAction(removeRows);

    menu->addSeparator();

    p->addSortMenu(menu);
    p->selectionController->addTrackContextMenu(menu);

    menu->popup(mapToGlobal(event->pos()));
}

void PlaylistWidget::keyPressEvent(QKeyEvent* e)
{
    const auto key = e->key();

    if(key == Qt::Key_Enter || key == Qt::Key_Return) {
        if(p->selectionController->hasTracks()) {
            p->selectionController->executeAction(TrackAction::Play);

            p->model->changeTrackState();
            p->playlistView->clearSelection();
        }
    }
    QWidget::keyPressEvent(e);
}

// void PlaylistWidget::spanHeaders(QModelIndex parent)
//{
//     for (int i = 0; i < p->model->rowCount(parent); i++)
//     {
//         auto idx = p->model->index(i, 0, parent);
//         if (p->model->hasChildren(idx))
//         {
//             p->playlist->setFirstColumnSpanned(i, parent, true);
//             spanHeaders(idx);
//         }
//     }
// }
} // namespace Fy::Gui::Widgets::Playlist
