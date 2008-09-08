/*
 * KFontInst - KDE Font Installer
 *
 * Copyright 2003-2008 Craig Drummond <craig@kde.org>
 *
 * ----
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef __POLICYKIT_AUTHENTICATOR_H__
#define __POLICYKIT_AUTHENTICATOR_H__

#include <QtCore/QObject>

class QDBusMessage;
class QDBusError;
class QWidget;
class PolicyKitAuthenticatorPrivate;

class PolicyKitAuthenticator : public QObject
{
    Q_OBJECT

    public:

    static PolicyKitAuthenticator * instance();

    PolicyKitAuthenticator();
    virtual ~PolicyKitAuthenticator();

    bool authenticate(const QString &action, QWidget *widet, bool gui);
    bool authenticate(const QString &action, uint winId, uint pid, bool gui);

    Q_SIGNALS:

    void quitLoop();

    private Q_SLOTS:

    void reply(const QDBusMessage &msg);
    void error(const QDBusError &er, const QDBusMessage &msg);

    private:

    bool isAuthenticated(const QDBusMessage &msg);

    PolicyKitAuthenticator(const PolicyKitAuthenticator &o);

    private:

    PolicyKitAuthenticatorPrivate * const d_ptr;
    Q_DECLARE_PRIVATE(PolicyKitAuthenticator)
};

#endif
