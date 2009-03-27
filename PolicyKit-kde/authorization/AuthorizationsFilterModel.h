/*  This file is part of the KDE project
    Copyright (C) 2008 Dario Freddi <drf54321@gmail.com>
    Copyright (C) 2009 Daniel Nicoletti <dantti85-pk@yahoo.com.br>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public
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

#ifndef AUTHORIZATIONSFILTERMODEL_H
#define AUTHORIZATIONSFILTERMODEL_H

#include <QSortFilterProxyModel>

namespace PolkitKde
{

class AuthorizationsFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    AuthorizationsFilterModel(QObject *parent = 0);
    ~AuthorizationsFilterModel();

protected:
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const;

private:
    bool groupHasMatchingItem(const QModelIndex &parent) const;
};

}

#endif /* AUTHORIZATIONSFILTERMODEL_H */
