/*
    Copyright 2007 Robert Knight <robertknight@gmail.com>
    Copyright 2007 Kevin Ottens <ervin@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

// Own
#include "core/systemmodel.h"

// Qt
#include <QFile>
#include <QHash>
#include <QTimer>

// KDE
#include <KConfigGroup>
#include <KDiskFreeSpace>
#include <KLocalizedString>
#include <KIcon>
#include <KGlobal>
#include <KUrl>
#include <KServiceTypeTrader>
#include <KStandardDirs>
#include <kfileplacesmodel.h>
#include <solid/device.h>
#include <solid/deviceinterface.h>
#include <solid/devicenotifier.h>
#include <solid/storageaccess.h>
#include <solid/storagedrive.h>

// Local
#include "core/models.h"
#include "systemmodel.h"


using namespace Kickoff;

static const int APPLICATIONS_ROW = 0;
static const int BOOKMARKS_ROW = 1;
static const int REMOVABLE_ROW = 2;
static const int FIXED_ROW = 3;
static const int LAST_ROW = FIXED_ROW;

struct UsageInfo
{
    UsageInfo()
        : used(0),
          available(0),
          dirty(true) {}

    quint64 used;
    quint64 available;
    bool dirty;
};

class SystemModel::Private
{
public:
    Private(SystemModel *parent)
        :q(parent)
        ,placesModel(new KFilePlacesModel(parent))
    {
        q->setSourceModel(placesModel);

        connect(placesModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
                q, SLOT(sourceDataChanged(QModelIndex,QModelIndex)));
        connect(placesModel, SIGNAL(rowsAboutToBeInserted(QModelIndex,int,int)),
                q, SLOT(sourceRowsAboutToBeInserted(QModelIndex,int,int)));
        connect(placesModel, SIGNAL(rowsInserted(QModelIndex,int,int)),
                q, SLOT(sourceRowsInserted(QModelIndex,int,int)));
        connect(placesModel, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)),
                q, SLOT(sourceRowsAboutToBeRemoved(QModelIndex,int,int)));
        connect(placesModel, SIGNAL(rowsRemoved(QModelIndex,int,int)),
                q, SLOT(sourceRowsRemoved(QModelIndex,int,int)));

        topLevelSections << i18n("Applications")
                         << i18n("Places")
                         << i18n("Removable Storage")
                         << i18n("Storage");
        loadApplications();
        connect(&refreshTimer, SIGNAL(timeout()),
                q, SLOT(startRefreshingUsageInfo()));
        refreshTimer.start(10000);
        QTimer::singleShot(0, q, SLOT(startRefreshingUsageInfo()));
    }

    void queryFreeSpace(const QString& mountPoint)
    {
        KDiskFreeSpace *freeSpace = KDiskFreeSpace::findUsageInfo(mountPoint);
        connect(freeSpace, SIGNAL(foundMountPoint(QString,quint64,quint64,quint64)),
                q, SLOT(freeSpaceInfoAvailable(QString,quint64,quint64,quint64)));
    }

    void loadApplications()
    {
       KConfigGroup appsGroup = componentData().config()->group("SystemApplications");
       QStringList defaultApps;
       defaultApps << "kde4-systemsettings.desktop";
       appsList = appsGroup.readEntry("DesktopFiles", defaultApps);
    }

    SystemModel * const q;
    KFilePlacesModel *placesModel;
    QStringList topLevelSections;
    QStringList appsList;
    QList<QString> mountPointsQueue;
    QMap<QString, UsageInfo> usageByMountpoint;
    QTimer refreshTimer;
};

SystemModel::SystemModel(QObject *parent)
    : QAbstractProxyModel(parent)
    , d(new Private(this))
{
}

SystemModel::~SystemModel()
{
    delete d;
}

QModelIndex SystemModel::mapFromSource(const QModelIndex &sourceIndex) const
{
    if (!sourceIndex.isValid()) return QModelIndex();

    QModelIndex parent;

    if (!d->placesModel->isDevice(sourceIndex)) {
        parent = index(BOOKMARKS_ROW, 0);
    } else {
        Solid::Device dev = d->placesModel->deviceForIndex(sourceIndex);

        Solid::StorageDrive *drive = 0;
        Solid::Device parentDevice = dev;
        while (parentDevice.isValid() && !drive) {
            drive = parentDevice.as<Solid::StorageDrive>();
            parentDevice = parentDevice.parent();
        }

        if (drive && (drive->isHotpluggable() || drive->isRemovable())) {
            parent = index(REMOVABLE_ROW, 0);
        } else {
            parent = index(FIXED_ROW, 0);
        }
    }

    return index(sourceIndex.row(), 0, parent);
}

QModelIndex SystemModel::mapToSource(const QModelIndex &proxyIndex) const
{
    if (!proxyIndex.isValid() || !proxyIndex.parent().isValid()) {
        return QModelIndex();
    }

    return d->placesModel->index(proxyIndex.row(), proxyIndex.column());
}

QModelIndex SystemModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!parent.isValid()) {
        return createIndex(row, column, 0);
    }

    // We use the row+1 of the parent as internal Id.
    return createIndex(row, column, parent.row()+1);
}

QModelIndex SystemModel::parent(const QModelIndex &item) const
{
    if (item.internalId()>0) {
        return index(item.internalId()-1, 0);
    } else {
        return QModelIndex();
    }
}

int SystemModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid()) {
        return LAST_ROW+1;
    } else if (!parent.parent().isValid()) {
        if (parent.row()==APPLICATIONS_ROW) {
            return d->appsList.size();
        } else {
            return d->placesModel->rowCount();
        }
    } else {
        return 0;
    }
}

int SystemModel::columnCount(const QModelIndex &/*parent*/) const
{
    return 1;
}

