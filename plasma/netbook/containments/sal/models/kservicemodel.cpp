/*
    Copyright 2009 Ivan Cukic <ivan.cukic+kde@gmail.com>
    Copyright 2010 Marco Martin <notmart@gmail.com>

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
#include "kservicemodel.h"
#include "commonmodel.h"

// Qt
#include <QMimeData>

// KDE
#include <KService>
#include <KIcon>
#include <KDebug>
#include <KRun>
#include <KServiceTypeTrader>
#include <KSycocaEntry>

//Plasma
#include <Plasma/AbstractRunner>
#include <Plasma/RunnerManager>


bool KServiceItemHandler::openUrl(const KUrl& url)
{
    QString urlString = url.path();
    KService::Ptr service = KService::serviceByDesktopPath(urlString);

    if (!service) {
        service = KService::serviceByDesktopName(urlString);
    }

    if (!service) {
        return false;
    }

    return KRun::run(*service, KUrl::List(), 0);
}


KServiceModel::KServiceModel(const KConfigGroup &group, QObject *parent)
        : QStandardItemModel(parent),
          m_config(group),
          m_path("/"),
          m_allRootEntriesModel(0)
{
    QHash<int, QByteArray> newRoleNames = roleNames();
    newRoleNames[CommonModel::Description] = "description";
    newRoleNames[CommonModel::Url] = "url";
    newRoleNames[CommonModel::Weight] = "weight";
    newRoleNames[CommonModel::ActionTypeRole] = "action";

    setRoleNames(newRoleNames);

    loadRootEntries(this);
}

KServiceModel::~KServiceModel()
{
}

QMimeData * KServiceModel::mimeData(const QModelIndexList &indexes) const
{
    KUrl::List urls;

    foreach (const QModelIndex & index, indexes) {
        QString urlString = data(index, CommonModel::Url).toString();

        KService::Ptr service = KService::serviceByDesktopPath(urlString);

        if (!service) {
            service = KService::serviceByDesktopName(urlString);
        }

        if (service) {
            urls << KUrl(service->entryPath());
        }
    }

    QMimeData *mimeData = new QMimeData();

    if (!urls.isEmpty()) {
        urls.populateMimeData(mimeData);
    }

    return mimeData;

}

void KServiceModel::setPath(const QString &path)
{
    clear();

    if (path == "/") {
        loadRootEntries(this);
    } else {
        loadServiceGroup(KServiceGroup::group(path));
        setSortRole(Qt::DisplayRole);
        sort(0, Qt::AscendingOrder);
    }
    m_path = path;
}

QString KServiceModel::path() const
{
    return m_path;
}

void KServiceModel::saveConfig()
{
    if (!m_allRootEntriesModel) {
        return;
    }

    QStringList enabledEntries;

    for (int i = 0; i <= m_allRootEntriesModel->rowCount() - 1; i++) {
        QModelIndex index = m_allRootEntriesModel->index(i, 0, QModelIndex());
        QStandardItem *item = m_allRootEntriesModel->itemFromIndex(index);
        if (item && item->checkState() == Qt::Checked) {
            enabledEntries << index.data(CommonModel::Url).value<QString>();
        }
    }

    m_config.writeEntry("EnabledEntries", enabledEntries);

    //sync should be kinda safe here this function is very rarely called
    m_config.sync();

    setPath("/");
}

QStandardItemModel *KServiceModel::allRootEntriesModel()
{
    if (!m_allRootEntriesModel) {
        m_allRootEntriesModel = new QStandardItemModel(this);
        loadRootEntries(m_allRootEntriesModel);
    }

    return m_allRootEntriesModel;
}

void KServiceModel::loadRootEntries(QStandardItemModel *model)
{
    QStringList defaultEnabledEntries;
    defaultEnabledEntries << "plasma-sal-contacts.desktop" << "plasma-sal-bookmarks.desktop"
        << "plasma-sal-multimedia.desktop" << "plasma-sal-internet.desktop"
        << "plasma-sal-graphics.desktop" << "plasma-sal-education.desktop"
        << "plasma-sal-games.desktop" << "plasma-sal-office.desktop";
    QSet <QString> enabledEntries = m_config.readEntry("EnabledEntries", defaultEnabledEntries).toSet();

    QHash<QString, KServiceGroup::Ptr> groupSet;
    KServiceGroup::Ptr group = KServiceGroup::root();
    KServiceGroup::List list = group->entries();

    for( KServiceGroup::List::ConstIterator it = list.constBegin();
         it != list.constEnd(); it++) {
        const KSycocaEntry::Ptr p = (*it);

        if (p->isType(KST_KServiceGroup)) {
            KServiceGroup::Ptr subGroup = KServiceGroup::Ptr::staticCast(p);

            if (!subGroup->noDisplay() && subGroup->childCount() > 0) {
                groupSet.insert(subGroup->relPath(), subGroup);
            }
        }

    }

    KService::List services = KServiceTypeTrader::self()->query("Plasma/Sal/Menu");
    if (!services.isEmpty()) {
        foreach (const KService::Ptr &service, services) {
            const QUrl url = QUrl(service->property("X-Plasma-Sal-Url", QVariant::String).toString());
            const int relevance = service->property("X-Plasma-Sal-Relevance", QVariant::Int).toInt();
            const QString groupName = url.path().remove(0, 1);

            if ((model != m_allRootEntriesModel) && enabledEntries.contains(service->storageId()) &&
                (url.scheme() != "kservicegroup" || groupSet.contains(groupName))) {
                model->appendRow(
                        StandardItemFactory::createItem(
                            KIcon(service->icon()),
                            service->name(),
                            service->comment(),
                            url.toString(),
                            relevance,
                            CommonModel::NoAction
                            )
                        );
            } else if (model == m_allRootEntriesModel && (url.scheme() != "kservicegroup" || groupSet.contains(groupName))) {
                QStandardItem * item  = StandardItemFactory::createItem(
                                KIcon(service->icon()),
                                service->name(),
                                service->comment(),
                                service->storageId(),
                                relevance,
                                CommonModel::NoAction
                                );
                    item->setCheckable(true);
                    item->setCheckState(enabledEntries.contains(service->storageId())?Qt::Checked:Qt::Unchecked);
                    model->appendRow(item);
            }

            if (groupSet.contains(groupName)) {
                groupSet.remove(groupName);
            }
        }
    }

    foreach (const KServiceGroup::Ptr group, groupSet) {
        if ((model != m_allRootEntriesModel) && enabledEntries.contains(group->relPath())) {
            model->appendRow(
                    StandardItemFactory::createItem(
                        KIcon(group->icon()),
                        group->caption(),
                        group->comment(),
                        QString("kserviceGroup://root/") + group->relPath(),
                        0.1,
                        CommonModel::NoAction
                        )
                    );
        } else if (model == m_allRootEntriesModel) {
            QStandardItem *item = StandardItemFactory::createItem(
                        KIcon(group->icon()),
                        group->caption(),
                        group->comment(),
                        group->storageId(),
                        0.1,
                        CommonModel::NoAction
                        );
            item->setCheckable(true);
            item->setCheckState(enabledEntries.contains(group->storageId())?Qt::Checked:Qt::Unchecked);
            model->appendRow(item);
        }
    }

    model->setSortRole(CommonModel::Weight);
    model->sort(0, Qt::DescendingOrder);
}

void KServiceModel::loadServiceGroup(KServiceGroup::Ptr group)
{
    if (group && group->isValid()) {
        KServiceGroup::List list = group->entries();

        for( KServiceGroup::List::ConstIterator it = list.constBegin();
             it != list.constEnd(); it++) {
            const KSycocaEntry::Ptr p = (*it);

            if (p->isType(KST_KService)) {
                const KService::Ptr service = KService::Ptr::staticCast(p);

                if (!service->noDisplay()) {
                    QString genericName = service->genericName();
                    if (genericName.isNull()) {
                        genericName = service->comment();
                    }
                    appendRow(
                        StandardItemFactory::createItem(
                            KIcon(service->icon()),
                            service->name(),
                            genericName,
                            service->entryPath(),
                            0.5,
                            CommonModel::AddAction
                            )
                        );
                }

            } else if (p->isType(KST_KServiceGroup)) {
                const KServiceGroup::Ptr subGroup = KServiceGroup::Ptr::staticCast(p);

                if (!subGroup->noDisplay() && subGroup->childCount() > 0) {
                    loadServiceGroup(subGroup);
                }
            }

        }

    }
}

#include "kservicemodel.moc"
