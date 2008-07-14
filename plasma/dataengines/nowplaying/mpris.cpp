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
#include <mpris_interface.h>

#include <QtDBus>
#include <QFile>

#include <kdebug.h>
#include <KIO/NetAccess>



MprisFactory::MprisFactory(QObject* parent)
    : DBusPlayerFactory(parent)
{
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
    return serviceName.startsWith("org.mpris");
}



Mpris::Mpris(const QString& name, PlayerFactory* factory)
    : QObject(),
      Player(factory),
      m_player(0),
      m_playerName(name)
{
    if (!name.startsWith("org.mpris")) {
        m_playerName = "org.mpris." + name;
    }
    setName(m_playerName);
    setup();
}

Mpris::~Mpris()
{
    delete m_player;
}

void Mpris::setup() {
    delete m_player;
    m_player = new OrgFreedesktopMediaPlayerInterface(
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
        connect(m_player, SIGNAL(StatusChange(int)),
                this,   SLOT(stateChanged(int)));

        capsChanged(m_player->GetCaps());
        trackChanged(m_player->GetMetadata());
        stateChanged(m_player->GetStatus());
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
    if (m_metadata.contains("trackNumber")) {
        QString track = m_metadata["trackNumber"].toString();
        int pos = track.indexOf('/');
        if (pos >= 0) {
            track.truncate(pos);
        }
        return track.toInt();
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
        return m_player->PositionGet() / 1000;
    }
    return 0;
}

float Mpris::volume()
{
    if (m_player->isValid()) {
        return ((float)m_player->VolumeGet()) / 100;
    }
    return 0;
}

QPixmap Mpris::artwork()
{
    if (m_metadata.contains("arturl")) {
        QString arturl = m_metadata["arturl"].toString();
        if (!arturl.isEmpty()) {
            if (!m_artfiles.contains(arturl) ||
                !QFile::exists(m_artfiles[arturl])) {
                QString artfile;
                if (!KIO::NetAccess::download(arturl, artfile, 0)) {
                    kWarning() << KIO::NetAccess::lastErrorString();
                    return QPixmap();
                }
                m_artfiles[arturl] = artfile;
            }
            return QPixmap(m_artfiles[arturl]);
        }
    }
    return QPixmap();
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
    kDebug() << m_playerName << "metadata:" << metadata;
    m_metadata = metadata;
}

void Mpris::stateChanged(int state)
{
    kDebug() << m_playerName << "state:" << state;
    switch (state) {
        case 0:
            m_state = Playing;
            break;
        case 1:
            m_state = Paused;
            break;
        case 2:
            m_state = Stopped;
            break;
        default:
            kDebug() << m_playerName << "unexpected state" << state;
    }
}

void Mpris::capsChanged(int caps)
{
    kDebug() << m_playerName << "capabilities:" << caps;
    m_caps = static_cast<Caps>(caps);
    if (!caps & CAN_PROVIDE_METADATA) {
        m_metadata.clear();
    }
}


#include "mpris.moc"
#include "mpris_p.moc"
