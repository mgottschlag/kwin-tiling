/*
 *   Copyright 2009 Artur Duque de Souza <asouza@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2 or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "salengine.h"
#include "salservice.h"


SearchLaunchEngine::SearchLaunchEngine(QObject* parent, const QVariantList& args)
    : Plasma::DataEngine(parent, args)
{
    Q_UNUSED(args);
}

void SearchLaunchEngine::init()
{
    setData("query", QString());
}

Plasma::Service *SearchLaunchEngine::serviceForSource(const QString &source)
{
    if (source != "query") {
        return 0;
    }

    SearchLaunchService *service = new SearchLaunchService(this);
    service->setParent(this);
    return service;
}

K_EXPORT_PLASMA_DATAENGINE(searchlaunch, SearchLaunchEngine)

#include "salengine.moc"
