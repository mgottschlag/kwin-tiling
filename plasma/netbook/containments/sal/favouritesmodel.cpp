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
#include "favouritesmodel.h"
#include "krunnermodel.h"

         /*
// Qt
#include <QBasicTimer>
#include <QDebug>
#include <QList>
#include <QMimeData>
#include <QString>
#include <QTimerEvent>*/

// KDE
#include <KService>
#include <KIcon>
#include <KDebug>
#include <Plasma/AbstractRunner>
#include <Plasma/RunnerManager>



class FavouritesModel::Private {
public:
    Private()
    {
    }

    ~Private()
    {
    }
};

FavouritesModel::FavouritesModel(QObject *parent)
        : QStandardItemModel(parent)
        , d(new Private())
{
    QHash<int, QByteArray> newRoleNames = roleNames();
    newRoleNames[Qt::UserRole + 1] = "description";
    newRoleNames[Qt::UserRole + 2] = "url";

    setRoleNames(newRoleNames);
}

FavouritesModel::~FavouritesModel()
{
    delete d;
}

Plasma::RunnerManager *FavouritesModel::runnerManager()
{
    return KRunnerModel::runnerManager();
}

void FavouritesModel::restore(KConfigGroup &cg)
{
    kDebug() << "----------------> Restoring Stuff...";

    KConfigGroup stripGroup(&cg, "stripwidget");

    // get all the favourites
    QStringList groupNames(stripGroup.groupList());
    qSort(groupNames);
    QMap<uint, KConfigGroup> favouritesConfigs;
    foreach (const QString &favouriteGroup, stripGroup.groupList()) {
        if (favouriteGroup.startsWith("favourite-")) {
            KConfigGroup favouriteConfig(&stripGroup, favouriteGroup);
            favouritesConfigs.insert(favouriteGroup.split("-").last().toUInt(), favouriteConfig);
        }
    }

    QVector<QString> runnerIds;
    QVector<QString> queries;
    QVector<QString> matchIds;
    QVector<QString> urls;
    int numIcons;

    if (favouritesConfigs.isEmpty()) {
        numIcons = 4;
        runnerIds.resize(4);
        queries.resize(4);
        matchIds.resize(4);
        urls << "konqueror" << "kmail" << "systemsettings" << "dolphin";
    } else {
        runnerIds.resize(favouritesConfigs.size());
        queries.resize(favouritesConfigs.size());
        matchIds.resize(favouritesConfigs.size());
        urls.resize(favouritesConfigs.size());
        QMap<uint, KConfigGroup>::const_iterator it = favouritesConfigs.constBegin();
        int i = 0;
        while (it != favouritesConfigs.constEnd()) {
            KConfigGroup favouriteConfig = it.value();

            runnerIds[i] = favouriteConfig.readEntry("runnerid");
            queries[i] = favouriteConfig.readEntry("query");
            matchIds[i] = favouriteConfig.readEntry("matchId");
            urls[i] = favouriteConfig.readEntry("url");
            ++i;
            ++it;
        }
        numIcons = stripGroup.groupList().size();
    }

    QString currentQuery;
    for (int i = 0; i < numIcons; ++i ) {
        if (!urls[i].isNull()) {
            add(urls[i]);
        } else {
            // perform the query
            runnerManager()->blockSignals(true);
            const bool found = runnerManager()->execQuery(queries[i], runnerIds[i]);
            runnerManager()->blockSignals(false);
            if (currentQuery == queries[i] || found) {
                currentQuery = queries[i];
                // find our match
                Plasma::QueryMatch match(runnerManager()->searchContext()->match(matchIds[i]));

                // we should verify some other saved information to avoid putting the
                // wrong item if the search result is different!
                if (match.isValid()) {
                    add(match, queries[i]);
                }
            }
        }
    }
}

void FavouritesModel::add(Plasma::QueryMatch match, const QString &query, const QPointF &point)
{
    appendRow(
            StandardItemFactory::createItem(
                match.icon(),
                match.text(),
                match.subtext(),
                QString("krunner://") + match.runner()->id() + "/" + match.id()
                )
            );
}

void FavouritesModel::add(const QString &fileName, const QPointF &point)
{
    KService::Ptr service = KService::serviceByDesktopPath(fileName);

    if (!service) {
        service = KService::serviceByDesktopName(fileName);
        if (!service) {
            return;
        }
    }

    appendRow(
            StandardItemFactory::createItem(
                KIcon(service->icon()),
                service->name(),
                service->genericName(),
                service->entryPath()
                )
            );
}

void FavouritesModel::save(KConfigGroup &cg)
{
    kDebug() << "----------------> Saving Stuff...";

    // erase the old stuff before saving the new one
    KConfigGroup oldGroup(&cg, "stripwidget");
    oldGroup.deleteGroup();

    KConfigGroup stripGroup(&cg, "stripwidget");

    int id = 0;
    //TODO: do it with qmodelindex
/*    foreach(Plasma::IconWidget *icon, m_itemView->items()) {
        // Write now just saves one for tests. Later will save
        // all the strip
        KConfigGroup config(&stripGroup, QString("favourite-%1").arg(id));

        //config.writeEntry("icon", match->);
        config.writeEntry("text", icon->text());

        if (m_favouritesIcons.contains(icon)) {
            Plasma::QueryMatch *match = m_favouritesIcons.value(icon);
            config.writeEntry("runnerid", match->runner()->id());
            config.writeEntry("query", m_favouritesQueries.value(match));
            config.writeEntry("matchId", match->id());
            config.writeEntry("subText", match->subtext());
        } else if (m_services.contains(icon)) {
            config.writeEntry("url", (m_services[icon])->entryPath());
            config.writeEntry("subText", m_services[icon]->genericName());
        }

        ++id;
    }*/
}

#include "favouritesmodel.moc"
