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

// Qt

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

    QVector<QString> urls;
    int numIcons;

    if (favouritesConfigs.isEmpty()) {
        numIcons = 4;
        urls << "konqueror" << "kmail" << "systemsettings" << "dolphin";
    } else {
        urls.resize(favouritesConfigs.size());
        QMap<uint, KConfigGroup>::const_iterator it = favouritesConfigs.constBegin();
        int i = 0;
        while (it != favouritesConfigs.constEnd()) {
            KConfigGroup favouriteConfig = it.value();

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
        }
    }
}


void FavouritesModel::add(const QString &urlString, const QModelIndex &before)
{

    KService::Ptr service = KService::serviceByDesktopPath(urlString);

    if (!service) {
        service = KService::serviceByDesktopName(urlString);
    }

    if (!service) {
        QUrl url(urlString);
        if (!url.isValid()) {
            return;
        }

        QString query = url.fragment();
        QString runnerId = url.host();
        QString matchId = url.path();
        if (matchId.startsWith(QLatin1String("/"))) {
            matchId = matchId.remove(0, 1);
        }

        //FIXME: another inefficient async query
        runnerManager()->blockSignals(true);
        runnerManager()->execQuery(query, runnerId);
        runnerManager()->blockSignals(false);

        Plasma::QueryMatch match(runnerManager()->searchContext()->match(matchId));

        if (match.isValid()) {
            appendRow(
                StandardItemFactory::createItem(
                    match.icon(),
                    match.text(),
                    match.subtext(),
                    urlString
                    )
                );
        }
    } else {
        appendRow(
            StandardItemFactory::createItem(
                KIcon(service->icon()),
                service->name(),
                service->genericName(),
                service->entryPath()
                )
            );
    }
}

void FavouritesModel::save(KConfigGroup &cg)
{
    kDebug() << "----------------> Saving Stuff...";

    // erase the old stuff before saving the new one
    KConfigGroup oldGroup(&cg, "stripwidget");
    oldGroup.deleteGroup();

    KConfigGroup stripGroup(&cg, "stripwidget");

    for (int i = 0; i <= rowCount(); i++) {
        QModelIndex currentIndex = index(i, 0);
        KConfigGroup config(&stripGroup, QString("favourite-%1").arg(i));
        //TODO: role name
        QString url = currentIndex.data(Qt::UserRole+2).value<QString>();
        if (!url.isNull()) {
            config.writeEntry("url", url);
        }
    }
}

#include "favouritesmodel.moc"
