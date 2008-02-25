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

// Local includes
#include "placesengine.h"

// Qt includes
#include <QtCore/QString>
#include <QtCore/QVariantList>


// KDE includes
#include <KDiskFreeSpace>

PlacesEngine::PlacesEngine(QObject *parent, const QVariantList &args)
    : Plasma::DataEngine(parent, args)
{
    // dataChanged(), rowsRemoved() and setupDone() signals from
    // KFilePlacesModel are not propagated between applications.
    // layoutChanged() is not emitted at all.
    connect(&m_placesModel, SIGNAL(modelReset()),
            this, SLOT(modelReset()));
    connect(&m_placesModel, SIGNAL(rowsInserted(QModelIndex,int,int)),
            this, SLOT(placesAdded(QModelIndex,int,int)));

    sendData();
}

PlacesEngine::~PlacesEngine()
{
}

void PlacesEngine::modelReset()
{
    kDebug() << "Model reset";

    clearSources();
}

void PlacesEngine::placesAdded(const QModelIndex &parent, int start, int end)
{
    kDebug() << "Places added:" << parent << "from" << start << "to" << end;
    sendData();
}

void PlacesEngine::diskFreeSpaceFound(const QString &mountPoint,
                                      quint64 kBSize,
                                      quint64 kBUsed,
                                      quint64 kBAvailable)
{
    kDebug() << "Sir! We got one!" << mountPoint + ": "
        << "size =" << kBSize
        << "used =" << kBUsed
        << "avail =" << kBAvailable;
    QString source;

    foreach (QString testsource, sources()) {
        kDebug() << "Testing" << query(testsource)["url"];
        KUrl url(query(testsource)["url"].toString());
        if (url.isLocalFile() && url.path() == mountPoint) {
            source = testsource;
            break;
        }
    }

    kDebug() << "Source:" << source;
    if (!source.isEmpty()) {
        setData(source, "kBSize", kBSize);
        setData(source, "kBUsed", kBUsed);
        setData(source, "kBAvailable", kBAvailable);
    }
}

void PlacesEngine::tryGetFreeSpace(const KUrl &url)
{
    if (!url.isLocalFile()) {
        return;
    }

    kDebug() << "Requesting free space on" << url;

    // suicidal object: don't need to worry about cleanup
    KDiskFreeSpace *diskFreeSpace = new KDiskFreeSpace(this);
    connect(diskFreeSpace,
            SIGNAL(foundMountPoint(QString,quint64,quint64,quint64)),
            this,
            SLOT(diskFreeSpaceFound(QString,quint64,quint64,quint64)));
    diskFreeSpace->readDF(url.path());
}

void PlacesEngine::sendData()
{
    int rowCount = m_placesModel.rowCount();
    for (int i = 0; i < rowCount; ++i) {
        QModelIndex index = m_placesModel.index(i,0);

        Data map;

        QString source = QString::number(i);

        setData(source, "name", m_placesModel.text(index));
        setData(source, "url", m_placesModel.url(index).url());
        setData(source, "icon", m_placesModel.icon(index));
        setData(source, "hidden",
                m_placesModel.data(index, KFilePlacesModel::HiddenRole));
        setData(source, "setupNeeded",
                m_placesModel.data(index, KFilePlacesModel::SetupNeededRole));

        if (m_placesModel.deviceForIndex(index).isValid()) {
            setData(source, "isDevice", true);
            tryGetFreeSpace(m_placesModel.url(index));
        } else {
            setData(source, "isDevice", false);
        }
    }
}

K_EXPORT_PLASMA_DATAENGINE(places, PlacesEngine)


