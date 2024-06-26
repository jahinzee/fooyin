/*
 * Fooyin
 * Copyright © 2022, Luke Taylor <LukeT1@proton.me>
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

#include "infomodel.h"

#include <core/player/playercontroller.h>
#include <core/track.h>
#include <utils/enum.h>
#include <utils/utils.h>

#include <QFileInfo>
#include <QFont>
#include <algorithm>

constexpr auto HeaderFontDelta = 2;

namespace Fooyin {
InfoItem::InfoItem()
    : InfoItem{Header, QStringLiteral(""), nullptr, ValueType::Concat, {}}
{ }

InfoItem::InfoItem(ItemType type, QString name, InfoItem* parent, ValueType valueType)
    : InfoItem{type, std::move(name), parent, valueType, {}}
{ }

InfoItem::InfoItem(ItemType type, QString name, InfoItem* parent, ValueType valueType, FormatFunc numFunc)
    : TreeItem{parent}
    , m_type{type}
    , m_valueType{valueType}
    , m_name{std::move(name)}
    , m_numValue{0}
    , m_formatNum{std::move(numFunc)}
{ }

InfoItem::ItemType InfoItem::type() const
{
    return m_type;
}

QString InfoItem::name() const
{
    return m_name;
}

QVariant InfoItem::value() const
{
    switch(m_valueType) {
        case(ValueType::Concat): {
            if(m_value.isEmpty()) {
                m_value = m_values.join(QStringLiteral("; "));
            }
            return m_value;
        }
        case(ValueType::Average):
            if(m_numValue == 0 && !m_numValues.empty()) {
                m_numValue = std::reduce(m_numValues.cbegin(), m_numValues.cend()) / m_numValues.size();
            }
            // Fallthrough
        case(ValueType::Total):
        case(ValueType::Max):
            if(m_formatNum) {
                return m_formatNum(m_numValue);
            }
            return QVariant::fromValue(m_numValue);
    }
    return m_value;
}

void InfoItem::addTrackValue(uint64_t value)
{
    switch(m_valueType) {
        case(ValueType::Concat):
            addTrackValue(QString::number(value));
            break;
        case(ValueType::Average):
            m_numValues.push_back(value);
            break;
        case(ValueType::Total):
            m_numValue += value;
            break;
        case(ValueType::Max):
            m_numValue = std::max(m_numValue, value);
            break;
    }
}

void InfoItem::addTrackValue(int value)
{
    addTrackValue(static_cast<uint64_t>(value));
}

void InfoItem::addTrackValue(const QString& value)
{
    if(m_values.size() > 100 || m_values.contains(value) || value.isEmpty()) {
        return;
    }
    m_values.append(value);
    m_values.sort();
}

void InfoItem::addTrackValue(const QStringList& values)
{
    for(const auto& strValue : values) {
        addTrackValue(strValue);
    }
}

struct InfoModel::Private
{
    InfoModel* self;

    std::unordered_map<QString, InfoItem> nodes;

    QFont headerFont;

    explicit Private(InfoModel* self_)
        : self{self_}
    {
        headerFont.setPointSize(headerFont.pointSize() + HeaderFontDelta);
    }

    void reset()
    {
        self->resetRoot();
        nodes.clear();
    }

    InfoItem* getOrAddNode(const QString& key, const QString& name, ItemParent parent, InfoItem::ItemType type,
                           InfoItem::ValueType valueType = InfoItem::Concat, InfoItem::FormatFunc numFunc = {})
    {
        if(key.isEmpty() || name.isEmpty()) {
            return nullptr;
        }

        if(nodes.contains(key)) {
            return &nodes.at(key);
        }

        InfoItem* parentItem{nullptr};

        if(parent == ItemParent::Root) {
            parentItem = self->rootItem();
        }
        else {
            const QString parentKey = Utils::Enum::toString(parent);
            if(nodes.contains(parentKey)) {
                parentItem = &nodes.at(parentKey);
            }
        }

        if(!parentItem) {
            return nullptr;
        }

        InfoItem item{type, name, parentItem, valueType, std::move(numFunc)};
        InfoItem* node = &nodes.emplace(key, std::move(item)).first->second;
        parentItem->appendChild(node);

        return node;
    }

    void checkAddEntryNode(const QString& key, const QString& name, InfoModel::ItemParent parent)
    {
        getOrAddNode(key, name, parent, InfoItem::Entry);
    }

    template <typename Value>
    void checkAddEntryNode(const QString& key, const QString& name, InfoModel::ItemParent parent, Value&& value,
                           InfoItem::ValueType valueType = InfoItem::ValueType::Concat,
                           InfoItem::FormatFunc numFunc  = {})
    {
        if constexpr(std::is_same_v<Value, QString> || std::is_same_v<Value, QStringList>) {
            if(value.isEmpty()) {
                return;
            }
        }
        auto* node = getOrAddNode(key, name, parent, InfoItem::Entry, valueType, std::move(numFunc));
        node->addTrackValue(std::forward<Value>(value));
    }

    void addTrackNodes()
    {
        checkAddEntryNode(QStringLiteral("Artist"), tr("Artist"), ItemParent::Metadata);
        checkAddEntryNode(QStringLiteral("Title"), tr("Title"), ItemParent::Metadata);
        checkAddEntryNode(QStringLiteral("Album"), tr("Album"), ItemParent::Metadata);
        checkAddEntryNode(QStringLiteral("Date"), tr("Date"), ItemParent::Metadata);
        checkAddEntryNode(QStringLiteral("Genre"), tr("Genre"), ItemParent::Metadata);
        checkAddEntryNode(QStringLiteral("AlbumArtist"), tr("Album Artist"), ItemParent::Metadata);
        checkAddEntryNode(QStringLiteral("TrackNumber"), tr("Track Number"), ItemParent::Metadata);
        checkAddEntryNode(QStringLiteral("FileName"), tr("File Name"), ItemParent::Location);
        checkAddEntryNode(QStringLiteral("FolderName"), tr("Folder Name"), ItemParent::Location);
        checkAddEntryNode(QStringLiteral("FilePath"), tr("File Path"), ItemParent::Location);
        checkAddEntryNode(QStringLiteral("FileSize"), tr("File Size"), ItemParent::Location);
        checkAddEntryNode(QStringLiteral("LastModified"), tr("Last Modified"), ItemParent::Location);
        checkAddEntryNode(QStringLiteral("Added"), tr("Added"), ItemParent::Location);
        checkAddEntryNode(QStringLiteral("Duration"), tr("Duration"), ItemParent::General);
        checkAddEntryNode(QStringLiteral("Bitrate"), tr("Bitrate"), ItemParent::General);
        checkAddEntryNode(QStringLiteral("SampleRate"), tr("Sample Rate"), ItemParent::General);
    }

    void addTrackNodes(int total, const Track& track)
    {
        checkAddEntryNode(QStringLiteral("Artist"), tr("Artist"), ItemParent::Metadata, track.artists());
        checkAddEntryNode(QStringLiteral("Title"), tr("Title"), ItemParent::Metadata, track.title());
        checkAddEntryNode(QStringLiteral("Album"), tr("Album"), ItemParent::Metadata, track.album());
        checkAddEntryNode(QStringLiteral("Date"), tr("Date"), ItemParent::Metadata, track.date());
        checkAddEntryNode(QStringLiteral("Genre"), tr("Genre"), ItemParent::Metadata, track.genres());
        checkAddEntryNode(QStringLiteral("AlbumArtist"), tr("Album Artist"), ItemParent::Metadata, track.albumArtist());
        checkAddEntryNode(QStringLiteral("TrackNumber"), tr("Track Number"), ItemParent::Metadata, track.trackNumber());

        const QFileInfo file{track.filepath()};

        checkAddEntryNode(QStringLiteral("FileName"), total > 1 ? tr("File Names") : tr("File Name"),
                          ItemParent::Location, file.fileName());
        checkAddEntryNode(QStringLiteral("FolderName"), total > 1 ? tr("Folder Names") : tr("Folder Name"),
                          ItemParent::Location, file.absolutePath());

        if(total == 1) {
            checkAddEntryNode(QStringLiteral("FilePath"), tr("File Path"), ItemParent::Location, track.filepath());
        }

        checkAddEntryNode(QStringLiteral("FileSize"), total > 1 ? tr("Total Size") : tr("File Size"),
                          ItemParent::Location, track.fileSize(), InfoItem::Total,
                          [](uint64_t size) -> QString { return Utils::formatFileSize(size, true); });
        checkAddEntryNode(QStringLiteral("LastModified"), tr("Last Modified"), ItemParent::Location,
                          track.modifiedTime(), InfoItem::Max, Utils::formatTimeMs);

        if(total == 1) {
            checkAddEntryNode(QStringLiteral("Added"), tr("Added"), ItemParent::Location, track.addedTime(),
                              InfoItem::Max, Utils::formatTimeMs);
        }

        checkAddEntryNode(QStringLiteral("Duration"), tr("Duration"), ItemParent::General, track.duration(),
                          InfoItem::ValueType::Total, Utils::msToString);
        checkAddEntryNode(QStringLiteral("Bitrate"), total > 1 ? tr("Avg. Bitrate") : tr("Bitrate"),
                          ItemParent::General, track.bitrate(), InfoItem::Average, [](uint64_t bitrate) -> QString {
                              return QString::number(bitrate) + QStringLiteral("kbps");
                          });

        checkAddEntryNode(QStringLiteral("SampleRate"), tr("Sample Rate"), ItemParent::General,
                          QString::number(track.sampleRate()) + QStringLiteral(" Hz"));
    }
};

InfoModel::InfoModel(QObject* parent)
    : TreeModel{parent}
    , p{std::make_unique<Private>(this)}
{ }

InfoModel::~InfoModel() = default;

void InfoModel::resetModel(const TrackList& tracks, const Track& playingTrack)
{
    TrackList infoTracks{tracks};

    if(infoTracks.empty()) {
        if(playingTrack.isValid()) {
            infoTracks.push_back(playingTrack);
        }
    }

    beginResetModel();
    p->reset();

    p->getOrAddNode(QStringLiteral("Metadata"), tr("Metadata"), ItemParent::Root, InfoItem::Header);
    p->getOrAddNode(QStringLiteral("Location"), tr("Location"), ItemParent::Root, InfoItem::Header);
    p->getOrAddNode(QStringLiteral("General"), tr("General"), ItemParent::Root, InfoItem::Header);

    const int total = static_cast<int>(infoTracks.size());

    if(total > 0) {
        p->checkAddEntryNode(QStringLiteral("Tracks"), tr("Tracks"), ItemParent::General, total,
                             InfoItem::ValueType::Total);

        for(const Track& track : infoTracks) {
            p->addTrackNodes(total, track);
        }
    }
    else {
        p->addTrackNodes();
    }

    endResetModel();
}

QVariant InfoModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(role == Qt::TextAlignmentRole) {
        return (Qt::AlignHCenter);
    }

    if(role != Qt::DisplayRole || orientation == Qt::Orientation::Vertical) {
        return {};
    }

    switch(section) {
        case(0):
            return QStringLiteral("Name");
        case(1):
            return QStringLiteral("Value");
        default:
            break;
    }

    return {};
}

int InfoModel::columnCount(const QModelIndex& /*parent*/) const
{
    return 2;
}

QVariant InfoModel::data(const QModelIndex& index, int role) const
{
    if(!checkIndex(index, CheckIndexOption::IndexIsValid)) {
        return {};
    }

    const auto* item              = static_cast<InfoItem*>(index.internalPointer());
    const InfoItem::ItemType type = item->type();

    if(role == InfoItem::Type) {
        return QVariant::fromValue<InfoItem::ItemType>(type);
    }

    if(role == Qt::FontRole) {
        if(type == InfoItem::Header) {
            return p->headerFont;
        }
        return {};
    }

    if(role != Qt::DisplayRole) {
        return {};
    }

    switch(index.column()) {
        case(0):
            return item->name();
        case(1):
            return item->value();
        default:
            break;
    }

    return {};
}
} // namespace Fooyin

#include "moc_infomodel.cpp"
