/*
 *   Copyright (C) 2008 Alex Merry <alex.merry@kdemail.net>
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

#include "placesengine.h"

#include <QtCore/QString>

#include <KDebug>
#include <KDiskFreeSpaceInfo>

#include "placeservice.h"

PlacesEngine::PlacesEngine(QObject *parent, const QVariantList &args)
    : Plasma::DataEngine(parent, args)
{
    connect(&m_placesModel, SIGNAL(modelReset()),
            this, SLOT(modelReset()));
    connect(&m_placesModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
            this, SLOT(dataChanged(QModelIndex,QModelIndex)));
    connect(&m_placesModel, SIGNAL(rowsInserted(QModelIndex,int,int)),
            this, SLOT(placesAdded(QModelIndex,int,int)));
    connect(&m_placesModel, SIGNAL(rowsRemoved(QModelIndex,int,int)),
            this, SLOT(placesRemoved(QModelIndex,int,int)));

    sendAllData();
}

PlacesEngine::~PlacesEngine()
{
}

void PlacesEngine::modelReset()
{
    removeAllSources();
}

void PlacesEngine::placesAdded(const QModelIndex&, int start, int end)
{
    sendData(start, end);
}

void PlacesEngine::placesRemoved(const QModelIndex&, int start, int end)
{
    kDebug() << "Places" << start << "through" << end << "removed";
    for (int index = start; index <= end; index++) {
        removeSource(QString::number(index));
    }
}

void PlacesEngine::dataChanged(const QModelIndex& topLeft,
                               const QModelIndex& bottomRight)
{
    sendData(topLeft.row(), bottomRight.row());
}

void PlacesEngine::sendAllData()
{
    sendData(0, m_placesModel.rowCount() - 1);
}

Plasma::Service *PlacesEngine::serviceForSource(const QString &source)
{
    const int row = source.toInt();
    const QModelIndex index = m_placesModel.index(row, 0);
    if (index.isValid()) {
        return new PlaceService(this, &m_placesModel, index);
    }

    return DataEngine::serviceForSource(source);
}

void PlacesEngine::sendData(int start, int end)
{
    for (int row = start; row <= end; ++row) {
        const QModelIndex index = m_placesModel.index(row, 0);

        Data map;

        const QString source = QString::number(row);

        setData(source, "name", m_placesModel.text(index));
        setData(source, "url", m_placesModel.url(index).url());
        setData(source, "icon", m_placesModel.icon(index));
        setData(source, "hidden",
                m_placesModel.data(index, KFilePlacesModel::HiddenRole));
        setData(source, "setupNeeded",
                m_placesModel.data(index, KFilePlacesModel::SetupNeededRole));
        setData(source, "isDevice",
                m_placesModel.deviceForIndex(index).isValid());

        const QString path = m_placesModel.url(index).path();
        if (!path.isEmpty()) {
            // We can't get free space for unmounted volumes :-(
            KDiskFreeSpaceInfo info = KDiskFreeSpaceInfo::freeSpaceInfo(path);
            setData(source, "kBSize", info.size()/1024); // deprecated
            setData(source, "kBUsed", info.used()/1024); // deprecated
            setData(source, "kBAvailable", info.available()/1024); // deprecated
            setData(source, "size (bytes)", info.size());
            setData(source, "used (bytes)", info.used());
            setData(source, "available (bytes)", info.available());
        }
    }
}

K_EXPORT_PLASMA_DATAENGINE(places, PlacesEngine)

#include "placesengine.moc"

