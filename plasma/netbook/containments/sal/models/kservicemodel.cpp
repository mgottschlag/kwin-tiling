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
          m_path("/")
{
    QHash<int, QByteArray> newRoleNames = roleNames();
    newRoleNames[CommonModel::Description] = "description";
    newRoleNames[CommonModel::Url] = "url";
    newRoleNames[CommonModel::Weight] = "weight";
    newRoleNames[CommonModel::ActionTypeRole] = "action";

    setRoleNames(newRoleNames);

    loadRootEntries();
}

KServiceModel::~KServiceModel()
{
}

void KServiceModel::setPath(const QString &path)
{
    clear();

    if (path == "/") {
        loadRootEntries();
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

QStandardItemModel *KServiceModel::allRootEntriesModel()
{
    if (!m_allRootEntriesModel) {
        m_allRootEntriesModel = new QStandardItemModel(this);
        QSet<QString> groupSet;
        KServiceGroup::Ptr group = KServiceGroup::root();
        KServiceGroup::List list = group->entries();

        for( KServiceGroup::List::ConstIterator it = list.constBegin();
            it != list.constEnd(); it++) {
            const KSycocaEntry::Ptr p = (*it);

            if (p->isType(KST_KServiceGroup)) {
                const KServiceGroup::Ptr subGroup = KServiceGroup::Ptr::staticCast(p);

                if (!subGroup->noDisplay() && subGroup->childCount() > 0) {
                    groupSet.insert(subGroup->relPath());
                }
            }

        }

        KService::List services = KServiceTypeTrader::self()->query("Plasma/Sal/Menu");
        if (!services.isEmpty()) {
            foreach (const KService::Ptr &service, services) {
                const QUrl url = QUrl(service->property("X-Plasma-Sal-Url", QVariant::String).toString());
                const int relevance = service->property("X-Plasma-Sal-Relevance", QVariant::Int).toInt();

                if (url.scheme() != "kservicegroup" || groupSet.contains(url.path().remove(0, 1))) {
                    m_allRootEntriesModel->appendRow(
                            StandardItemFactory::createItem(
                                KIcon(service->icon()),
                                service->name(),
                                service->comment(),
                                service->storageId(),
                                relevance,
                                CommonModel::NoAction
                                )
                            );
                }
            }
        }

        setSortRole(CommonModel::Weight);
        sort(0, Qt::DescendingOrder);
    }

    return m_allRootEntriesModel;
}

void KServiceModel::loadRootEntries()
{
    QStringList defaultEnabledEntries;
    defaultEnabledEntries << "plasma-sal-contacts.desktop" << "plasma-sal-bookmarks.desktop"
        << "plasma-sal-multimedia.desktop" << "plasma-sal-internet.desktop"
        << "plasma-sal-graphics.desktop" << "plasma-sal-education.desktop"
        << "plasma-sal-games.desktop" << "plasma-sal-office.desktop";
    QSet <QString> enabledEntries = m_config.readEntry("EnabledEntries", defaultEnabledEntries).toSet();

    QSet<QString> groupSet;
    KServiceGroup::Ptr group = KServiceGroup::root();
    KServiceGroup::List list = group->entries();

    for( KServiceGroup::List::ConstIterator it = list.constBegin();
         it != list.constEnd(); it++) {
        const KSycocaEntry::Ptr p = (*it);

        if (p->isType(KST_KServiceGroup)) {
            const KServiceGroup::Ptr subGroup = KServiceGroup::Ptr::staticCast(p);

            if (!subGroup->noDisplay() && subGroup->childCount() > 0) {
                groupSet.insert(subGroup->relPath());
            }
        }

    }

    KService::List services = KServiceTypeTrader::self()->query("Plasma/Sal/Menu");
    if (!services.isEmpty()) {
        foreach (const KService::Ptr &service, services) {
            const QUrl url = QUrl(service->property("X-Plasma-Sal-Url", QVariant::String).toString());
            const int relevance = service->property("X-Plasma-Sal-Relevance", QVariant::Int).toInt();

            if (enabledEntries.contains(service->storageId()) &&
                (url.scheme() != "kservicegroup" || groupSet.contains(url.path().remove(0, 1)))) {
                appendRow(
                        StandardItemFactory::createItem(
                            KIcon(service->icon()),
                            service->name(),
                            service->comment(),
                            url.toString(),
                            relevance,
                            CommonModel::NoAction
                            )
                        );
            }
        }
    }

    setSortRole(CommonModel::Weight);
    sort(0, Qt::DescendingOrder);
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

                appendRow(
                    StandardItemFactory::createItem(
                        KIcon(service->icon()),
                        service->name(),
                        service->comment(),
                        service->entryPath(),
                        0.5,
                        CommonModel::AddAction
                        )
                    );


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
