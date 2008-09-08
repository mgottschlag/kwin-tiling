#ifndef __POLICYKIT_MECHANISM_H__
#define __POLICYKIT_MECHANISM_H__

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

#include <QtCore/QObject>
#include <polkit/polkit.h>

class PolicyKitMechanismPrivate;

class PolicyKitMechanism : public QObject
{
    Q_OBJECT

    public:

    static PolicyKitMechanism * instance();

    PolicyKitMechanism();
    virtual ~PolicyKitMechanism();

    PolKitResult canDoAction(const QString &action, unsigned int pid);
    PolKitResult canDoAction(const QString &action, const QString &dbusName);
    void removeAction(const QString &action);

    /* @internal */
    void addWatch(int fd);
    /* @internal */
    void removeWatch(int fd);

    private Q_SLOTS:

    void contextWatchActivated(int fd);

    private:

    PolicyKitMechanism(const PolicyKitMechanism &o);

    PolKitAction * getAction(const QString &action);

    private:

    PolicyKitMechanismPrivate * const d_ptr;
    Q_DECLARE_PRIVATE(PolicyKitMechanism)
};

#endif
