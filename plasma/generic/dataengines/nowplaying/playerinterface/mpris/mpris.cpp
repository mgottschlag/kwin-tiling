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

#include "mpris.h"
#include "mpris_p.h"
#include <mprisplayer.h>

#include <QtDBus>
#include <QFile>

#include <kdebug.h>
#include <KIO/NetAccess>



MprisFactory::MprisFactory(QObject* parent)
    : DBusPlayerFactory(parent)
{
    setObjectName( QLatin1String("MprisFactory" ));
    qDBusRegisterMetaType<MprisDBusVersion>();
    qDBusRegisterMetaType<MprisDBusStatus>();
}

Player::Ptr MprisFactory::create(const QVariantList& args)
{
    if (args.isEmpty()) {
        return Player::Ptr(0);
    }

    QString dbusName(args.first().toString());
    if (dbusName.isEmpty()) {
        return Player::Ptr(0);
    }

    Mpris* player = new Mpris(dbusName, this);
    if (!player->isRunning()) {
        delete player;
        player = 0;
    }

    return Player::Ptr(player);
}

bool MprisFactory::matches(const QString& serviceName)
{
    return serviceName.startsWith(QLatin1String("org.mpris")) &&
           !serviceName.startsWith(QLatin1String("org.mpris.MediaPlayer2"));
}



Mpris::Mpris(const QString& name, PlayerFactory* factory)
    : QObject(),
      Player(factory),
      m_player(0),
      m_playerName(name),
      m_artworkLoaded(false)
{
    if (!name.startsWith(QLatin1String("org.mpris"))) {
        m_playerName = "org.mpris." + name;
    }
    setName(m_playerName);
    setup();
}

Mpris::~Mpris()
{
    delete m_player;
}

void Mpris::setup()
{
    delete m_player;
    m_player = new MprisPlayer(
            m_playerName,
            "/Player",
            QDBusConnection::sessionBus());

    m_metadata.clear();
    m_state = Stopped;
    m_caps = NO_CAPS;

    if (m_player->isValid()) {
        connect(m_player, SIGNAL(CapsChange(int)),
                this,   SLOT(capsChanged(int)));
        connect(m_player, SIGNAL(TrackChange(QVariantMap)),
                this,   SLOT(trackChanged(QVariantMap)));
        connect(m_player, SIGNAL(StatusChange(MprisDBusStatus)),
                this,   SLOT(stateChanged(MprisDBusStatus)));

        QDBusReply<int> caps = m_player->GetCaps();
        if (caps.isValid()) {
            capsChanged(caps);
        }

        QDBusReply<QVariantMap> metadata = m_player->GetMetadata();
        if (metadata.isValid()) {
            trackChanged(metadata);
        }

        QDBusReply<MprisDBusStatus> status = m_player->GetStatus();
        if (status.isValid()) {
            stateChanged(status);
        }
    }
}

bool Mpris::isRunning()
{
    if (!m_player->isValid()) {
        setup();
    }

    return m_player->isValid();
}

Player::State Mpris::state()
{
    return m_state;
}

QString Mpris::artist()
{
    if (m_metadata.contains("artist")) {
        return m_metadata["artist"].toString();
    }
    return QString();
}

QString Mpris::album()
{
    if (m_metadata.contains("album")) {
        return m_metadata["album"].toString();
    }
    return QString();
}

QString Mpris::title()
{
    if (m_metadata.contains("title")) {
        return m_metadata["title"].toString();
    }
    return QString();
}

int Mpris::trackNumber()
{
    QVariant track;
    if (m_metadata.contains("trackNumber")) {
        track = m_metadata["trackNumber"];
    } else if (m_metadata.contains("tracknumber")) {
        track = m_metadata["tracknumber"];
    }
    if (track.isValid()) {
        if (track.canConvert(QVariant::Int)) {
            return track.toInt();
        } else {
            QString text = track.toString();
            int pos = text.indexOf('/');
            if (pos >= 0) {
                text.truncate(pos);
            }
            return text.toInt();
        }
    }
    return 0;
}

QString Mpris::comment()
{
    if (m_metadata.contains("comment")) {
        return m_metadata["comment"].toString();
    }
    return QString();
}

QString Mpris::genre()
{
    if (m_metadata.contains("genre")) {
        return m_metadata["genre"].toString();
    }
    return QString();
}

