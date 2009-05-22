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

#ifndef PK_K_ACTION_H
#define PK_K_ACTION_H

#include "ExplicitAuthorizationsModel.h"

#include <polkit-dbus/polkit-dbus.h>

#include "ui_PkKAction.h"

namespace PolkitQt
{
    class Action;
    class ActionButton;
}

namespace PolkitKde
{

class PkKAction : public QWidget, Ui::PkKAction
{
    Q_OBJECT

public:
    explicit PkKAction(QWidget *parent = 0);
    virtual ~PkKAction();

    void setPolKitPolicyFileEntry(PolKitPolicyFileEntry *pfe);

private slots:
    void on_allowAnyCB_currentIndexChanged(int index);
    void on_allowInactiveCB_currentIndexChanged(int index);
    void on_allowActiveCB_currentIndexChanged(int index);
    void updateEntryView();

    void on_modifyPB_clicked();
    void on_revertPB_clicked();

    void on_grantPB_clicked();
    void on_blockPB_clicked();
    void on_revokePB_clicked();
    void revokeChanged();

    void explicitAuthClicked(const QModelIndex &index);
    void updateExplicitModel();

    void openUrl(const QString &url);

private:
    void checkDefaults();
    bool checkModified();

//     PolKitAction  *m_pkRevokeAction;
    PolKitAuthorizationDB *m_authdb;

    QList<PolKitResult> resultsOrdered();
    PolKitPolicyFileEntry *m_pfe;
    QFont m_fontBold, m_fontNotBold;

    ExplicitAuthorizationsModel* m_explicitModel;

    PolKitPolicyDefault *m_defaults;
    PolKitResult m_defaultsAllowAny;
    PolKitResult m_defaultsAllowInactive;
    PolKitResult m_defaultsAllowActive;

    PolKitPolicyDefault *m_factory;
    PolKitResult m_factoryAllowAny;
    PolKitResult m_factoryAllowInactive;
    PolKitResult m_factoryAllowActive;

    PolkitQt::ActionButton *m_pkModifyBt;
    PolkitQt::ActionButton *m_pkReadChB;
    PolkitQt::Action *m_pkRevokeAct;

    QString m_oldActionId;
    bool m_updatingEntry;
};


}

#endif
