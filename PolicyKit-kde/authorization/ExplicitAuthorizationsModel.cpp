/*  This file is part of the KDE project
Copyright (C) 2008 Trever Fischer <wm161@wm161.net>
Copyright (C) 2008 Daniel Nicoletti <dantti85-pk@yahoo.com.br>

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

#include "ExplicitAuthorizationsModel.h"

#include <KDebug>
#include <KGlobal>
#include <KLocale>
#include <KUser>
#include <KIcon>
#include <KIconLoader>
#include <QPainter>

#include <QDateTime>

#include <Context>

using namespace PolkitQt;

namespace PolkitKde
{

ExplicitAuthorizationsModel::ExplicitAuthorizationsModel(QObject* parent)
        : QStandardItemModel(parent)
{
    setHeaders();
}

ExplicitAuthorizationsModel::~ExplicitAuthorizationsModel()
{
}

void
ExplicitAuthorizationsModel::setHeaders()
{
    QStringList headers;
    headers << "User" << "Scope" << "Obtained" << "How" << "Constraints";
    setHorizontalHeaderLabels(headers);
}

void
ExplicitAuthorizationsModel::clear()
{
    QList<QStandardItem *> firstItems;
    firstItems = QStandardItemModel::takeColumn(0);
    foreach(QStandardItem *item, firstItems) {
        PolKitAuthorization *auth;
        auth = item->data(PolkitAuthRole).value<PolKitAuthorization*>();
        polkit_authorization_unref(auth);
        // manually delete then as
        // QStandardItemModel won't delete them anymore
        delete item;
    }
    QStandardItemModel::clear();
    setHeaders();
}

void
ExplicitAuthorizationsModel::addAuth(PolKitAuthorization* auth)
{
    QList<QStandardItem*> row;
    uid_t uid = polkit_authorization_get_uid(auth);
    DBusError dbus_error;

    dbus_error_init(&dbus_error);
    // Here we check to see if the authorization is relevant
    // which means that if a process is terminated the
    // authorization is not relevant
    if (!polkit_tracker_is_authorization_relevant(Context::instance()->getPolKitTracker(),
                                                  auth,
                                                  &dbus_error)) {
        if (dbus_error_is_set(&dbus_error)) {
            kDebug() << "Cannot determine if authorization is relevant: "
            << dbus_error.name << ": " << dbus_error.message;
            dbus_error_free(&dbus_error);
        } else {
            return;
        }
    }

    KUser user(uid);
    QString forUser;
    if (user.isValid()) {
        if (!user.property(KUser::FullName).toString().isEmpty()) {
            forUser = user.property(KUser::FullName).toString() + " (" + user.loginName() + ')';
        } else {
            forUser = user.loginName();
        }
    } else {
        forUser = QString("uid %1").arg(uid);
    }

    //headers << "Entity" << "Scope" << "Obtained" << "How" << "Constraints";
    QStandardItem* first = new QStandardItem(forUser); //User
    first->setData(QVariant::fromValue(auth), PolkitAuthRole);
    polkit_authorization_ref(auth);
    row << first;

    QString scopeStr;
    int pid;
    polkit_uint64_t start;
    char exe[512];
    switch (polkit_authorization_get_scope(auth)) {
    case POLKIT_AUTHORIZATION_SCOPE_PROCESS_ONE_SHOT:
        polkit_authorization_scope_process_get_pid(auth, &pid, &start);
        exe[sizeof(exe) - 1] = '\0';
        polkit_sysdeps_get_exe_for_pid(pid, exe, sizeof(exe) - 1);
        scopeStr = i18nc("Authorization scope; PID is Process ID", "Single shot PID: %1 (%2)", pid, exe);
        break;
    case POLKIT_AUTHORIZATION_SCOPE_PROCESS:
        polkit_authorization_scope_process_get_pid(auth, &pid, &start);
        exe[sizeof(exe) - 1] = '\0';
        polkit_sysdeps_get_exe_for_pid(pid, exe, sizeof(exe) - 1);
        scopeStr = i18nc("Authorization scope; PID is Process ID", "PID: %1 (%2)", pid, exe);
        break;
    case POLKIT_AUTHORIZATION_SCOPE_SESSION:
        scopeStr = i18nc("Authorization scope", "This Session");
        break;
    case POLKIT_AUTHORIZATION_SCOPE_ALWAYS:
        scopeStr = i18nc("Authorization scope", "Always");
        break;
    }
    row << new QStandardItem(scopeStr); //Scope

    row << new QStandardItem(KGlobal::locale()->formatDateTime(QDateTime::fromTime_t(polkit_authorization_get_time_of_grant(auth)))); //Obtained time

    QString howStr;
    polkit_bool_t is_negative;
    if (polkit_authorization_was_granted_via_defaults(auth, &uid)) {
        KUser user(uid);
        if (user.isValid()) {
            howStr = i18n("Auth as %1 (uid %2)", user.loginName(), uid);
        } else {
            howStr = i18n("Auth as uid %1", uid);
        }
    } else if (polkit_authorization_was_granted_explicitly(auth, &uid, &is_negative)) {
        KUser user(uid);
        if (is_negative) {
            if (user.isValid()) {
                howStr = i18n("Blocked by %1 (uid %2)", user.loginName(), uid);
            } else {
                howStr = i18n("Blocked by uid %1", uid);
            }
        } else {
            if (user.isValid()) {
                howStr = i18n("Granted by %1 (uid %2)", user.loginName(), uid);
            } else {
                howStr = i18n("Granted by uid %1", uid);
            }
        }
    }
    row << new QStandardItem(howStr);//How

    // loads the user icon
    QPixmap icon;
    if (!user.faceIconPath().isEmpty()) {
        icon = KIconLoader::global()->loadIcon(user.faceIconPath(),
                                               KIconLoader::NoGroup,
                                               KIconLoader::SizeHuge,
                                               KIconLoader::DefaultState);
    }
    if (icon.isNull()) {
        icon = KIconLoader::global()->loadIcon("user-identity",
                                               KIconLoader::NoGroup,
                                               KIconLoader::SizeHuge,
                                               KIconLoader::DefaultState);
    }
    // create a paiter to paint the action icon over the key icon
    QPainter painter(&icon);
    if (is_negative) {
        const int iconSize = icon.size().width();
        // the the emblem icon to size 32
        int overlaySize = 32;
        // try to load the action icon
        const QPixmap pixmap = KIconLoader::global()->loadIcon("process-stop",
                                                               KIconLoader::NoGroup,
                                                               overlaySize,
                                                               KIconLoader::DefaultState,
                                                               QStringList(),
                                                               0,
                                                               true);
        QPoint startPoint;
        // bottom right corner
        startPoint = QPoint(iconSize - overlaySize - 2,
                            iconSize - overlaySize - 2);
        painter.drawPixmap(startPoint, pixmap);
    }
    first->setIcon(icon);

    //Constraints
    m_constraintList.clear();
    polkit_authorization_constraints_foreach(auth, buildConstraintList, &m_constraintList);
    if (m_constraintList.size()) {
        row << new QStandardItem(m_constraintList.join(", "));
    } else {
        row << new QStandardItem(i18nc("No auth found", "None"));
    }

    appendRow(row);
}

polkit_bool_t
ExplicitAuthorizationsModel::buildConstraintList(PolKitAuthorization */*auth*/, PolKitAuthorizationConstraint *authc, void *user_data)
{
    QStringList *constraintList = (QStringList *) user_data;

    switch (polkit_authorization_constraint_type(authc)) {
    case POLKIT_AUTHORIZATION_CONSTRAINT_TYPE_REQUIRE_LOCAL:
        constraintList->append(i18n("Must be on console"));
        break;
    case POLKIT_AUTHORIZATION_CONSTRAINT_TYPE_REQUIRE_ACTIVE:
        constraintList->append(i18n("Must be in active session"));
        break;
    case POLKIT_AUTHORIZATION_CONSTRAINT_TYPE_REQUIRE_EXE:
        constraintList->append(i18n("Must be program %1", polkit_authorization_constraint_get_exe(authc)));
        break;
    case POLKIT_AUTHORIZATION_CONSTRAINT_TYPE_REQUIRE_SELINUX_CONTEXT:
        constraintList->append(i18n("Must be SELinux Context %1", polkit_authorization_constraint_get_selinux_context(authc)));
        break;
    default:
        char buf[128];
        buf[sizeof(buf) - 1] = '\0';
        polkit_authorization_constraint_to_string(authc, buf, sizeof(buf) - 1);
        constraintList->append(buf);
        break;
    }

    return false;
}

PolKitAuthorization*
ExplicitAuthorizationsModel::authEntry(const QModelIndex &index) const
{
    return data(index.sibling(index.row(), 0), PolkitAuthRole).value<PolKitAuthorization*>();
}

}

#include "ExplicitAuthorizationsModel.moc"