QString Mpris::lyrics()
{
    if (m_metadata.contains("lyrics")) {
        return m_metadata["lyrics"].toString();
    }
    return QString();
}

int Mpris::length()
{
    if (m_metadata.contains("time")) {
        return m_metadata["time"].toInt();
    } else if (m_metadata.contains("length")) {
        // stupid audacious...
        return m_metadata["length"].toInt()/1000;
    }
    return 0;
}

int Mpris::position()
{
    if (m_player->isValid()) {
        QDBusReply<int> positionMs = m_player->PositionGet();
        if (positionMs.isValid()) {
            return positionMs / 1000;
        }
    }
    return 0;
}

float Mpris::volume()
{
    if (m_player->isValid()) {
        QDBusReply<int> volumePercent = m_player->VolumeGet();
        if (volumePercent.isValid())
            return ((float)volumePercent) / 100;
    }
    return 0;
}

QPixmap Mpris::artwork()
{
    if (m_artworkLoaded) {
        return m_artwork;
    }

    m_artwork = QPixmap();
    const QString arturl = m_metadata["arturl"].toString();
    if (!arturl.isEmpty()) {
        if (!m_artfiles.contains(arturl) ||
            (!m_artfiles[arturl].isEmpty() && !QFile::exists(m_artfiles[arturl]))) {
            QString artfile;
            if (!KIO::NetAccess::download(arturl, artfile, 0)) {
                kWarning() << KIO::NetAccess::lastErrorString();
                artfile.clear();
            }

            m_artfiles[arturl] = artfile;
        }

        const QString url = m_artfiles[arturl];
        if (!url.isEmpty()) {
            m_artwork = QPixmap(url);
        }
    }

    m_artworkLoaded = true;
    return m_artwork;
}

bool Mpris::canPlay()
{
    return m_caps & CAN_PLAY;
}

void Mpris::play()
{
    if (m_player->isValid()) {
        m_player->Play();
    }
}

bool Mpris::canPause()
{
    return m_caps & CAN_PAUSE;
}

void Mpris::pause()
{
    if (m_player->isValid()) {
        m_player->Pause();
    }
}

bool Mpris::canStop()
{
    return m_state != Stopped;
}

void Mpris::stop()
{
    if (m_player->isValid()) {
        m_player->Stop();
    }
}

bool Mpris::canGoPrevious()
{
    return m_caps & CAN_GO_PREV;
}

void Mpris::previous()
{
    if (m_player->isValid()) {
        m_player->Prev();
    }
}

bool Mpris::canGoNext()
{
    return m_caps & CAN_GO_NEXT;
}

void Mpris::next()
{
    if (m_player->isValid()) {
        m_player->Next();
    }
}

bool Mpris::canSetVolume()
{
    return true;
}

void Mpris::setVolume(qreal volume)
{
    if (m_player->isValid()) {
        m_player->VolumeSet((int)(volume * 100));
    }
}

bool Mpris::canSeek()
{
    return m_caps & CAN_SEEK;
}

void Mpris::seek(int time)
{
    if (m_player->isValid()) {
        // m_player->seek() wants milliseconds
        m_player->PositionSet(time * 1000);
    }
}

// SLOTS

void Mpris::trackChanged(const QVariantMap& metadata)
{
    //kDebug() << m_playerName << "metadata:" << metadata;
    const QString oldArt = m_metadata.value("arturl").toString();
    m_metadata = metadata;
    if (m_artworkLoaded) {
        m_artworkLoaded = oldArt == m_metadata.value("arturl");
    }
}

void Mpris::stateChanged(MprisDBusStatus state)
{
    kDebug() << m_playerName << "state:" << state.play;
    switch (state.play) {
        case MprisDBusStatus::Playing:
            m_state = Playing;
            break;
        case MprisDBusStatus::Paused:
            m_state = Paused;
            break;
        case MprisDBusStatus::Stopped:
            m_state = Stopped;
            break;
        default:
            kDebug() << m_playerName << "unexpected state" << state.play;
    }
}

void Mpris::capsChanged(int caps)
{
    kDebug() << m_playerName << "capabilities:" << caps;
    m_caps = (DBusCaps)caps;
    if (!(caps & CAN_PROVIDE_METADATA)) {
        m_metadata.clear();
    }
}


#include "mpris.moc"
#include "mpris_p.moc"
