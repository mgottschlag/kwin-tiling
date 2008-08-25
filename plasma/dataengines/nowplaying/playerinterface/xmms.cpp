/*
 *   Copyright 2007 Alex Merry <alex.merry@kdemail.net>
 *
 *   Based on Xmms support in the Kopete Now Listening plugin
 *   Copyright 2002 by Will Stephenson <will@stevello.free-online.co.uk>
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

#include "xmms.h"
#include "xmms_p.h"

#include <kdebug.h>

#include <xmmsctrl.h>

XmmsFactory::XmmsFactory(QObject* parent)
    : PollingPlayerFactory(parent)
{
}

Player::Ptr XmmsFactory::create(const QVariantList& args)
{
    int session = 0;
    if (!args.isEmpty() && args.first().canConvert<int>()) {
        session = args.first().toInt();
        if (session < 0) {
            return Player::Ptr(0);
        }
    }
    if (xmms_remote_is_running(session)) {
        Xmms* player = new Xmms(session, this);
        kDebug() << "Creating a player for XMMS session" << session;
        return Player::Ptr(player);
    }
    return Player::Ptr(0);
}

bool XmmsFactory::exists(const QVariantList& args)
{
    int session = 0;
    if (!args.isEmpty() && args.first().canConvert<int>()) {
        session = args.first().toInt();
    }
    return (session >= 0) && xmms_remote_is_running(session);
}





Xmms::Xmms(int session, PlayerFactory* factory)
    : Player(factory),
      m_session(session)
{
    if (m_session == 0) {
        setName("XMMS");
    } else {
        setName("XMMS" + QString::number(m_session));
    }
}

Xmms::~Xmms()
{
}

bool Xmms::isRunning()
{
    return xmms_remote_is_running(m_session);
}

Player::State Xmms::state()
{
    if (xmms_remote_is_paused(m_session)) {
        return Paused;
    } else if (xmms_remote_is_playing(m_session)) {
        return Playing;
    }
    return Stopped;
}

QString Xmms::artist()
{
    // let's hope no-one changes the default title string
    QString track = xmms_remote_get_playlist_title(m_session, xmms_remote_get_playlist_pos(0));
    return track.section(" - ", 0, 0);
}

QString Xmms::album()
{
    return QString();
}

QString Xmms::title()
{
    // let's hope no-one changes the default title string
    QString track = xmms_remote_get_playlist_title(m_session, xmms_remote_get_playlist_pos(0));
    return track.section(" - ", -1, -1);
}

int Xmms::trackNumber()
{
    // we can get the playlist pos, but that's not what we mean by "trackNumber"
    return 0;
}

QString Xmms::comment()
{
    return QString();
}

QString Xmms::genre()
{
    return QString();
}

int Xmms::length()
{
    return xmms_remote_get_playlist_time(m_session, xmms_remote_get_playlist_pos(0));
}

int Xmms::position()
{
    return xmms_remote_get_output_time(m_session);
}

float Xmms::volume()
{
    return xmms_remote_get_main_volume(m_session);
}

void Xmms::play()
{
    xmms_remote_play(m_session);
}

void Xmms::pause()
{
    xmms_remote_pause(m_session);
}

void Xmms::stop()
{
    xmms_remote_stop(m_session);
}

void Xmms::previous()
{
    xmms_remote_playlist_prev(m_session);
}

void Xmms::next()
{
    xmms_remote_playlist_next(m_session);
}

void Xmms::setVolume(qreal volume)
{
    xmms_remote_set_main_volume(m_session, volume);
}

void Xmms::seek(int time)
{
    xmms_remote_jump_to_time(m_session, time);
}


#include "xmms.moc"
