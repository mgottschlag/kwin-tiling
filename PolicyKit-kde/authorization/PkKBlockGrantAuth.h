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

#ifndef PK_K_BLOCK_GRANT_AUTH_H
#define PK_K_BLOCK_GRANT_AUTH_H

#include <KDialog>
#include <KUser>

#include <QStandardItemModel>

#include <polkit-dbus/polkit-dbus.h>
#include <Action>

#include "ui_PkKBlockGrantAuth.h"

namespace PolkitKde
{

class PkKBlockGrantAuth : public KDialog, Ui::PkKBlockGrantAuth
{
    Q_OBJECT

public:
    PkKBlockGrantAuth(PolKitPolicyFileEntry *pfe, bool block = false, QWidget *parent = 0);
    ~PkKBlockGrantAuth();

private slots:
    void fillCombo(bool showSystemUsers);
    void userIndexChanged(int index);
    void actionUpdated();

    virtual void slotButtonClicked(int button);

private:
    QStandardItemModel *m_userModelSIM;
    bool m_block;
    KUser m_currentUser;

    PolKitAction *m_pkGrantAction;
    PolKitAuthorizationDB *m_authDB;
    PolKitPolicyFileEntry *m_pfe;

    PolkitQt::Action *m_pkGrantAct;
};


}

#endif
