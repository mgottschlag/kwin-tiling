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
#include "core/recentlyusedmodel.h"

// Qt
#include <QFileInfo>

// KDE
#include <KDesktopFile>
#include <KDirWatch>
#include <KIcon>
#include <KLocalizedString>
#include <KRecentDocument>
#include <KUrl>
#include <KDebug>

// Local
#include "core/models.h"
#include "core/recentapplications.h"
#include "recentadaptor.h"

using namespace Kickoff;

class RecentlyUsedModel::Private
{
public:
    Private(RecentlyUsedModel *parent, RecentType recenttype, int maxRecentApps)
            : q(parent)
            , recenttype(recenttype)
            , maxRecentApps(maxRecentApps >= 0 ? maxRecentApps : Kickoff::RecentApplications::self()->defaultMaximum())
            , recentDocumentItem(0)
            , recentAppItem(0)
    {
    }
    void removeExistingItem(const QString& path) {
        if (!itemsByPath.contains(path)) {
            return;
        }

        QStandardItem *existingItem = itemsByPath[path];
        kDebug() << "Removing existing item" << existingItem;
        Q_ASSERT(existingItem->parent());
        existingItem->parent()->removeRow(existingItem->row());
        itemsByPath.remove(path);
    }
    void addRecentApplication(KService::Ptr service, bool append) {
        // remove existing item if any
        removeExistingItem(service->entryPath());

        QStandardItem *appItem = StandardItemFactory::createItemForService(service);
        itemsByPath.insert(service->entryPath(), appItem);

        if (append) {
            recentAppItem->appendRow(appItem);
        } else {
            recentAppItem->insertRow(0, appItem);
        }

        while (recentAppItem->rowCount() > maxRecentApps) {
            QList<QStandardItem*> row = recentAppItem->takeRow(recentAppItem->rowCount() - 1);

            //don't leave pending stuff in itemsByPath
            if (row.count() > 0) {
                itemsByPath.remove(row.first()->data(UrlRole).toString());
            }
            qDeleteAll(row.begin(), row.end());
        }
    }
    void addRecentDocument(const QString& desktopPath, bool append) {
        // remove existing item if any
        KDesktopFile desktopFile(desktopPath);
        KUrl documentUrl = desktopFile.readUrl();

        removeExistingItem(documentUrl.url());

        QStandardItem *documentItem = StandardItemFactory::createItemForUrl(desktopPath);
        documentItem->setData(true, Kickoff::SubTitleMandatoryRole);
        itemsByPath.insert(desktopPath, documentItem);

        //kDebug() << "Document item" << documentItem << "text" << documentItem->text() << "url" << documentUrl.url();
        if (append) {
            recentDocumentItem->appendRow(documentItem);
        } else {
            recentDocumentItem->insertRow(0, documentItem);
        }
    }
    void loadRecentDocuments() {
        // create branch for documents and add existing items
        recentDocumentItem = new QStandardItem(i18n("Documents"));
        QStringList documents = KRecentDocument::recentDocuments();
        foreach(const QString& document, documents) {
            addRecentDocument(document, true);
        }
        q->appendRow(recentDocumentItem);
    }
    void loadRecentApplications() {
        recentAppItem = new QStandardItem(i18n("Applications"));
        QList<KService::Ptr> services = RecentApplications::self()->recentApplications();
        for(int i = 0; i < maxRecentApps && i < services.count(); ++i) {
            addRecentApplication(services[i], true);
        }
        q->appendRow(recentAppItem);
    }

    RecentlyUsedModel * const q;
    RecentType recenttype;
    int maxRecentApps;

    QStandardItem *recentDocumentItem;
    QStandardItem *recentAppItem;
    QHash<QString, QStandardItem*> itemsByPath;
};

RecentlyUsedModel::RecentlyUsedModel(QObject *parent, RecentType recenttype, int maxRecentApps)
        : KickoffModel(parent)
        , d(new Private(this, recenttype, maxRecentApps))
{
    QDBusConnection dbus = QDBusConnection::sessionBus();
    (void)new RecentAdaptor(this);
    QDBusConnection::sessionBus().registerObject("/kickoff/RecentAppDoc", this);
    dbus.connect(QString(), "/kickoff/RecentAppDoc", "org.kde.plasma", "clearRecentDocumentsAndApplications", this, SLOT(clearRecentDocumentsAndApplications()));

    if(recenttype != DocumentsOnly) {
        d->loadRecentApplications();

        // listen for changes to the list of recent applications
        connect(RecentApplications::self(), SIGNAL(applicationAdded(KService::Ptr, int)),
                this, SLOT(recentApplicationAdded(KService::Ptr, int)));
        connect(RecentApplications::self(), SIGNAL(applicationRemoved(KService::Ptr)),
                this, SLOT(recentApplicationRemoved(KService::Ptr)));
        connect(RecentApplications::self(), SIGNAL(cleared()),
                this, SLOT(recentApplicationsCleared()));
    }
    if(recenttype != ApplicationsOnly) {
        d->loadRecentDocuments();

        // listen for changes to the list of recent documents
        KDirWatch *recentDocWatch = new KDirWatch(this);
        recentDocWatch->addDir(KRecentDocument::recentDocumentDirectory(), KDirWatch::WatchFiles);
        connect(recentDocWatch, SIGNAL(created(QString)), this, SLOT(recentDocumentAdded(QString)));
        connect(recentDocWatch, SIGNAL(deleted(QString)), this, SLOT(recentDocumentRemoved(QString)));
    }
}
RecentlyUsedModel::~RecentlyUsedModel()
{
    delete d;
}

void RecentlyUsedModel::recentDocumentAdded(const QString& path)
{
    kDebug() << "Recent document added" << path;
    d->addRecentDocument(path, false);
}
void RecentlyUsedModel::recentDocumentRemoved(const QString& path)
{
    kDebug() << "Recent document removed" << path;
    d->removeExistingItem(path);
}

void RecentlyUsedModel::recentApplicationAdded(KService::Ptr service, int)
{
    if (service) {
        d->addRecentApplication(service, false);
    }
}

void RecentlyUsedModel::recentApplicationRemoved(KService::Ptr service)
{
    if (service) {
        d->removeExistingItem(service->entryPath());
    }
}

void RecentlyUsedModel::recentApplicationsCleared()
{
    QSet<QStandardItem*> appItems;
    const int rows = d->recentAppItem->rowCount();
    for (int i = 0;i < rows;i++) {
        appItems << d->recentAppItem->child(i);
    }
    QMutableHashIterator<QString, QStandardItem*> iter(d->itemsByPath);
    while (iter.hasNext()) {
        iter.next();
        if (appItems.contains(iter.value())) {
            iter.remove();
        }
    }

    d->recentAppItem->removeRows(0, d->recentAppItem->rowCount());
}

void RecentlyUsedModel::clearRecentApplications()
{
    RecentApplications::self()->clear();
}
void RecentlyUsedModel::clearRecentDocuments()
{
    KRecentDocument::clear();
}

void RecentlyUsedModel::clearRecentDocumentsAndApplications()
{
    clearRecentDocuments();
    clearRecentApplications();
}


#include "recentlyusedmodel.moc"

