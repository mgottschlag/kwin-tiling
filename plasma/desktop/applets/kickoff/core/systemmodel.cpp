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

#include "systemmodel.h"

// Qt
#include <QHash>
#include <QTimer>

// KDE
#include <KAuthorized>
#include <KDebug>
#include <KDiskFreeSpaceInfo>
#include <KIcon>
#include <KUrl>
#include <KSycoca>
#include <KFilePlacesModel>
#include <Solid/Device>
#include <Solid/DeviceInterface>
#include <Solid/DeviceNotifier>
#include <Solid/StorageAccess>
#include <Solid/StorageDrive>

// Local
#include "core/models.h"
#include "core/systemmodel.h"

using namespace Kickoff;

static const int APPLICATIONS_ROW = 0;
static const int BOOKMARKS_ROW = 1;
static const int REMOVABLE_ROW = 2;
static const int FIXED_ROW = 3;
static const int LAST_ROW = FIXED_ROW;

class SystemModel::Private
{
public:
    Private(SystemModel *parent)
            : q(parent),
              placesModel(new KFilePlacesModel(parent)),
              refreshRequested(false)
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
        connect(KSycoca::self(), SIGNAL(databaseChanged(QStringList)), q, SLOT(reloadApplications()));
    }

    SystemModel * const q;
    KFilePlacesModel *placesModel;
    QStringList topLevelSections;
    KService::List appsList;
    QMap<QString, UsageInfo> usageByMountpoint;
    QWeakPointer<UsageFinder> usageFinder;
    bool refreshRequested;
};

SystemModel::SystemModel(QObject *parent)
        : KickoffProxyModel(parent)
        , d(new Private(this))
{
    qRegisterMetaType<UsageInfo>("UsageInfo");
    reloadApplications();
}

SystemModel::~SystemModel()
{
    delete d;
}

QModelIndex SystemModel::mapFromSource(const QModelIndex &sourceIndex) const
{
    if (!sourceIndex.isValid()) {
        return QModelIndex();
    }

    QModelIndex parent;

    if (!d->placesModel->isDevice(sourceIndex)) {
        parent = index(BOOKMARKS_ROW, 0);
    } else {
        bool isFixedDevice = d->placesModel->data(sourceIndex, KFilePlacesModel::FixedDeviceRole).toBool();

        if (!isFixedDevice) {
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
    return createIndex(row, column, parent.row() + 1);
}

QModelIndex SystemModel::parent(const QModelIndex &item) const
{
    if (item.internalId() > 0) {
        return index(item.internalId() - 1, 0);
    } else {
        return QModelIndex();
    }
}

int SystemModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid()) {
        return LAST_ROW + 1;
    } else if (!parent.parent().isValid()) {
        switch (parent.row()) {
        case APPLICATIONS_ROW:
            if (KAuthorized::authorize("run_command")) {
                return d->appsList.size() + 1;
            } else {
                return d->appsList.size();
            }
            break;
        case BOOKMARKS_ROW:
            return d->placesModel->rowCount();
            break;
        case REMOVABLE_ROW:
            return d->placesModel->rowCount();
            break;
        default:
            return 0;
        }
    }

    return 0;
}

int SystemModel::columnCount(const QModelIndex &/*parent*/) const
{
    return 1;
}

QVariant SystemModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    if (index.internalId() == 0) {
        if (role == Qt::DisplayRole) {
            return d->topLevelSections[index.row()];
        } else {
            return QVariant();
        }
    }

    if (index.internalId() - 1 == APPLICATIONS_ROW) {
        if (d->appsList.count() < index.row()) {
            return QVariant();
        } else if (d->appsList.count() == index.row()) {
            // "Run Command"
            switch (role) {
                case Qt::DisplayRole:
                    return i18n("Run Command...");
                case Qt::DecorationRole:
                    return KIcon("system-run");
                case SubTitleRole:
                    return i18n("Run a command or a search query");
                case UrlRole:
                    return "run:/";
                default:
                    return QVariant();
            }
        }

        KService::Ptr service = d->appsList[index.row()];

        switch (role) {
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

    if (role == UrlRole && !d->placesModel->isHidden(mapToSource(index))) {
        QModelIndex parent = index.parent();
        QModelIndex sourceIndex = mapToSource(index);

        bool isDevice = d->placesModel->isDevice(sourceIndex);
        bool wellPlaced = false;

        if (!isDevice && parent.row() == BOOKMARKS_ROW) {
            wellPlaced = true;
        } else if (isDevice) {
            bool fixed = d->placesModel->data(sourceIndex, KFilePlacesModel::FixedDeviceRole).toBool();

            if (!fixed && parent.row() == REMOVABLE_ROW) {
                wellPlaced = true;
            } else if (fixed && parent.row() == FIXED_ROW) {
                wellPlaced = true;
            }
        }

        if (wellPlaced) {
            return d->placesModel->url(sourceIndex).url();
        } else {
            return QVariant();
        }
    } else if (role == DeviceUdiRole) {
        QModelIndex sourceIndex = mapToSource(index);

        if (d->placesModel->isDevice(sourceIndex)) {
            Solid::Device dev = d->placesModel->deviceForIndex(sourceIndex);
            return dev.udi();
        } else {
            return QVariant();
        }
    } else if (role == SubTitleRole) {
        QModelIndex sourceIndex = mapToSource(index);

        if (d->placesModel->isDevice(sourceIndex)) {
            Solid::Device dev = d->placesModel->deviceForIndex(sourceIndex);
            Solid::StorageAccess *access = dev.as<Solid::StorageAccess>();

            if (access) {
                return access->filePath();
            }
        } else if (index.parent().row() != APPLICATIONS_ROW) {
            KUrl url = d->placesModel->url(sourceIndex);
            return url.isLocalFile() ? url.toLocalFile() : url.prettyUrl();
        }

        return QVariant();
    } else if (role == DiskUsedSpaceRole || role == DiskFreeSpaceRole) {
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

            if (role == DiskUsedSpaceRole) {
                return info.used;
            } else {
                return info.available;
            }
        }
    }

    return d->placesModel->data(mapToSource(index), role);
}

