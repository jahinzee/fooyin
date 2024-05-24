/*
 * Fooyin
 * Copyright © 2024, Luke Taylor <LukeT1@proton.me>
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

#include <QToolButton>

namespace Fooyin {
class ToolButton : public QToolButton
{
    Q_OBJECT

public:
    explicit ToolButton(QWidget* parent = nullptr);

    void setStretchEnabled(bool enabled);

    void setIconPadding(int padding);
    void setMinimumIconSize(int size);
    void setMaximumIconSize(int size);

signals:
    void entered();

protected:
    void enterEvent(QEnterEvent* event) override;
    void paintEvent(QPaintEvent* event) override;

private:
    int m_padding;
    int m_minimumSize;
    int m_maximumSize;
    bool m_stretchEnabled;
};
} // namespace Fooyin
