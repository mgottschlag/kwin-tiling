/*
 *   Copyright 2007 Alex Merry <alex.merry@kdemail.net>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2 as
 *   published by the Free Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef NOWPLAYINGENGINE_H
#define NOWPLAYINGENGINE_H

#include <plasma/dataengine.h>

#include <QMap>
#include <QString>

#include "player.h"

class DBusWatcher;
class PollingWatcher;
class PlayerControl;

/**
 * The Now Playing data engine.
 *
 * Use plasmaengineexplorer and request the "help" source for more info.
 */
class NowPlayingEngine : public Plasma::DataEngine
{
    Q_OBJECT

public:
    NowPlayingEngine(QObject* parent, const QVariantList& args);
    QStringList sources() const;
    Plasma::Service* serviceForSource(const QString& source);

protected:
    bool sourceRequestEvent(const QString &source);
    bool updateSourceEvent(const QString& source);

private slots:
    void addPlayer(Player::Ptr player);
    void removePlayer(Player::Ptr player);

private:
    QMap<QString, Player::Ptr> players;
    QMap<QString, PlayerControl*> controllers;
    DBusWatcher* dbusWatcher;
    PollingWatcher* pollingWatcher;
};

K_EXPORT_PLASMA_DATAENGINE(nowplaying, NowPlayingEngine)

#endif // NOWPLAYINGENGINE_H