QVariant SystemModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal || section != 0) {
        return QVariant();
    }

    switch (role) {
    case Qt::DisplayRole:
        return i18n("Computer");
        break;
    default:
        return QVariant();
    }
}

UsageFinder::UsageFinder(QObject *parent)
    : QThread(parent)
{

}

void UsageFinder::add(int index, const QString &mountPoint)
{
    //NOTE: this is not particularly perfect for a threaded object ;)
    //      but the assumption here is that add is called before run()
    m_toCheck.append(qMakePair(index, mountPoint));
}

void UsageFinder::run()
{
    typedef QPair<int, QString> CheckPair;
    foreach (CheckPair check, m_toCheck) {
        KDiskFreeSpaceInfo freeSpace = KDiskFreeSpaceInfo::freeSpaceInfo(check.second);
        if (freeSpace.isValid()) {
            UsageInfo info;
            info.used = freeSpace.used() / 1024;
            info.available = freeSpace.available() / 1024;
            emit usageInfo(check.first, check.second, info);
        }
    }
}

void SystemModel::refreshUsageInfo()
{
    if (d->usageFinder) {
        d->refreshRequested = true;
    } else {
        QTimer::singleShot(100, this, SLOT(startUsageInfoFetch()));
    }
}

void SystemModel::setUsageInfo(int index, const QString &mountPoint, const UsageInfo &usageInfo)
{
    QModelIndex sourceIndex = d->placesModel->index(index, 0);
    if (sourceIndex.isValid()) {
        d->usageByMountpoint[mountPoint] = usageInfo;
        QModelIndex index = mapFromSource(sourceIndex);
        emit dataChanged(index, index);
    }
}

void SystemModel::stopRefreshingUsageInfo()
{
    d->refreshRequested = false;
}

void SystemModel::usageFinderFinished()
{
    if (d->refreshRequested) {
        d->refreshRequested = false;
        QTimer::singleShot(100, this, SLOT(startUsageInfoFetch()));
    }
}

void SystemModel::startUsageInfoFetch()
{
    if (d->usageFinder) {
        return;
    }

    UsageFinder *usageFinder = new UsageFinder(this);
    d->usageFinder = usageFinder;
    connect(usageFinder, SIGNAL(finished()),
            this, SLOT(usageFinderFinished()));
    connect(usageFinder, SIGNAL(finished()),
            usageFinder, SLOT(deleteLater()));
    connect(usageFinder, SIGNAL(usageInfo(int,QString,UsageInfo)),
            this, SLOT(setUsageInfo(int,QString,UsageInfo)));

    bool hasDevices = false;

    for (int i = 0; i < d->placesModel->rowCount(); ++i) {
        QModelIndex sourceIndex = d->placesModel->index(i, 0);
        if (d->placesModel->isDevice(sourceIndex)) {
            Solid::Device dev = d->placesModel->deviceForIndex(sourceIndex);
            Solid::StorageAccess *access = dev.as<Solid::StorageAccess>();

            if (access && !access->filePath().isEmpty()) {
                usageFinder->add(i, access->filePath());
                hasDevices = true;
            }
        }
    }

    if (hasDevices) {
        usageFinder->start();
    } else {
        delete usageFinder;
    }
}

void SystemModel::reloadApplications()
{
    const QStringList apps = Kickoff::systemApplicationList();
    d->appsList.clear();

    foreach (const QString &app, apps) {
        KService::Ptr service = KService::serviceByStorageId(app);

        if (service) {
            d->appsList << service;
        }
    }
}

void Kickoff::SystemModel::sourceDataChanged(const QModelIndex &start, const QModelIndex &end)
{
    if (start.parent().isValid()) return;

    for (int row = BOOKMARKS_ROW; row <= LAST_ROW; ++row) {
        QModelIndex section = index(row, 0);

        QModelIndex new_start = index(start.row(), start.column(), section);
        QModelIndex new_end = index(end.row(), end.column(), section);
        emit dataChanged(new_start, new_end);
    }
}

void Kickoff::SystemModel::sourceRowsAboutToBeInserted(const QModelIndex &parent, int start, int end)
{
    if (parent.isValid()) return;

    for (int row = BOOKMARKS_ROW; row <= LAST_ROW; ++row) {
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

    for (int row = BOOKMARKS_ROW; row <= LAST_ROW; ++row) {
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
