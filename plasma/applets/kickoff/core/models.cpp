/*
    Copyright 2007 Robert Knight <robertknight@gmail.com>

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
#include "core/models.h"
#include "core/leavemodel.h"

// Qt
#include <QFileInfo>
#include <QStandardItem>
#include <QDir>

// KDE
#include <KDebug>
#include <KConfigGroup>
#include <KDesktopFile>
#include <KIcon>
#include <KGlobal>
#include <KMimeType>
#include <KUrl>
#include <Solid/Device>
#include <Solid/StorageAccess>
#include <Solid/StorageDrive>

using namespace Kickoff;

namespace Kickoff
{

Q_GLOBAL_STATIC_WITH_ARGS(KUrl, homeUrl, (QDir::homePath()))
Q_GLOBAL_STATIC_WITH_ARGS(KUrl, remoteUrl, ("remote:/"))
K_GLOBAL_STATIC(StandardItemFactoryData, factoryData)

StandardItemFactoryData* deviceFactoryData()
{
    return factoryData;
}
} // namespace Kickoff

QStandardItem *StandardItemFactory::createItemForUrl(const QString& urlString)
{
    KUrl url(urlString);

    QStandardItem *item = 0;

    if (url.isLocalFile() && urlString.endsWith(".desktop")) {
        // .desktop files may be services (type field == 'Application' or 'Service')
        // or they may be other types such as links.
        //
        // first look in the KDE service database to see if this file is a service,
        // otherwise represent it as a generic .desktop file
        KService::Ptr service = KService::serviceByDesktopPath(url.path());
        if (service) {
            return createItemForService(service);
        }

        item = new QStandardItem;
        KDesktopFile desktopFile(url.path());
        item->setText(QFileInfo(urlString.mid(0, urlString.lastIndexOf('.'))).completeBaseName());
        item->setIcon(KIcon(desktopFile.readIcon()));

        //FIXME: desktopUrl is a hack around borkage in KRecentDocuments which
        //       stores a path in the URL field!
        KUrl desktopUrl(desktopFile.desktopGroup().readPathEntry("URL", QString()));
        if (!desktopUrl.url().isEmpty()) {
            item->setData(desktopUrl.url(), Kickoff::UrlRole);
        } else {
            // desktopUrl.url() is empty if the file doesn't exist so set the
            // url role to that which was passed so that the item can still be
            // manually removed
            item->setData(urlString, Kickoff::UrlRole);
        }

        QString subTitle = desktopUrl.isLocalFile() ? desktopUrl.path() : desktopUrl.prettyUrl();
        item->setData(subTitle, Kickoff::SubTitleRole);

        setSpecialUrlProperties(desktopUrl, item);
    }
    else if (url.scheme() == "leave") {
        item = LeaveModel::createStandardItem(urlString);
    }
    else {
        item = new QStandardItem;
        const QString subTitle = url.isLocalFile() ? url.path() : url.prettyUrl();
        QString basename = QFileInfo(urlString).completeBaseName();
        if (basename.isNull())
            basename = subTitle;
        item->setText(basename);
        item->setIcon(KIcon(KMimeType::iconNameForUrl(url)));
        item->setData(url.url(), Kickoff::UrlRole);
        item->setData(subTitle, Kickoff::SubTitleRole);

        setSpecialUrlProperties(url, item);
    }

    return item;
}

void StandardItemFactory::setSpecialUrlProperties(const KUrl& url,QStandardItem *item)
{
    // specially handled URLs
    if (homeUrl() && url == *homeUrl()) {
        item->setText(i18n("Home Folder"));
        item->setIcon(KIcon("user-home"));
    } else if (remoteUrl() && url == *remoteUrl()) {
        item->setText(i18n("Network Folders"));
    }
}

QStandardItem *StandardItemFactory::createItemForService(KService::Ptr service)
{
    QStandardItem *appItem = new QStandardItem;

    QString genericName = service->genericName();
    QString appName = service->name();

    appItem->setText(genericName.isEmpty() ? appName : genericName);
    appItem->setIcon(KIcon(service->icon()));
    appItem->setData(service->entryPath(),Kickoff::UrlRole);

    if (!genericName.isEmpty()) {
        appItem->setData(service->name(),Kickoff::SubTitleRole);
    }

    return appItem;
}

bool Kickoff::isLaterVersion(KService::Ptr first , KService::Ptr second)
{
    // a very crude heuristic using the .desktop path names
    // which only understands kde3 vs kde4
    bool firstIsKde4 = first->entryPath().contains("kde4");
    bool secondIsKde4 = second->entryPath().contains("kde4");

    return firstIsKde4 && !secondIsKde4;
}

QStringList Kickoff::systemApplicationList()
{
    KConfigGroup appsGroup = componentData().config()->group("SystemApplications");
    QStringList apps;
    apps << "systemsettings";
    apps = appsGroup.readEntry("DesktopFiles", apps);
    return apps;
}

#if 0
void Kickoff::swapModelIndexes(QModelIndex& first,QModelIndex& second)
{
    Q_ASSERT(first.isValid());
    Q_ASSERT(second.isValid());

    QAbstractItemModel *firstModel = const_cast<QAbstractItemModel*>(first.model());
    QAbstractItemModel *secondModel = const_cast<QAbstractItemModel*>(second.model());

    Q_ASSERT(firstModel && secondModel);

    QMap<int,QVariant> firstIndexData = firstModel->itemData(first);
    QMap<int,QVariant> secondIndexData = secondModel->itemData(second);

    firstModel->setItemData(first,secondIndexData);
    secondModel->setItemData(second,firstIndexData);
}
#endif

K_GLOBAL_STATIC_WITH_ARGS(KComponentData,kickoffComponent,("kickoff",QByteArray(),KComponentData::SkipMainComponentRegistration))
KComponentData Kickoff::componentData()
{
    return *kickoffComponent;
}



