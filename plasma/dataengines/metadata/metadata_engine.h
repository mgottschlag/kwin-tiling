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

#ifndef METADATA_ENGINE_H_
#define METADATA_ENGINE_H_

#include <Plasma/DataEngine>


class MetaDataEngine: public Plasma::DataEngine
{
    Q_OBJECT

public:
    MetaDataEngine(QObject* parent, const QVariantList& args);
    ~MetaDataEngine();

protected:
    void init();
    bool sourceRequestEvent(const QString& name);

protected slots:
    bool updateSourceEvent(const QString& source);

private:
    /**
     * Converts the meta key \a key to a readable format into \a text.
     * Returns true, if the string \a key represents a meta information
     * that should be shown. If false is returned, \a text is not modified.
     */
    bool convertMetaInfo(const QString& key, QString& text) const;

};

#endif /* METADATA_ENGINE_H_ */
