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

#include "PkKAction.h"
#include <unistd.h>
#include <KToolInvocation>
#include <KIconLoader>

#include <KDebug>

#include "PkKCommon.h"
#include "PkKStrings.h"
#include "PkKBlockGrantAuth.h"

#include <Context>
#include <ActionButton>
#include <Auth>

using namespace PolkitQt;

namespace PolkitKde
{

PkKAction::PkKAction(QWidget *parent)
        : QWidget(parent),
          m_pfe(0),
          m_updatingEntry(false)
{
    setupUi(this);

    m_fontBold.setBold(true);
    m_fontNotBold.setBold(false);

    blockPB->setIcon(KIcon("object-locked"));
    grantPB->setIcon(KIcon("dialog-ok"));
    revertPB->setIcon(KIcon("document-revert"));
    revokePB->setIcon(KIcon("list-remove"));

    connect(vendorUrlKUL, SIGNAL(leftClickedUrl(const QString &)),
            SLOT(openUrl(const QString &)));

    PolKitResult result;
    QList<PolKitResult> results = resultsOrdered();
    for (int i = 0; i < results.size(); i++) {
        result = results.at(i);
        // Adds all results
        allowAnyCB->addItem(PkKStrings::getPolKitResult(result), result);
        allowInactiveCB->addItem(PkKStrings::getPolKitResult(result), result);
        allowActiveCB->addItem(PkKStrings::getPolKitResult(result), result);
    }
    m_explicitModel = new ExplicitAuthorizationsModel(explicitAuthTV);
    explicitAuthTV->setModel(m_explicitModel);

    // Caller should not unref this object.
    m_authdb = polkit_context_get_authorization_db(Context::instance()->getPolKitContext());

    connect(explicitAuthTV, SIGNAL(clicked(const QModelIndex &)),
            this, SLOT(explicitAuthClicked(const QModelIndex &)));

    // Using the new polkit-qt lib
    m_pkModifyBt = new ActionButton(modifyPB, AUTH_ACTION_MODIFY_DEFAULTS, this);
    m_pkModifyBt->setText(i18n("&Modify"));
    m_pkModifyBt->setIcon(KIcon("object-locked"));
    m_pkModifyBt->setYesIcon(KIcon("security-high"));
    m_pkModifyBt->setNoIcon(KIcon("security-low"));
    m_pkModifyBt->setAuthIcon(KIcon("object-locked-verified"));

    m_pkReadChB = new ActionButton(showChB, AUTH_ACTION_READ, this);
    m_pkReadChB->setText(i18n("&Show authorizations from all users"));
    m_pkReadChB->setIcon(KIcon("object-locked"));
    m_pkReadChB->setYesIcon(KIcon("security-high"));
    m_pkReadChB->setNoIcon(KIcon("security-low"));
    m_pkReadChB->setAuthIcon(KIcon("object-locked-verified"));
    connect(m_pkReadChB, SIGNAL(clicked(QAbstractButton *, bool)),
            m_pkReadChB, SLOT(activate()));
    connect(m_pkReadChB, SIGNAL(activated()),
            this, SLOT(updateExplicitModel()));

    m_pkRevokeAct = new Action(AUTH_ACTION_REVOKE, this);
    connect(m_pkRevokeAct, SIGNAL(dataChanged()),
            this, SLOT(revokeChanged()));
}

PkKAction::~PkKAction()
{
    if (m_pfe != 0) {
        polkit_policy_file_entry_unref(m_pfe);
    }
}

polkit_bool_t fillAuthModel(PolKitAuthorizationDB* authdb, PolKitAuthorization* auth, void* user_data)
{
    Q_UNUSED(authdb)
    ExplicitAuthorizationsModel* model = (ExplicitAuthorizationsModel*)user_data;
    model->addAuth(auth);

    return false;
}

void PkKAction::updateEntryView()
{
    QString actionId = polkit_policy_file_entry_get_id(m_pfe);
    // check to see if we are modifying an entrie
    // if so we should wait
    if (m_updatingEntry && m_oldActionId == actionId) {
        // this is not beautiful but it's the best
        // way to avoid conflictings updates
        return;
    }

    // the pfe is just arriving
    m_pkModifyBt->setMasterEnabled(false);
    revokePB->setEnabled(false);
    m_oldActionId       = actionId;
    //Action description
    QString description = QString::fromUtf8(polkit_policy_file_entry_get_action_description(m_pfe));
    QString iconName    = polkit_policy_file_entry_get_action_icon_name(m_pfe);
    QString vendor      = QString::fromLocal8Bit(polkit_policy_file_entry_get_action_vendor(m_pfe));
    QString vendorUrl   = QString::fromLocal8Bit(polkit_policy_file_entry_get_action_vendor_url(m_pfe));

    descriptionL->setText(description);

    if (vendorUrl.isEmpty()) {
        vendorUrlKUL->setText(vendor);
    } else {
        vendorUrlKUL->setText(vendor);
        vendorUrlKUL->setUrl(vendorUrl);
    }

    QPixmap icon;
    if (!iconName.isEmpty()) {
        icon = KIconLoader::global()->loadIcon(iconName,
                                               KIconLoader::NoGroup, KIconLoader::SizeLarge, KIconLoader::DefaultState, QStringList(), NULL, true);
    }
    if (icon.isNull())
        icon = KIconLoader::global()->loadIcon("preferences-desktop-cryptography",
                                               KIconLoader::NoGroup, KIconLoader::SizeLarge);
    appIconL->setPixmap(icon);

    // IMPLICIT AUTHORIZATIONS
    // defaults are what is set now
    m_defaults              = polkit_policy_file_entry_get_default(m_pfe);
    m_defaultsAllowAny      = polkit_policy_default_get_allow_any(m_defaults);
    m_defaultsAllowInactive = polkit_policy_default_get_allow_inactive(m_defaults);
    m_defaultsAllowActive   = polkit_policy_default_get_allow_active(m_defaults);
    // factory are the values that came from the .policy file
    m_factory               = polkit_policy_file_entry_get_default_factory(m_pfe);
    m_factoryAllowAny       = polkit_policy_default_get_allow_any(m_factory);
    m_factoryAllowInactive  = polkit_policy_default_get_allow_inactive(m_factory);
    m_factoryAllowActive    = polkit_policy_default_get_allow_active(m_factory);

    if (polkit_policy_default_equals(m_defaults, m_factory))
        revertPB->setEnabled(false);
    else
        revertPB->setEnabled(true);

    PolKitResult result;
    QList<PolKitResult> results = resultsOrdered();
    for (int i = 0; i < results.size(); i++) {
        result = results.at(i);
        // Activate or deactivating bold
        if (result != m_factoryAllowAny) {
            allowAnyCB->setItemData(i, m_fontBold, Qt::FontRole);
        } else {
            allowAnyCB->setItemData(i, m_fontNotBold, Qt::FontRole);
        }

        if (result != m_factoryAllowInactive) {
            allowInactiveCB->setItemData(i, m_fontBold, Qt::FontRole);
        } else {
            allowInactiveCB->setItemData(i, m_fontNotBold, Qt::FontRole);
        }

        if (result != m_factoryAllowActive) {
            allowActiveCB->setItemData(i, m_fontBold, Qt::FontRole);
        } else {
            allowActiveCB->setItemData(i, m_fontNotBold, Qt::FontRole);
        }
    }

    allowAnyCB->setCurrentIndex(allowAnyCB->findData(m_defaultsAllowAny));
    allowInactiveCB->setCurrentIndex(allowInactiveCB->findData(m_defaultsAllowInactive));
    allowActiveCB->setCurrentIndex(allowActiveCB->findData(m_defaultsAllowActive));

    // Explicit Authorizations
    updateExplicitModel();
}

void PkKAction::updateExplicitModel()
{
    PolKitError  *err;
    PolKitAction *action;
    PolKitCaller *self;
    DBusError    dbus_error;

    m_explicitModel->clear();
    dbus_error_init(&dbus_error);
    action = polkit_action_new_from_string_representation(polkit_policy_file_entry_get_id(m_pfe));
    self   = polkit_tracker_get_caller_from_pid(Context::instance()->getPolKitTracker(),
                                                QCoreApplication::applicationPid(),
                                                &dbus_error);

    if (m_authdb && action && self != NULL) {
        if (showChB->isChecked()) {
            polkit_authorization_db_foreach_for_action(m_authdb,
                                                       action,
                                                       fillAuthModel,
                                                       m_explicitModel,
                                                       &err);
        } else {
            polkit_authorization_db_foreach_for_action_for_uid(m_authdb,
                                                               action,
                                                               getuid(),
                                                               fillAuthModel,
                                                               m_explicitModel,
                                                               &err);
        }
    }
    explicitAuthTV->resizeColumnToContents(0);
    explicitAuthTV->resizeColumnToContents(1);
    explicitAuthTV->resizeColumnToContents(2);
    explicitAuthTV->resizeColumnToContents(3);
    explicitAuthTV->resizeColumnToContents(4);
}

void PkKAction::revokeChanged()
{
    QModelIndexList modelIndexL = explicitAuthTV->selectionModel()->selectedRows();
    if (modelIndexL.size()) {
        explicitAuthClicked(modelIndexL.at(0));
    }
}

void PkKAction::explicitAuthClicked(const QModelIndex &index)
{
    PolKitAuthorization *auth = m_explicitModel->authEntry(index);
    if (auth) {
        uid_t our_uid;
        uid_t for_uid;
        uid_t pimp_uid;
        bool need_revoke;
        polkit_bool_t is_negative;

        our_uid = getuid();

        for_uid = polkit_authorization_get_uid(auth);

        // we need org.fd.polkit.revoke if:
        // 1) the auth is for another user than us
        // 2) the auth was granted by someone else than us
        need_revoke = false;
        if (for_uid != our_uid) {
            need_revoke = true;
        } else if (polkit_authorization_was_granted_explicitly(auth, &pimp_uid, &is_negative)) {
            if (pimp_uid != our_uid) {
                need_revoke = true;
            }
        }

        if (need_revoke) {
            m_pkRevokeAct->setPolkitAction(AUTH_ACTION_REVOKE);
        } else {
            m_pkRevokeAct->setPolkitAction();
        }
        revokePB->setEnabled(m_pkRevokeAct->isEnabled());
    } else {
        revokePB->setEnabled(false);
    }
}

void PkKAction::on_grantPB_clicked()
{
    PkKBlockGrantAuth *frm = new PkKBlockGrantAuth(m_pfe, false, this);
    frm->show();
}

void PkKAction::on_blockPB_clicked()
{
    PkKBlockGrantAuth *frm = new PkKBlockGrantAuth(m_pfe, true, this);
    frm->exec();
    delete frm;
}

void PkKAction::on_revokePB_clicked()
{
    QModelIndexList modelIndexL = explicitAuthTV->selectionModel()->selectedRows();

    PolKitAuthorization *auth;

    if (modelIndexL.size() && (auth = m_explicitModel->authEntry(modelIndexL.at(0)))) {
        polkit_authorization_ref(auth);
        if (!m_pkRevokeAct->activate(winId())) {
            return;
        }

        PolKitAuthorizationDB *authdb;
        PolKitError *pk_error;

        authdb = polkit_context_get_authorization_db(Context::instance()->getPolKitContext());

        pk_error = NULL;
        if (!polkit_authorization_db_revoke_entry(authdb, auth, &pk_error)) {
            kDebug() << "Error doing revoke: " << polkit_error_get_error_name(pk_error)
                     << ": "                   << polkit_error_get_error_message(pk_error);
            polkit_error_free(pk_error);
        }
        polkit_authorization_unref(auth);
    }
}

void PkKAction::setPolKitPolicyFileEntry(PolKitPolicyFileEntry *pfe)
{
    if (m_pfe) {
        polkit_policy_file_entry_unref(m_pfe);
    }
    m_pfe = pfe;
    polkit_policy_file_entry_ref(m_pfe);
    updateEntryView();
}

void PkKAction::on_allowAnyCB_currentIndexChanged(int index)
{
    if (!m_pfe) {
        return;
    }

    if (allowAnyCB->itemData(index) == m_factoryAllowAny) {
        allowAnyCB->setFont(m_fontNotBold);
        allowAnyL->setFont(m_fontNotBold);
    } else {
        allowAnyCB->setFont(m_fontBold);
        allowAnyL->setFont(m_fontBold);
    }
    checkDefaults();
    checkModified();
}

void PkKAction::on_allowInactiveCB_currentIndexChanged(int index)
{
    if (!m_pfe) {
        return;
    }

    if (allowInactiveCB->itemData(index) == m_factoryAllowInactive) {
        allowInactiveCB->setFont(m_fontNotBold);
        allowInactiveL->setFont(m_fontNotBold);
    } else {
        allowInactiveCB->setFont(m_fontBold);
        allowInactiveL->setFont(m_fontBold);
    }
    checkDefaults();
    checkModified();
}

void PkKAction::on_allowActiveCB_currentIndexChanged(int index)
{
    if (!m_pfe) {
        return;
    }

    if (allowActiveCB->itemData(index) == m_factoryAllowActive) {
        allowActiveCB->setFont(m_fontNotBold);
        allowActiveL->setFont(m_fontNotBold);
    } else {
        allowActiveCB->setFont(m_fontBold);
        allowActiveL->setFont(m_fontBold);
    }
    checkDefaults();
    checkModified();
}

void PkKAction::checkDefaults()
{
    if (polkit_policy_default_equals(m_defaults, m_factory)) {
        // IF they are equal we still need to check if the user has the defaults
        if (allowAnyCB->itemData(allowAnyCB->currentIndex())           == m_factoryAllowAny &&
            allowInactiveCB->itemData(allowInactiveCB->currentIndex()) == m_factoryAllowInactive &&
            allowActiveCB->itemData(allowActiveCB->currentIndex())     == m_factoryAllowActive)
        {
            revertPB->setEnabled(false);
        } else {
            revertPB->setEnabled(true);
        }
    } else {
        revertPB->setEnabled(true);
    }
}

bool PkKAction::checkModified()
{
    // IF they are equal we still need to check if the user has the defaults
    if (allowAnyCB->itemData(allowAnyCB->currentIndex())           == m_defaultsAllowAny &&
        allowInactiveCB->itemData(allowInactiveCB->currentIndex()) == m_defaultsAllowInactive &&
        allowActiveCB->itemData(allowActiveCB->currentIndex())     == m_defaultsAllowActive)
    {
        m_pkModifyBt->setMasterEnabled(false);
    } else {
        m_pkModifyBt->setMasterEnabled(true);
    }
    return modifyPB->isEnabled();
}

void PkKAction::on_modifyPB_clicked()
{
    m_updatingEntry = true;
    PolKitError *pk_error;
    PolKitPolicyDefault *new_defaults;
    // we cache the pfe as this is all asynchronous
    PolKitPolicyFileEntry *pfe = m_pfe;
    polkit_policy_file_entry_ref(pfe);
    kDebug() << "Setting PFE";

    PolKitResult allowAny      = (PolKitResult) allowAnyCB->itemData(allowAnyCB->currentIndex()).toInt();
    PolKitResult allowInactive = (PolKitResult) allowInactiveCB->itemData(allowInactiveCB->currentIndex()).toInt();
    PolKitResult allowActive   = (PolKitResult) allowActiveCB->itemData(allowActiveCB->currentIndex()).toInt();

    // manually call activate as this method is reentrant
    if (m_pkModifyBt->activate()) {
        new_defaults = polkit_policy_default_new();
        polkit_policy_default_set_allow_any(new_defaults, allowAny);
        polkit_policy_default_set_allow_inactive(new_defaults, allowInactive);
        polkit_policy_default_set_allow_active(new_defaults, allowActive);

        pk_error = NULL;
        polkit_policy_file_entry_debug(m_pfe);
        if (!polkit_policy_file_entry_set_default(pfe, new_defaults, &pk_error)) {
            kDebug() << "Error: code=" << polkit_error_get_error_code(pk_error)
            << ": " << polkit_error_get_error_name(pk_error)
            << ": " << polkit_error_get_error_message(pk_error);
            polkit_error_free(pk_error);
        }

        polkit_policy_default_unref(new_defaults);
    }
    m_updatingEntry = false;
    polkit_policy_file_entry_unref(pfe);
}

void PkKAction::on_revertPB_clicked()
{
    PolKitPolicyFileEntry *pfe = m_pfe;
    polkit_policy_file_entry_ref(pfe);
    if (polkit_policy_default_equals(m_defaults, m_factory)) {
        // IF They are equal but we got clicked
        // is because the user changed the values
        // lets select the default ones
        allowAnyCB->setCurrentIndex(allowAnyCB->findData(m_defaultsAllowAny));
        allowInactiveCB->setCurrentIndex(allowInactiveCB->findData(m_defaultsAllowInactive));
        allowActiveCB->setCurrentIndex(allowActiveCB->findData(m_defaultsAllowActive));
    } else if (Auth::computeAndObtainAuth(AUTH_ACTION_MODIFY_DEFAULTS, winId())) {
        // if auth is fine just try to change
        PolKitError *pk_error = NULL;
        if (!polkit_policy_file_entry_set_default(pfe, m_factory, &pk_error)) {
            kDebug() << "Error: code=" << polkit_error_get_error_code(pk_error)
            << ": "           << polkit_error_get_error_name(pk_error)
            << ": "           << polkit_error_get_error_message(pk_error);
            polkit_error_free(pk_error);
        }
    }
    polkit_policy_file_entry_unref(pfe);
}

void PkKAction::openUrl(const QString &url)
{
    KToolInvocation::invokeBrowser(url);
}

QList<PolKitResult> PkKAction::resultsOrdered()
{
    return QList<PolKitResult>()
           << POLKIT_RESULT_NO
           << POLKIT_RESULT_ONLY_VIA_ADMIN_AUTH_ONE_SHOT
           << POLKIT_RESULT_ONLY_VIA_ADMIN_AUTH
           << POLKIT_RESULT_ONLY_VIA_ADMIN_AUTH_KEEP_SESSION
           << POLKIT_RESULT_ONLY_VIA_ADMIN_AUTH_KEEP_ALWAYS
           << POLKIT_RESULT_ONLY_VIA_SELF_AUTH_ONE_SHOT
           << POLKIT_RESULT_ONLY_VIA_SELF_AUTH
           << POLKIT_RESULT_ONLY_VIA_SELF_AUTH_KEEP_SESSION
           << POLKIT_RESULT_ONLY_VIA_SELF_AUTH_KEEP_ALWAYS
           << POLKIT_RESULT_YES;
}


}

#include "PkKAction.moc"
