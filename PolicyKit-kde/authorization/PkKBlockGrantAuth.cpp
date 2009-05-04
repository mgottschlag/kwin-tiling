/**************************************************************************
 *   Copyright (C) 2008 Daniel Nicoletti <dantti85-pk@yahoo.com.br>        *
 *   Copyright (C) 2008 Dario Freddi <drf54321@gmail.com>                  *
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

#include "PkKBlockGrantAuth.h"
#include "PkKCommon.h"

#include <Context>

#include <unistd.h>
#include <KIcon>
#include <KDebug>

using namespace PolkitQt;

namespace PolkitKde
{

PkKBlockGrantAuth::PkKBlockGrantAuth(PolKitPolicyFileEntry *pfe, bool block, QWidget *parent)
        : KDialog(parent), m_block(block), m_pfe(pfe)
{
    polkit_policy_file_entry_ref(m_pfe);

    KIcon iconBlock("object-locked");
    KIcon iconGrant("dialog-ok");

    setupUi(mainWidget());
    setWindowIcon(m_block ? iconBlock : iconGrant);
    setModal(true);

    QPixmap icon;
    if (m_block)
        icon = KIconLoader::global()->loadIcon("object-locked",
                                               KIconLoader::NoGroup, KIconLoader::SizeHuge);
    else
        icon = KIconLoader::global()->loadIcon("dialog-ok",
                                               KIconLoader::NoGroup, KIconLoader::SizeHuge);
    iconL->setPixmap(icon);

    setCaption(m_block ? i18n("Grant Negative Authorization") : i18n("Grant Authorization"));

    QString actionId = polkit_policy_file_entry_get_id(pfe);

    label->setText(m_block ?
                   i18n("<b><big>Granting a negative authorization for the <i>%1</i> "
                        "action requires more information</big></b>", actionId) :
                   i18n("<b><big>Granting an authorization for the <i>%1</i> "
                        "action requires more information</big></b>", actionId));

    instructionsL->setText(m_block ?
                           i18n("Select the user to block for the action and, optionally, "
                                "any constraints on the negative authorization that you are about to grant.") :
                           i18n("Select the beneficiary and, optionally, how to constrain the authorization "
                                "that you are about to grant."));

    beneficiaryL->setText(m_block ?
                          i18n("Select the user that will receive the negative authorization.") :
                          i18n("Select the user that will receive the authorization."));

    setButtonText(KDialog::Ok, m_block ? i18n("Block") : i18n("Grant"));
    setButtonIcon(KDialog::Ok, m_block ? iconBlock : iconGrant);
    m_pkGrantAct = new Action(AUTH_ACTION_GRANT, this);
    connect(m_pkGrantAct, SIGNAL(dataChanged()), SLOT(actionUpdated()));
    enableButtonOk(false);

    m_pkGrantAction = polkit_action_new();
    polkit_action_set_action_id(m_pkGrantAction, AUTH_ACTION_GRANT);
    m_authDB = polkit_context_get_authorization_db(Context::instance()->getPolKitContext());

    m_userModelSIM = new QStandardItemModel(this);
    userCB->setModel(m_userModelSIM);
    m_userModelSIM->setSortRole(Qt::UserRole);
    fillCombo(false);
    connect(showSysUsersChB, SIGNAL(toggled(bool)), this, SLOT(fillCombo(bool)));
    connect(userCB, SIGNAL(currentIndexChanged(int)), this, SLOT(userIndexChanged(int)));

}

PkKBlockGrantAuth::~PkKBlockGrantAuth()
{
    polkit_policy_file_entry_unref(m_pfe);
}

void PkKBlockGrantAuth::fillCombo(bool showSystemUsers)
{
    m_userModelSIM->clear();
    QStandardItem *item;
    m_userModelSIM->appendRow(item = new QStandardItem(i18n("Select User")));
    item->setSelectable(false);
    foreach(const KUser& user, KUser::allUsers()) {
        if (!showSystemUsers) {
            /* TODO: there's probably better heuristic / tests than theses... */
            if (user.uid() < 1000) {
                continue;
            }
            if (user.shell().isEmpty() || user.shell() == "/bin/false" || user.shell() == "/sbin/nologin") {
                continue;
            }
        }

        // how to display user name and login
        QString display;
        if (!user.property(KUser::FullName).toString().isEmpty()) {
            display = user.property(KUser::FullName).toString() + " (" + user.loginName() + ')';
        } else {
            display = user.loginName();
        }
        QStandardItem *item = new QStandardItem(display);
        item->setData(user.uid(), Qt::UserRole);

        // load user icon face
        if (!user.faceIconPath().isEmpty()) {
            item->setIcon(KIcon(user.faceIconPath()));
        } else {
            item->setIcon(KIcon("user-identity"));
        }

        m_userModelSIM->appendRow(item);
    }
    m_userModelSIM->sort(0);
}

