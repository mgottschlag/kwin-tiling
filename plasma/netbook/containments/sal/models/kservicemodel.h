/*
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

#ifndef KSERVICEMODEL_H
#define KSERVICEMODEL_H

#include <QStandardItemModel>

#include <KUrl>
#include <KServiceGroup>
#include <KConfigGroup>

#include "standarditemfactory.h"

namespace Plasma {
}

namespace KServiceItemHandler {
    bool openUrl(const KUrl& url);
}

class  KServiceModel : public QStandardItemModel
{
    Q_OBJECT

public:
    KServiceModel(const KConfigGroup &group, QObject *parent);
    virtual ~KServiceModel();

    virtual QMimeData *mimeData(const QModelIndexList &indexes) const;

    void setPath(const QString &path);
    QString path() const;

    QStandardItemModel *allRootEntriesModel();

public Q_SLOTS:
    void saveConfig();

protected:
    void loadRootEntries(QStandardItemModel *model);
    void loadServiceGroup(KServiceGroup::Ptr group);

private:
    KConfigGroup m_config;
    QString m_path;
    QStandardItemModel *m_allRootEntriesModel;
};

#endif // KSERVICEMODEL_H

