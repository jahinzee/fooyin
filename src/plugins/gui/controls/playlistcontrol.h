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

#include <core/typedefs.h>

#include <QWidget>

namespace Gui::Widgets {
class PlaylistControl : public QWidget
{
    Q_OBJECT

public:
    explicit PlaylistControl(QWidget* parent = nullptr);
    ~PlaylistControl() override;

    void setupUi();

    void playModeChanged(Core::Player::PlayMode mode);

signals:
    void repeatClicked();
    void shuffleClicked();

protected:
    void setMode(Core::Player::PlayMode mode) const;

private:
    struct Private;
    std::unique_ptr<PlaylistControl::Private> p;
};
} // namespace Gui::Widgets