void PkKBlockGrantAuth::userIndexChanged(int index)
{
    if (m_userModelSIM->data(m_userModelSIM->index(index, 0), Qt::UserRole).toUInt()) {
        if (m_pkGrantAction && polkit_authorization_db_is_uid_blocked_by_self(m_authDB,
                m_pkGrantAction,
                getuid(),
                NULL)) {
            enableButtonOk(false);
        } else {
            enableButtonOk(m_pkGrantAct->isEnabled());
        }
    } else {
        enableButtonOk(false);
    }

    if (m_block) {
        if (m_userModelSIM->data(m_userModelSIM->index(index, 0), Qt::UserRole).toUInt() == m_currentUser.uid()) {
            setButtonIcon(KDialog::Ok, KIcon());
        } else {
            setButtonIcon(KDialog::Ok, KIcon("object-locked"));
        }
    }

}

void PkKBlockGrantAuth::actionUpdated()
{
    userIndexChanged(userCB->currentIndex());
}

void PkKBlockGrantAuth::slotButtonClicked(int button)
{
    if (button == KDialog::Ok) {
        PolKitAction *action;
        PolKitError *pk_error;
        unsigned int num_constraints;
        PolKitAuthorizationConstraint *constraints[3];

        action = polkit_action_new();
        polkit_action_set_action_id(action, polkit_policy_file_entry_get_id(m_pfe));

        num_constraints = 0;

        if (activeRB->isChecked())
            constraints[num_constraints++] = polkit_authorization_constraint_get_require_active();
        else if (consoleRB->isChecked())
            constraints[num_constraints++] = polkit_authorization_constraint_get_require_local();
        else if (activeConsoleRB->isChecked()) {
            constraints[num_constraints++] = polkit_authorization_constraint_get_require_local();
            constraints[num_constraints++] = polkit_authorization_constraint_get_require_active();
        }
        constraints[num_constraints] = NULL;

        uint selectedUID = m_userModelSIM->data(m_userModelSIM->index(userCB->currentIndex(), 0), Qt::UserRole).toUInt();
        if (selectedUID && m_pkGrantAct->activate(winId())) {
            polkit_bool_t res;
            pk_error = NULL;

            if (m_block)
                res = polkit_authorization_db_grant_negative_to_uid(m_authDB,
                        action,
                        selectedUID,
                        constraints,
                        &pk_error);
            else
                res = polkit_authorization_db_grant_to_uid(m_authDB,
                        action,
                        selectedUID,
                        constraints,
                        &pk_error);

            if (!res) {
                kDebug() << "Error granting auth: " << polkit_error_get_error_code(pk_error)
                << ": "                    << polkit_error_get_error_name(pk_error)
                << ": "                    << polkit_error_get_error_message(pk_error);
                polkit_error_free(pk_error);
            }
            KDialog::slotButtonClicked(button);
        }
    } else {
        KDialog::slotButtonClicked(button);
    }
}


}

#include "PkKBlockGrantAuth.moc"
