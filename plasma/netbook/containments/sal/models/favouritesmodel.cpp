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
#include "kservicemodel.h"
#include "commonmodel.h"

// Qt

// KDE
#include <KService>
#include <KIcon>
#include <KDebug>
#include <KRun>

//Plasma
#include <Plasma/AbstractRunner>
#include <Plasma/RunnerManager>


FavouritesModel::FavouritesModel(QObject *parent)
        : QStandardItemModel(parent)
{
    QHash<int, QByteArray> newRoleNames = roleNames();
    newRoleNames[CommonModel::Description] = "description";
    newRoleNames[CommonModel::Url] = "url";
    newRoleNames[CommonModel::Weight] = "weight";
    newRoleNames[CommonModel::ActionTypeRole] = "action";

    setRoleNames(newRoleNames);
}

FavouritesModel::~FavouritesModel()
{
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

    for (int i = 0; i < numIcons; ++i ) {
        if (!urls[i].isNull()) {
            add(urls[i]);
        }
    }
}


void FavouritesModel::add(const QUrl &url, const QModelIndex &before)
{

    KService::Ptr service = KService::serviceByDesktopPath(url.path());

    if (!service) {
        service = KService::serviceByDesktopName(url.path());
    }

    if (!service) {
        if (!url.isValid()) {
            return;
        }

        QString query = url.path();
        QString runnerId = url.host();
        QString matchId = url.fragment();
        if (matchId.startsWith(QLatin1Char('/'))) {
            matchId = matchId.remove(0, 1);
        }

        //FIXME: another inefficient async query
        runnerManager()->blockSignals(true);
        runnerManager()->execQuery(query, runnerId);
        runnerManager()->blockSignals(false);

        Plasma::QueryMatch match(runnerManager()->searchContext()->match(matchId));

        if (match.isValid()) {
            if (before.isValid()) {
                insertRow(
                    before.row(),
                    StandardItemFactory::createItem(
                        match.icon(),
                        match.text(),
                        match.subtext(),
                        url.path(),
                        1, //don't need weigt here
                        CommonModel::RemoveAction
                        )
                    );
            } else {
                appendRow(
                    StandardItemFactory::createItem(
                        match.icon(),
                        match.text(),
                        match.subtext(),
                        url.path(),
                        1, //don't need weigt here
                        CommonModel::RemoveAction
                        )
                    );
            }
        }
    } else {
        if (before.isValid()) {
            insertRow(
                before.row(),
                StandardItemFactory::createItem(
                    KIcon(service->icon()),
                    service->name(),
                    service->genericName(),
                    service->entryPath(),
                    1, //don't need weigt here
                    CommonModel::RemoveAction
                    )
                );
        } else {
            appendRow(
                StandardItemFactory::createItem(
                    KIcon(service->icon()),
                    service->name(),
                    service->genericName(),
                    service->entryPath(),
                    1, //don't need weigt here
                    CommonModel::RemoveAction
                    )
                );
        }
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
        QString url = currentIndex.data(CommonModel::Url).value<QString>();
        if (!url.isNull()) {
            config.writeEntry("url", url);
        }
    }
}

#include "favouritesmodel.moc"
