/*
 *   Copyright (C) 2007 Alex Merry <huntedhacker@tiscali.co.uk>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2 as
 *   published by the Free Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef PLACESENGINE_H
#define PLACESENGINE_H

#include <plasma/dataengine.h>

#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>

#include <kfileplacesmodel.h>

class PlacesEngine : public Plasma::DataEngine
{
    Q_OBJECT

public:
    PlacesEngine(QObject* parent, const QVariantList& args);
    ~PlacesEngine();

private Q_SLOTS:
    // KFilePlacesModel
    void modelReset();
    void placesAdded(const QModelIndex &parent, int start, int end);

    // KFreeDiskSpace
    void diskFreeSpaceFound(const QString &mountPoint,
                            quint64 kBSize,
                            quint64 kBUsed,
                            quint64 kBAvailable);

private:
    void tryGetFreeSpace(const KUrl &url);

    void sendData();

    KFilePlacesModel m_placesModel;
};


#endif // PLACESENGINE_H
