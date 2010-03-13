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


KServiceModel::KServiceModel(QObject *parent)
        : QStandardItemModel(parent)
{
    QHash<int, QByteArray> newRoleNames = roleNames();
    newRoleNames[CommonModel::Description] = "description";
    newRoleNames[CommonModel::Url] = "url";
    newRoleNames[CommonModel::Weight] = "weight";
    newRoleNames[CommonModel::ActionTypeRole] = "action";

    setRoleNames(newRoleNames);

    KService::List services = KServiceTypeTrader::self()->query("Plasma/Sal/Menu");
    if (!services.isEmpty()) {
        foreach (const KService::Ptr &service, services) {
            const QString query = service->property("X-Plasma-Sal-Query", QVariant::String).toString();
            const QString runner = service->property("X-Plasma-Sal-Runner", QVariant::String).toString();
            const int relevance = service->property("X-Plasma-Sal-Relevance", QVariant::Int).toInt();

            appendRow(
                    StandardItemFactory::createItem(
                        KIcon(service->icon()),
                        service->name(),
                        service->comment(),
                        QString("krunner://") + runner + "/" + query,
                        relevance, //don't need weigt here
                        CommonModel::NoAction
                        )
                    );
        }
    }
}

KServiceModel::~KServiceModel()
{
}


#include "kservicemodel.moc"
