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

#include "nowplayingengine.h"

#include <config-nowplaying.h>

#include <QStringList>

#include <KDebug>
#include <KLocale>

#include "player.h"
#include "playerfactory.h"
#include "dbuswatcher.h"
#include "pollingwatcher.h"
#include "mpris.h"
#ifdef JUK_FOUND
#include "juk.h"
#endif // JUK_FOUND
#ifdef AMAROK_FOUND
#include "amarok.h"
#endif // AMAROK_FOUND
#ifdef XMMS_FOUND
#include "xmms.h"
#endif // XMMS_FOUND

NowPlayingEngine::NowPlayingEngine(QObject* parent,
                                   const QVariantList& args)
    : Plasma::DataEngine(parent),
      dbusWatcher(new DBusWatcher(this)),
      pollingWatcher(new PollingWatcher(this))
{
    Q_UNUSED(args)

    connect(dbusWatcher, SIGNAL(newPlayer(Player::Ptr)),
            this,        SLOT(addPlayer(Player::Ptr)));
    connect(dbusWatcher, SIGNAL(playerDisappeared(Player::Ptr)),
            this,        SLOT(removePlayer(Player::Ptr)));
    connect(pollingWatcher, SIGNAL(newPlayer(Player::Ptr)),
            this,        SLOT(addPlayer(Player::Ptr)));
    connect(pollingWatcher, SIGNAL(playerDisappeared(Player::Ptr)),
            this,        SLOT(removePlayer(Player::Ptr)));

    dbusWatcher->addFactory(new MprisFactory(dbusWatcher));
#ifdef JUK_FOUND
    dbusWatcher->addFactory(new JukFactory(dbusWatcher));
#endif
#ifdef AMAROK_FOUND
    dbusWatcher->addFactory(new AmarokFactory(dbusWatcher));
#endif
#ifdef XMMS_FOUND
    pollingWatcher->addFactory(new XmmsFactory(pollingWatcher));
#endif
}

bool NowPlayingEngine::sourceRequestEvent(const QString &source)
{
    kDebug() << "Source" << source << "was requested";
    QString lowerSource = source.toLower();
    if (lowerSource == "help") {
        setData(source, "Use 'players' to get a list of players.\n"
                        "Use 'properties' to get a list of all properties that may be returned."
                        );
        return true;
    } else if (lowerSource == "properties") {
        setData(source, "State",           "QString - playing|paused|stopped");
        setData(source, "Artist",          "QString - the artist metadata for the\n"
                                           "          current track, if available");
        setData(source, "Album",           "QString - the album metadata for the\n"
                                           "          current track, if available");
        setData(source, "Title",           "QString - the title metadata for the\n"
                                           "          current track, if available");
        setData(source, "Track number",    "int     - the album/collection track number\n"
                                           "          (eg: on a CD) if known, 0 otherwise");
        setData(source, "Comment",         "QString - the comment metadata for the\n"
                                           "          current track, if available");
        setData(source, "Genre",           "QString - the comment metadata for the\n"
                                           "          current track, if available");
        setData(source, "Length",          "int     - the length of the current track\n"
                                           "          in seconds, 0 if unknown");
        setData(source, "Position",        "int     - the position of the current track\n"
                                           "          in seconds, 0 if unknown");
        setData(source, "Volume",          "float   - the volume, given as a float\n"
                                           "          between 0 and 1, or -1 if unknown");
        setData(source, "Artwork",         "QPixmap - the album artwork, if available");
        return true;
    } else if (lowerSource == "players") {
        setData(source, sources());
        return true;
    } else if (players.contains(source)) {
        return updateSourceEvent(source);
    } else {
        kWarning() << "Player" << source << "not found";
    }

    return false;
}

QStringList NowPlayingEngine::sources() const
{
    return players.keys();
}

bool NowPlayingEngine::updateSourceEvent(const QString& source)
{
    QString lowerSource = source.toLower();
    if (lowerSource == "help" || lowerSource == "properties") {
        // help text doesn't change
        return true;
    }

    if (!players.contains(source)) {
        kDebug() << "Can't find source" << source;
         removeAllData(source);
        return false;
    }
    Player::Ptr player = players[source];
    Q_ASSERT(player);

    if (!player->isRunning()) {
        kDebug() << source << "isn't running";
        removePlayer(player);
        return false;
    }

    switch(player->state()) {
        case Player::Playing:
            setData(source, "State", "playing");
            break;
        case Player::Paused:
            setData(source, "State", "paused");
            break;
        case Player::Stopped:
            setData(source, "State", "stopped");
            break;
    }

    setData(source, "Artist", player->artist());
    setData(source, "Album", player->album());
    setData(source, "Title", player->title());
    setData(source, "Track number", player->trackNumber());
    setData(source, "Comment", player->comment());
    setData(source, "Genre", player->genre());
    setData(source, "Length", player->length());
    setData(source, "Position", player->position());
    setData(source, "Volume", player->volume());
    setData(source, "Artwork", player->artwork());

    return true;
}

void NowPlayingEngine::addPlayer(Player::Ptr player)
{
    kDebug() << "Adding player" << player->name();
    players.insert(player->name(), player);
    emit sourceAdded(player->name());
}

void NowPlayingEngine::removePlayer(Player::Ptr player)
{
    kDebug() << "Player" << player->name() << "disappeared";
    if (players.contains(player->name())) {
        Q_ASSERT(player == players[player->name()]);

        players.remove(player->name());
        removeSource(player->name());
    } else {
        kDebug() << "We didn't know about player" << player->name();
    }
}

#include "nowplayingengine.moc"
