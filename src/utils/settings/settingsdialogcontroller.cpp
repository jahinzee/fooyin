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

#include <utils/settings/settingsdialogcontroller.h>

#include <utils/id.h>

#include "settingsdialog.h"

namespace Fy::Utils {
struct SettingsDialogController::Private
{
    QByteArray geometry;
    PageList pages;
};

SettingsDialogController::SettingsDialogController(QObject* parent)
    : QObject{parent}
    , p{std::make_unique<Private>()}
{ }

SettingsDialogController::~SettingsDialogController() = default;

void SettingsDialogController::open()
{
    openAtPage({});
}

void SettingsDialogController::openAtPage(const Id& page)
{
    auto* settingsDialog = new SettingsDialog{p->pages};
    QObject::connect(settingsDialog, &QDialog::destroyed, this,
                     [this, settingsDialog]() { p->geometry = settingsDialog->saveGeometry(); });
    settingsDialog->setAttribute(Qt::WA_DeleteOnClose);

    if(p->geometry.isEmpty()) {
        settingsDialog->resize(750, 450);
    }
    else {
        settingsDialog->restoreGeometry(p->geometry);
    }
    settingsDialog->openSettings();

    if(page.isValid()) {
        settingsDialog->openPage(page);
    }
}

void SettingsDialogController::addPage(SettingsPage* page)
{
    p->pages.push_back(page);
}

QByteArray SettingsDialogController::geometry() const
{
    return p->geometry;
}

void SettingsDialogController::updateGeometry(const QByteArray& geometry)
{
    p->geometry = geometry;
}
} // namespace Fy::Utils
