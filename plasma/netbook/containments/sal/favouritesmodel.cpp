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
#include <QTimerEvent>

// KDE
#include <KService>
#include <KStandardDirs>
#include <Plasma/AbstractRunner>
#include <Plasma/RunnerManager>
*/

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


#include "favouritesmodel.moc"