QVariant SystemModel::data(const QModelIndex &index, int role) const
{
    if (index.internalId()==0) {
        if (role==Qt::DisplayRole) {
            return d->topLevelSections[index.row()];
        } else {
            return QVariant();
        }
    }

    if (index.internalId()-1==APPLICATIONS_ROW) {
        KService::Ptr service = KService::serviceByStorageId(d->appsList[index.row()]);

        if (!service) {
            return QVariant();
        }

        switch(role) {
        case Qt::DisplayRole:
            return service->name();
        case Qt::DecorationRole:
            return KIcon(service->icon());
        case SubTitleRole:
            return service->genericName();
        case UrlRole:
            return service->entryPath();
        default:
            return QVariant();
        }
    }

    if (role==UrlRole && !d->placesModel->isHidden(mapToSource(index))) {
        QModelIndex parent = index.parent();
        QModelIndex sourceIndex = mapToSource(index);

        bool isDevice = d->placesModel->isDevice(sourceIndex);
        bool wellPlaced = false;

        if (!isDevice && parent.row()==BOOKMARKS_ROW) {
            wellPlaced = true;
        } else if (isDevice) {
            Solid::Device dev = d->placesModel->deviceForIndex(sourceIndex);

            Solid::StorageDrive *drive = 0;
            Solid::Device parentDevice = dev;
            while (parentDevice.isValid() && !drive) {
                drive = parentDevice.as<Solid::StorageDrive>();
                parentDevice = parentDevice.parent();
            }

            bool fixed = !drive || (!drive->isHotpluggable() && !drive->isRemovable());

            if (!fixed && parent.row()==REMOVABLE_ROW) {
                wellPlaced = true;
            } else if (fixed && parent.row()==FIXED_ROW) {
                wellPlaced = true;
            }
        }

        if (wellPlaced) {
            return d->placesModel->url(sourceIndex).url();
        } else {
            return QVariant();
        }
    } else if (role==DeviceUdiRole) {
        QModelIndex sourceIndex = mapToSource(index);

        if (d->placesModel->isDevice(sourceIndex)) {
            Solid::Device dev = d->placesModel->deviceForIndex(sourceIndex);
            return dev.udi();
        } else {
            return QVariant();
        }
    } else if (role==SubTitleRole) {
        QModelIndex sourceIndex = mapToSource(index);

        if (d->placesModel->isDevice(sourceIndex)) {
            Solid::Device dev = d->placesModel->deviceForIndex(sourceIndex);
            Solid::StorageAccess *access = dev.as<Solid::StorageAccess>();

            if (access) {
                return access->filePath();
            }
        } else if (index.parent().row()!=APPLICATIONS_ROW) {
            KUrl url = d->placesModel->url(sourceIndex);
            return url.isLocalFile() ? url.path() : url.prettyUrl();
        }

        return QVariant();
    } else if (role==DiskUsedSpaceRole || role== DiskFreeSpaceRole) {
        QModelIndex sourceIndex = mapToSource(index);
        QString mp;

        if (d->placesModel->isDevice(sourceIndex)) {
            Solid::Device dev = d->placesModel->deviceForIndex(sourceIndex);
            Solid::StorageAccess *access = dev.as<Solid::StorageAccess>();

            if (access) {
                mp = access->filePath();
            }
        }

        if (!mp.isEmpty() && d->usageByMountpoint.contains(mp)) {
            UsageInfo info = d->usageByMountpoint[mp];

            if (role==DiskUsedSpaceRole) {
                return info.used;
            } else {
                return info.available;
            }
        }
    }

    return d->placesModel->data(mapToSource(index), role);
}

