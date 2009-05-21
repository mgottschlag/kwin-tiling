/***************************************************************************
 *   Copyright (C) 2008 by Daniel Nicoletti                                *
 *   dantti85-pk@yahoo.com.br                                              *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#ifndef PK_K_AUTHORIZATIONS_H
#define PK_K_AUTHORIZATIONS_H

#include <QSocketNotifier>

#include <polkit-dbus/polkit-dbus.h>

#include <kdemacros.h>

#include "PkKAction.h"

#include "ui_PkKAuthorization.h"

namespace PolkitKde
{

class PoliciesModel;
class AuthorizationsModel;
class AuthorizationsFilterModel;

class KDE_EXPORT PkKAuthorization : public QWidget, Ui::PkKAuthorization
{
    Q_OBJECT

public:
    PkKAuthorization(QWidget *parent = 0);
    ~PkKAuthorization();

public slots:
    void newAction(const QString &action);

private slots:
    void UpdateActionTree();
    void actionClicked(const QModelIndex &index);
    void setFilterRegExp(const QString &filter);

private:
    PolKitContext *m_pkContext;
    PoliciesModel *m_model;
    QList<PolKitPolicyFileEntry *> pkPFileEntry;

    PkKAction *m_pkKAction;
    bool m_displayingAction;

    QMap<int, QSocketNotifier*> m_watches;
    AuthorizationsFilterModel *m_proxyModel;
    static polkit_bool_t
    buildActionList(PolKitPolicyCache *policy_cache, PolKitPolicyFileEntry *pfe, void *user_data);
};


}

#endif
