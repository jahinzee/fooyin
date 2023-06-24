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

#include "stringfuncs.h"

namespace Fy::Core::Scripting {
QString num(const QStringList& vec)
{
    if(vec.empty() || vec.size() > 2) {
        return {};
    }
    if(vec.size() == 1) {
        return vec[0];
    }
    return QStringLiteral("%1").arg(vec[0].toInt(), vec[1].toInt(), 10, QLatin1Char('0'));
}
} // namespace Fy::Core::Scripting