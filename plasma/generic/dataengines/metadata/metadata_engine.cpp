/*
 * Copyright (C) 2009 Anne-Marie Mahfouf <annma@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "metadata_engine.h"

#include <QDateTime>
#include <QUrl>

#include <kfilemetainfo.h>

// Nepomuk
#include <Nepomuk/Resource>
#include <Nepomuk/ResourceManager>
#include <Nepomuk/Tag>

MetaDataEngine::MetaDataEngine(QObject* parent, const QVariantList& args) :
        Plasma::DataEngine(parent, args)
{
}

MetaDataEngine::~MetaDataEngine()
{
}

void MetaDataEngine::init()
{
    Nepomuk::ResourceManager::instance()->init();
}

bool MetaDataEngine::sourceRequestEvent(const QString &name)
{
    updateSourceEvent(name);
    return true;
}

bool MetaDataEngine::updateSourceEvent(const QString &name)
{
    // Get picture general properties through KFileMetaInfo
    const KFileMetaInfo::WhatFlags flags = KFileMetaInfo::Fastest |
                                           KFileMetaInfo::TechnicalInfo |
                                           KFileMetaInfo::ContentInfo;
    const KFileMetaInfo fileMetaInfo(name, QString(), flags);
    if (fileMetaInfo.isValid()) {
        const QHash<QString, KFileMetaInfoItem>& items = fileMetaInfo.items();
        QHash<QString, KFileMetaInfoItem>::const_iterator it = items.constBegin();
        const QHash<QString, KFileMetaInfoItem>::const_iterator end = items.constEnd();
        QString labelText;
        while (it != end) {
            const KFileMetaInfoItem& metaInfoItem = it.value();
            const QVariant& value = metaInfoItem.value();

            if (value.isValid() && convertMetaInfo(metaInfoItem.name(), labelText)) {
                if (labelText == i18nc("@label", "Size")) {
                    const QString s = QString::number(value.toDouble()/1024/1024, 'f', 2);
                    setData(name, labelText, QString::fromLatin1("%1 Mb").arg(s));
                } else if (labelText == i18nc("@label", "Source Modified")) {
                    QVariant newValue = QVariant(value.toDateTime());
                    setData(name, labelText, newValue.toString());
                } else {
                    setData(name, labelText, value.toString());
                }
            }

            ++it;
        }
    }

    // Get picture tags through Nepomuk
    QUrl uri = QUrl(name);
    QStringList tags;
    Nepomuk::Resource res( uri);
    QList<Nepomuk::Tag> picTags = res.tags();
    Q_FOREACH(const Nepomuk::Tag& tag, picTags) {
        tags.append(tag.label() + ", ");
    }

    setData(name, "tags", tags);
    // Get picture comment through Nepomuk
    setData(name, "comment", res.description());
    // Get picture rating through Nepomuk
    setData(name, "rating", QString::number(res.rating()));

    return true;
}

bool MetaDataEngine::convertMetaInfo(const QString& key, QString& text) const
{
    struct MetaKey {
        const char* key;
        QString text;
    };

    // keys list, more can be added
    static const MetaKey keys[] = {
    { "http://freedesktop.org/standards/xesam/1.0/core#cameraModel", i18nc("@label", "Model") },
    { "http://freedesktop.org/standards/xesam/1.0/core#focalLength", i18nc("@label", "Focal Length") },
    { "http://freedesktop.org/standards/xesam/1.0/core#mimeType", i18nc("@label", "Mime Type") },
    { "http://freedesktop.org/standards/xesam/1.0/core#cameraManufacturer", i18nc("@label", "Manufacturer") },
    { "http://freedesktop.org/standards/xesam/1.0/core#sourceModified", i18nc("@label", "Source Modified") },
    { "http://freedesktop.org/standards/xesam/1.0/core#orientation", i18nc("@label", "Orientation") },
    { "http://freedesktop.org/standards/xesam/1.0/core#flashUsed", i18nc("@label", "Flash Used") },
    { "http://freedesktop.org/standards/xesam/1.0/core#height", i18nc("@label", "Height") },
    { "http://freedesktop.org/standards/xesam/1.0/core#width",  i18nc("@label", "Width") },
    { "http://freedesktop.org/standards/xesam/1.0/core#url", i18nc("@label", "Url") },    
    { "http://freedesktop.org/standards/xesam/1.0/core#size", i18nc("@label", "Size") },
    { "http://freedesktop.org/standards/xesam/1.0/core#aperture", i18nc("@label", "Aperture") },
    { "http://freedesktop.org/standards/xesam/1.0/core#meteringMode", i18nc("@label", "Metering Mode") },
    { "http://freedesktop.org/standards/xesam/1.0/core#35mmEquivalent", i18nc("@label", "35mm Equivalent") },
    { "http://freedesktop.org/standards/xesam/1.0/core#fileExtension", i18nc("@label", "File Extension") },
    { "http://freedesktop.org/standards/xesam/1.0/core#name", i18nc("@label", "Name") },
    { "http://freedesktop.org/standards/xesam/1.0/core#exposureTime", i18nc("@label", "Exposure Time") }
    };

    // search if the key exists and get its value
    int top = 0;
    int bottom = sizeof(keys) / sizeof(MetaKey) - 1;
    while (top <= bottom) {
        const int result = key.compare(keys[top].key);
        if (result == 0) {
            text = keys[top].text;
            return true;
        }
        top++;
    }

    return false;
}

K_EXPORT_PLASMA_DATAENGINE(metadata, MetaDataEngine)

#include "metadata_engine.moc"
