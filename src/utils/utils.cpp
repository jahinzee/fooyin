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

#include "utils.h"

#include <QDir>
#include <QFile>
#include <QLabel>
#include <QMessageBox>
#include <QPixmap>
#include <QRandomGenerator>
#include <QStringBuilder>
#include <QVBoxLayout>
#include <QWidget>

namespace Fy::Utils {
namespace File {
QString cleanPath(const QString& path)
{
    return (path.trimmed().isEmpty()) ? QString("") : QDir::cleanPath(path);
}

bool isSamePath(const QString& filename1, const QString& filename2)
{
    const auto cleaned1 = cleanPath(filename1);
    const auto cleaned2 = cleanPath(filename2);

    return (cleaned1.compare(cleaned2) == 0);
}

bool isSubdir(const QString& dir, const QString& parentDir)
{
    if(isSamePath(dir, parentDir)) {
        return false;
    }

    const auto cleanedDir       = cleanPath(dir);
    const auto cleanedParentDir = cleanPath(parentDir);

    if(cleanedDir.isEmpty() || cleanedParentDir.isEmpty()) {
        return false;
    }

    const QFileInfo info(cleanedDir);

    QDir d1(cleanedDir);
    if(info.exists() && info.isFile()) {
        const auto d1String = getParentDirectory(cleanedDir);
        if(isSamePath(d1String, parentDir)) {
            return true;
        }

        d1 = QDir(d1String);
    }

    const QDir d2(cleanedParentDir);

    while(!d1.isRoot()) {
        d1 = QDir(getParentDirectory(d1.absolutePath()));
        if(isSamePath(d1.absolutePath(), d2.absolutePath())) {
            return true;
        }
    }

    return false;
}

bool exists(const QString& filename)
{
    return (!filename.isEmpty()) && QFile::exists(filename);
}

QString getParentDirectory(const QString& filename)
{
    const auto cleaned = cleanPath(filename);
    const auto index   = cleaned.lastIndexOf(QDir::separator());

    return (index > 0) ? cleanPath(cleaned.left(index)) : QDir::rootPath();
}

bool createDirectories(const QString& path)
{
    return QDir().mkpath(path);
}
} // namespace File

int randomNumber(int min, int max)
{
    if(min == max) {
        return max;
    }
    return QRandomGenerator::global()->bounded(min, max + 1);
}

QString msToString(uint64_t ms)
{
    constexpr auto msPerSecond = 1000;
    constexpr auto msPerMinute = msPerSecond * 60;
    constexpr auto msPerHour   = msPerMinute * 60;
    constexpr auto msPerDay    = msPerHour * 24;
    constexpr auto msPerWeek   = msPerDay * 7;

    const uint64_t weeks   = ms / msPerWeek;
    const uint64_t days    = (ms % msPerWeek) / msPerDay;
    const uint64_t hours   = (ms % msPerDay) / msPerHour;
    const uint64_t minutes = (ms % msPerHour) / msPerMinute;
    const uint64_t seconds = (ms % msPerMinute) / msPerSecond;

    QString formattedTime;

    if(weeks > 0) {
        formattedTime += QString::number(weeks) + "wk ";
    }
    if(days > 0) {
        formattedTime += QString::number(days) + "d ";
    }
    if(hours > 0) {
        formattedTime += QString{"%1:"}.arg(hours, 2, 10, QChar('0'));
    }
    if(minutes > 0 || hours > 0) {
        formattedTime += QString{"%1:"}.arg(minutes, 2, 10, QChar('0'));
    }

    formattedTime += QString{"%1"}.arg(seconds, 2, 10, QChar('0'));
    return formattedTime;
}

QString secsToString(uint64_t secs)
{
    const int seconds = static_cast<int>(secs);
    const QTime t(0, 0, 0);
    auto time = t.addSecs(seconds);
    return time.toString(time.hour() == 0 ? "mm:ss" : "hh:mm:ss");
}

uint64_t currentDateToInt()
{
    const auto str = QDateTime::currentDateTimeUtc().toString("yyyyMMddHHmmss");
    return static_cast<uint64_t>(str.toULongLong());
}

QString formatTimeMs(uint64_t time)
{
    const QDateTime dateTime  = QDateTime::fromMSecsSinceEpoch(static_cast<qint64>(time));
    QString formattedDateTime = dateTime.toString("yyyy-MM-dd HH:mm:ss");
    return formattedDateTime;
}

QString formatFileSize(uint64_t bytes)
{
    const QStringList units = {"bytes", "KB", "MB", "GB", "TB"};
    auto size               = static_cast<double>(bytes);
    int unitIndex{0};

    while(size >= 1024 && unitIndex < units.size() - 1) {
        size /= 1024;
        ++unitIndex;
    }

    const QString sizeString  = QString::number(size, 'f', 1);
    const QString& unitString = units[unitIndex];
    QString formattedSize     = sizeString % " " % unitString;

    if(unitIndex == 0) {
        return formattedSize;
    }

    const QString bytesString = QString::number(bytes);
    formattedSize             = formattedSize % " (" % bytesString % " bytes)";

    return formattedSize;
}

void setMinimumWidth(QLabel* label, const QString& text)
{
    const QString oldText = label->text();
    label->setText(text);
    label->setMinimumWidth(0);
    auto width = label->sizeHint().width();
    label->setText(oldText);

    label->setMinimumWidth(width);
}

QString capitalise(const QString& s)
{
    QStringList parts = s.split(' ', Qt::SkipEmptyParts);

    for(auto& part : parts) {
        part.replace(0, 1, part[0].toUpper());
    }

    return parts.join(" ");
}

QPixmap scaleImage(QPixmap& image, int size)
{
    static const int maximumSize = size;
    static const int scale       = 4 * maximumSize;
    const int width              = image.size().width();
    const int height             = image.size().height();
    if(width > maximumSize || height > maximumSize) {
        return image.scaled(scale, scale)
            .scaled(maximumSize, maximumSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }
    return image;
}

QPixmap changePixmapColour(const QPixmap& orig, const QColor& color)
{
    QPixmap pixmap{orig.size()};
    pixmap.fill(color);
    pixmap.setMask(orig.createMaskFromColor(Qt::transparent));
    return pixmap;
}

void showMessageBox(const QString& text, const QString& infoText)
{
    QMessageBox message;
    message.setText(text);
    message.setInformativeText(infoText);
    message.exec();
}

} // namespace Fy::Utils