void SystemModel::startRefreshingUsageInfo()
{
    if (!d->mountPointsQueue.isEmpty()) {
        return;
    }

    int rowCount = d->placesModel->rowCount();
    for (int i=0; i<rowCount; ++i) {
        QModelIndex index = d->placesModel->index(i, 0);
        if (d->placesModel->isDevice(index)) {
            Solid::Device dev = d->placesModel->deviceForIndex(index);
            Solid::StorageAccess *access = dev.as<Solid::StorageAccess>();

            if (access && !access->filePath().isEmpty()) {
                d->mountPointsQueue << access->filePath();
            }
        }
    }

    if (!d->mountPointsQueue.isEmpty()) {
        d->queryFreeSpace(d->mountPointsQueue.takeFirst());
    }
}

void SystemModel::freeSpaceInfoAvailable(const QString& mountPoint, quint64,
                                         quint64 kbUsed, quint64 kbAvailable)
{
    UsageInfo info;
    info.used = kbUsed;
    info.available = kbAvailable;

    d->usageByMountpoint[mountPoint] = info;

    // More to process
    if (!d->mountPointsQueue.isEmpty()) {
        d->queryFreeSpace(d->mountPointsQueue.takeFirst());
        return;
    }

    // We're done, let's emit the changes
    int rowCount = d->placesModel->rowCount();
    for (int i=0; i<rowCount; ++i) {
        QModelIndex sourceIndex = d->placesModel->index(i, 0);
        if (d->placesModel->isDevice(sourceIndex)) {
            Solid::Device dev = d->placesModel->deviceForIndex(sourceIndex);
            Solid::StorageAccess *access = dev.as<Solid::StorageAccess>();

            if (access && d->usageByMountpoint.contains(access->filePath())) {
                info = d->usageByMountpoint[access->filePath()];

                if (info.dirty) {
                    info.dirty = false;
                    d->usageByMountpoint[access->filePath()] = info;
                } else {
                    d->usageByMountpoint.remove(access->filePath());
                }

                QModelIndex index = mapFromSource(sourceIndex);
                emit dataChanged(index, index);
            }
        }
    }
}

void Kickoff::SystemModel::sourceDataChanged(const QModelIndex &start, const QModelIndex &end)
{
    if (start.parent().isValid()) return;

    for (int row = BOOKMARKS_ROW; row<=LAST_ROW; ++row) {
        QModelIndex section = index(row, 0);

        QModelIndex new_start = index(start.row(), start.column(), section);
        QModelIndex new_end = index(end.row(), end.column(), section);
        emit dataChanged(new_start, new_end);
    }
}

void Kickoff::SystemModel::sourceRowsAboutToBeInserted(const QModelIndex &parent, int start, int end)
{
    if (parent.isValid()) return;

    for (int row = BOOKMARKS_ROW; row<=LAST_ROW; ++row) {
        QModelIndex section = index(row, 0);
        beginInsertRows(section, start, end);
    }
}

void Kickoff::SystemModel::sourceRowsInserted(const QModelIndex &parent, int /*start*/, int /*end*/)
{
    if (parent.isValid()) return;

    endInsertRows();
}

void Kickoff::SystemModel::sourceRowsAboutToBeRemoved(const QModelIndex &parent, int start, int end)
{
    if (parent.isValid()) return;

    for (int row = BOOKMARKS_ROW; row<=LAST_ROW; ++row) {
        QModelIndex section = index(row, 0);
        beginRemoveRows(section, start, end);
    }
}

void Kickoff::SystemModel::sourceRowsRemoved(const QModelIndex &parent, int /*start*/, int /*end*/)
{
    if (parent.isValid()) return;

    endRemoveRows();
}

#include "systemmodel.moc"
