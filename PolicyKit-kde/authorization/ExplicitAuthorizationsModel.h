/*  This file is part of the KDE project
Copyright (C) 2008 Trever Fischer <wm161@wm161.net>

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

#ifndef EXPLICITAUTHORIZATIONSMODEL_H
#define EXPLICITAUTHORIZATIONSMODEL_H

#include <QStandardItemModel>
#include <QStringList>
#include <polkit-dbus/polkit-dbus.h>

namespace PolkitKde
{

class ExplicitAuthorizationsModel : public QStandardItemModel
{
    Q_OBJECT
public:
    enum {
        PolkitAuthRole = 42
    };

    ExplicitAuthorizationsModel(QObject* parent = 0);
    virtual ~ExplicitAuthorizationsModel();

    void clear();

    void addAuth(PolKitAuthorization* auth);
    PolKitAuthorization* authEntry(const QModelIndex &index) const;

private:
    void setHeaders();
    static polkit_bool_t
    buildConstraintList(PolKitAuthorization *auth, PolKitAuthorizationConstraint *authc, void *user_data);
    QStringList m_constraintList;
    PolKitTracker *m_pkTracker;
};


}

Q_DECLARE_METATYPE(PolKitAuthorization*)
#endif
