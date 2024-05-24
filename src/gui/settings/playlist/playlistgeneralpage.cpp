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

#include "playlistgeneralpage.h"

#include "internalguisettings.h"

#include <core/coresettings.h>
#include <gui/guiconstants.h>
#include <gui/guisettings.h>
#include <utils/settings/settingsmanager.h>

#include <QCheckBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QSpinBox>

namespace Fooyin {
class PlaylistGeneralPageWidget : public SettingsPageWidget
{
    Q_OBJECT

public:
    explicit PlaylistGeneralPageWidget(SettingsManager* settings);

    void load() override;
    void apply() override;
    void reset() override;

private:
    SettingsManager* m_settings;

    QCheckBox* m_cursorFollowsPlayback;
    QCheckBox* m_playbackFollowsCursor;
    QCheckBox* m_rewindPrevious;

    QCheckBox* m_scrollBars;
    QCheckBox* m_header;
    QCheckBox* m_altColours;
    QCheckBox* m_tabsAddButton;

    QSpinBox* m_imagePadding;
    QSpinBox* m_imagePaddingTop;
};

PlaylistGeneralPageWidget::PlaylistGeneralPageWidget(SettingsManager* settings)
    : m_settings{settings}
    , m_cursorFollowsPlayback{new QCheckBox(tr("Cursor follows playback"), this)}
    , m_playbackFollowsCursor{new QCheckBox(tr("Playback follows cursor"), this)}
    , m_rewindPrevious{new QCheckBox(tr("Rewind track on previous"), this)}
    , m_scrollBars{new QCheckBox(tr("Show scrollbar"), this)}
    , m_header{new QCheckBox(tr("Show header"), this)}
    , m_altColours{new QCheckBox(tr("Alternate row colours"), this)}
    , m_tabsAddButton{new QCheckBox(tr("Show add button"), this)}
    , m_imagePadding{new QSpinBox(this)}
    , m_imagePaddingTop{new QSpinBox(this)}
{
    auto* layout = new QGridLayout(this);

    m_rewindPrevious->setToolTip(tr(
        "If the current track has been playing for more than 5s, restart it instead of moving to the previous track"));

    m_imagePadding->setMinimum(0);
    m_imagePadding->setMaximum(100);
    m_imagePadding->setSuffix(QStringLiteral("px"));

    m_imagePaddingTop->setMinimum(0);
    m_imagePaddingTop->setMaximum(100);
    m_imagePaddingTop->setSuffix(QStringLiteral("px"));

    auto* behaviour       = new QGroupBox(tr("Behaviour"), this);
    auto* behaviourLayout = new QGridLayout(behaviour);

    behaviourLayout->addWidget(m_cursorFollowsPlayback, 0, 0, 1, 2);
    behaviourLayout->addWidget(m_playbackFollowsCursor, 1, 0, 1, 2);
    behaviourLayout->addWidget(m_rewindPrevious, 2, 0, 1, 2);

    auto* appearance       = new QGroupBox(tr("Appearance"), this);
    auto* appearanceLayout = new QGridLayout(appearance);

    auto* tabsGroup       = new QGroupBox(tr("Playlist Tabs"), this);
    auto* tabsGroupLayout = new QGridLayout(tabsGroup);

    auto* padding       = new QGroupBox(tr("Image Padding"), this);
    auto* paddingLayout = new QGridLayout(padding);

    auto* paddingLabel    = new QLabel(tr("Left/Right") + QStringLiteral(":"), this);
    auto* paddingTopLabel = new QLabel(tr("Top") + QStringLiteral(":"), this);

    paddingLayout->addWidget(paddingLabel, 0, 0);
    paddingLayout->addWidget(m_imagePadding, 0, 1);
    paddingLayout->addWidget(paddingTopLabel, 1, 0);
    paddingLayout->addWidget(m_imagePaddingTop, 1, 1);
    paddingLayout->setColumnStretch(2, 1);

    int row{0};
    appearanceLayout->addWidget(m_scrollBars, row++, 0, 1, 2);
    appearanceLayout->addWidget(m_header, row++, 0, 1, 2);
    appearanceLayout->addWidget(m_altColours, row++, 0, 1, 2);
    appearanceLayout->addWidget(padding, row, 0, 1, 3);
    appearanceLayout->setColumnStretch(2, 1);
    appearanceLayout->setRowStretch(appearanceLayout->rowCount(), 1);

    auto* addButtonLabel = new QLabel(tr("⚠ This will disable moving tabs by dragging"), this);

    tabsGroupLayout->addWidget(m_tabsAddButton, 0, 0);
    tabsGroupLayout->addWidget(addButtonLabel, 1, 0);

    layout->addWidget(behaviour, 0, 0);
    layout->addWidget(appearance, 1, 0);
    layout->addWidget(tabsGroup, 2, 0);

    layout->setRowStretch(layout->rowCount(), 1);
}

void PlaylistGeneralPageWidget::load()
{
    m_cursorFollowsPlayback->setChecked(m_settings->value<Settings::Gui::CursorFollowsPlayback>());
    m_playbackFollowsCursor->setChecked(m_settings->value<Settings::Gui::PlaybackFollowsCursor>());
    m_rewindPrevious->setChecked(m_settings->value<Settings::Core::RewindPreviousTrack>());

    m_scrollBars->setChecked(m_settings->value<Settings::Gui::Internal::PlaylistScrollBar>());
    m_header->setChecked(m_settings->value<Settings::Gui::Internal::PlaylistHeader>());
    m_altColours->setChecked(m_settings->value<Settings::Gui::Internal::PlaylistAltColours>());

    m_tabsAddButton->setChecked(m_settings->value<Settings::Gui::Internal::PlaylistTabsAddButton>());

    m_imagePadding->setValue(m_settings->value<Settings::Gui::Internal::PlaylistImagePadding>());
    m_imagePaddingTop->setValue(m_settings->value<Settings::Gui::Internal::PlaylistImagePaddingTop>());
}

void PlaylistGeneralPageWidget::apply()
{
    m_settings->set<Settings::Gui::CursorFollowsPlayback>(m_cursorFollowsPlayback->isChecked());
    m_settings->set<Settings::Gui::PlaybackFollowsCursor>(m_playbackFollowsCursor->isChecked());
    m_settings->set<Settings::Core::RewindPreviousTrack>(m_rewindPrevious->isChecked());

    m_settings->set<Settings::Gui::Internal::PlaylistScrollBar>(m_scrollBars->isChecked());
    m_settings->set<Settings::Gui::Internal::PlaylistHeader>(m_header->isChecked());
    m_settings->set<Settings::Gui::Internal::PlaylistAltColours>(m_altColours->isChecked());

    m_settings->set<Settings::Gui::Internal::PlaylistTabsAddButton>(m_tabsAddButton->isChecked());

    m_settings->set<Settings::Gui::Internal::PlaylistImagePadding>(m_imagePadding->value());
    m_settings->set<Settings::Gui::Internal::PlaylistImagePaddingTop>(m_imagePaddingTop->value());
}

void PlaylistGeneralPageWidget::reset()
{
    m_settings->reset<Settings::Gui::CursorFollowsPlayback>();
    m_settings->reset<Settings::Gui::PlaybackFollowsCursor>();
    m_settings->reset<Settings::Core::RewindPreviousTrack>();

    m_settings->reset<Settings::Gui::Internal::PlaylistScrollBar>();
    m_settings->reset<Settings::Gui::Internal::PlaylistHeader>();
    m_settings->reset<Settings::Gui::Internal::PlaylistAltColours>();

    m_settings->reset<Settings::Gui::Internal::PlaylistTabsAddButton>();

    m_settings->reset<Settings::Gui::Internal::PlaylistImagePadding>();
    m_settings->reset<Settings::Gui::Internal::PlaylistImagePaddingTop>();
}

PlaylistGeneralPage::PlaylistGeneralPage(SettingsManager* settings)
    : SettingsPage{settings->settingsDialog()}
{
    setId(Constants::Page::PlaylistGeneral);
    setName(tr("General"));
    setCategory({tr("Playlist")});
    setWidgetCreator([settings] { return new PlaylistGeneralPageWidget(settings); });
}
} // namespace Fooyin

#include "moc_playlistgeneralpage.cpp"
#include "playlistgeneralpage.moc"
