/*
 * Copyright 2008  Alex Merry <alex.merry@kdemail.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef PLAYERFACTORY_H
#define PLAYERFACTORY_H

#include <QObject>
#include <QVariantList>

#include "player.h"

/**
 * A player factory that creates players.
 */
class PlayerFactory : public QObject
{
    Q_OBJECT
public:
    PlayerFactory(QObject* parent = 0);
    /**
     * Create a player.
     *
     * Returns 0 if a valid player could not be created.
     */
    virtual Player::Ptr create(const QVariantList& args = QVariantList()) = 0;
};

/**
 * A player factory that is polled.
 */
class PollingPlayerFactory : public PlayerFactory
{
    Q_OBJECT
public:
    PollingPlayerFactory(QObject* parent = 0);
    /**
     * Whether create(args) will return a player
     *
     * Note that just because this returns true, it
     * should not be assumed that create(args) will not
     * return 0.  However, if this returns false,
     * it can be assumed that create(args) will always
     * return 0.
     */
    virtual bool exists(const QVariantList& args = QVariantList()) = 0;
};

/**
 * A player factory that creates players based on a
 * DBus service.
 */
class DBusPlayerFactory : public PlayerFactory
{
    Q_OBJECT
public:
    DBusPlayerFactory(QObject* parent = 0);
    /**
     * Whether the given dbus service name is a service
     * for this player
     */
    virtual bool matches(const QString& serviceName) = 0;
    // don't let the QString overload hide this
    virtual Player::Ptr create(const QVariantList& args = QVariantList()) = 0;
    Player::Ptr create(const QString& serviceName);
};

#endif // PLAYERFACTORY_H
