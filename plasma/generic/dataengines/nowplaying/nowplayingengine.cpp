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

#include "playerinterface/player.h"
#include "playerinterface/playerfactory.h"
#include "playerinterface/dbuswatcher.h"
#include "playerinterface/pollingwatcher.h"
#include "playerinterface/mpris/mpris.h"
#include "playerinterface/mpris2/mpris2.h"
#include "playerinterface/juk.h"
#ifdef XMMS_FOUND
#include "playerinterface/xmms.h"
#endif // XMMS_FOUND

#include "playercontrol.h"
#include "playercontainer.h"

NowPlayingEngine::NowPlayingEngine(QObject* parent,
                                   const QVariantList& args)
    : Plasma::DataEngine(parent),
      dbusWatcher(new DBusWatcher(this)),
      pollingWatcher(0)
{
    Q_UNUSED(args)

    setData("players", QStringList());

    kWarning() << "The \"nowplaying\" engine is deprecated; use the \"mpris2\" engine instead";

    connect(dbusWatcher, SIGNAL(newPlayer(Player::Ptr)),
            this,        SLOT(addPlayer(Player::Ptr)));
    connect(dbusWatcher, SIGNAL(playerDisappeared(Player::Ptr)),
            this,        SLOT(removePlayer(Player::Ptr)));

    dbusWatcher->addFactory(new Mpris2Factory(dbusWatcher));
    dbusWatcher->addFactory(new MprisFactory(dbusWatcher));
    dbusWatcher->addFactory(new JukFactory(dbusWatcher));

#ifdef XMMS_FOUND
    pollingWatcher = new PollingWatcher(this);
    connect(pollingWatcher, SIGNAL(newPlayer(Player::Ptr)),
            this,        SLOT(addPlayer(Player::Ptr)));
    connect(pollingWatcher, SIGNAL(playerDisappeared(Player::Ptr)),
            this,        SLOT(removePlayer(Player::Ptr)));
    pollingWatcher->addFactory(new XmmsFactory(pollingWatcher));
#endif
}

Plasma::Service* NowPlayingEngine::serviceForSource(const QString& source)
{
    PlayerContainer* container = qobject_cast<PlayerContainer*>(containerForSource(source));
    if (container) {
        return container->service(this);
    } else {
        return DataEngine::serviceForSource(source);
    }
}


bool NowPlayingEngine::sourceRequestEvent(const QString& source)
{
    kDebug() << "Source" << source << "was requested";
    if (source == "help") {
        setData(source, "Use 'players' to get a list of players.\n"
                        "Use 'properties' to get a list of all properties that may be returned."
                        );
        return true;
    } else if (source == "properties") {
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
        setData(source, "Lyrics",          "QString - song lyrics, if available");
        return true;
    }

    return false;
}

void NowPlayingEngine::addPlayer(Player::Ptr player)
{
    kDebug() << "Adding player" << player->name();
    Plasma::DataContainer *container = containerForSource("players");
    QStringList players;
    if (container) {
        players = container->data()["players"].toStringList();
    }

    players << player->name();
    setData("players", players);

    addSource(new PlayerContainer(player, this));
}

void NowPlayingEngine::removePlayer(Player::Ptr player)
{
    kDebug() << "Player" << player->name() << "disappeared";
    Plasma::DataContainer *container = containerForSource("players");
    if (container) {
        QStringList players = container->data()["players"].toStringList();
        players.removeAll(player->name());
        setData("players", players);
    }

    removeSource(player->name());
}

#include "nowplayingengine.moc"
