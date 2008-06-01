/*
 *   Copyright © 2008 Fredrik Höglund <fredrik@kde.org>
 *   Copyright © 2008 Rafael Fernández López <ereslibre@kde.org>
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Library General Public
 *   License as published by the Free Software Foundation; either
 *   version 2 of the License, or (at your option) any later version.
 *
 *   This library is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *   Library General Public License for more details.
 *
 *   You should have received a copy of the GNU Library General Public License
 *   along with this library; see the file COPYING.LIB.  If not, write to
 *   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *   Boston, MA 02110-1301, USA.
 */

#include "proxymodel.h"

#include <KDirModel>


ProxyModel::ProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
}

ProxyModel::~ProxyModel()
{
}

QModelIndex ProxyModel::indexForUrl(const KUrl &url) const
{
    const KDirModel *dirModel = static_cast<KDirModel*>(sourceModel());
    return mapFromSource(dirModel->indexForUrl(url));
}

KFileItem ProxyModel::itemForIndex(const QModelIndex &index) const
{
    const KDirModel *dirModel = static_cast<KDirModel*>(sourceModel());
    return dirModel->itemForIndex(mapToSource(index));
}

bool ProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
    const KDirModel *dirModel = static_cast<KDirModel*>(sourceModel());
    const KFileItem item1 = dirModel->itemForIndex(left);
    const KFileItem item2 = dirModel->itemForIndex(right);

    // Sort directories first
    if (item1.isDir() && !item2.isDir())
        return true;

    if (!item1.isDir() && item2.isDir())
        return false;

    return QSortFilterProxyModel::lessThan(left, right);
}

bool ProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    const KDirModel *dirModel = static_cast<KDirModel*>(sourceModel());
    const KFileItem item = dirModel->itemForIndex(dirModel->index(sourceRow, KDirModel::Name, sourceParent));

    const QString regExpOrig = filterRegExp().pattern();
    const QStringList regExps = regExpOrig.split(';');
    foreach (const QString &regExpStr, regExps) {
        QRegExp regExp(regExpStr);
        regExp.setPatternSyntax(QRegExp::Wildcard);

        if (regExp.indexIn(item.name()) != -1) {
            return true;
        }
    }

    return false;
}

