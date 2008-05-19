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

#include "juk.h"
#include "juk_p.h"
#include <juk_interface.h>

#include <QtDBus>


JukFactory::JukFactory(QObject* parent)
    : DBusPlayerFactory(parent)
{
}

Player::Ptr JukFactory::create(const QVariantList& args)
{
    Q_UNUSED(args);

    Juk* player = new Juk(this);
    if (!player->isRunning()) {
        delete player;
        player = 0;
    }
    return Player::Ptr(player);
}

bool JukFactory::matches(const QString& serviceName)
{
    return serviceName == "org.kde.juk";
}





Juk::Juk(PlayerFactory* factory)
    : Player(factory),
      jukPlayer(new JukPlayer("org.kde.juk", "/Player",
                              QDBusConnection::sessionBus()))
{
    setName("JuK");
}

Juk::~Juk()
{
    delete jukPlayer;
}

bool Juk::isRunning()
{
    if (!jukPlayer->isValid()) {
        delete jukPlayer;
        jukPlayer = new JukPlayer("org.kde.juk", "/Player",
                                    QDBusConnection::sessionBus());
    }
    return jukPlayer->isValid();
}

Player::State Juk::state()
{
    if (jukPlayer->isValid()) {
        if (jukPlayer->playing()) {
            return Playing;
        }
        if (jukPlayer->paused()) {
            return Paused;
        }
    }
    return Stopped;
}

QString Juk::artist()
{
    if (jukPlayer->isValid()) {
        return jukPlayer->trackProperty("Artist");
    }
    return QString();
}

QString Juk::album()
{
    if (jukPlayer->isValid()) {
        return jukPlayer->trackProperty("Album");
    }
    return QString();
}

QString Juk::title()
{
    if (jukPlayer->isValid()) {
        return jukPlayer->trackProperty("Title");
    }
    return QString();
}

int Juk::trackNumber()
{
    if (jukPlayer->isValid()) {
        return jukPlayer->trackProperty("Track").value().toInt();
    }
    return 0;
}

QString Juk::comment()
{
    if (jukPlayer->isValid()) {
        return jukPlayer->trackProperty("Comment");
    }
    return QString();
}

QString Juk::genre()
{
    if (jukPlayer->isValid()) {
        return jukPlayer->trackProperty("Genre");
    }
    return QString();
}

int Juk::length()
{
    if (jukPlayer->isValid()) {
        return jukPlayer->totalTime();
    }
    return 0;
}

int Juk::position()
{
    if (jukPlayer->isValid()) {
        return jukPlayer->currentTime();
    }
    return 0;
}

float Juk::volume()
{
    if (jukPlayer->isValid()) {
        return jukPlayer->volume();
    }
    return 0;
}

void Juk::play()
{
    if (jukPlayer->isValid()) {
        jukPlayer->play();
    }
}

void Juk::pause()
{
    if (jukPlayer->isValid()) {
        jukPlayer->pause();
    }
}

void Juk::stop()
{
    if (jukPlayer->isValid()) {
        jukPlayer->stop();
    }
}

void Juk::previous()
{
    if (jukPlayer->isValid()) {
        jukPlayer->back();
    }
}

void Juk::next()
{
    if (jukPlayer->isValid()) {
        jukPlayer->forward();
    }
}

void Juk::setVolume(float volume) {
    if (jukPlayer->isValid()) {
        jukPlayer->setVolume(volume);
    }
}

void Juk::seek(int time)
{
    if (jukPlayer->isValid()) {
        // jukPlayer->seek() wants milliseconds
        jukPlayer->seek(time * 1000);
    }
}


#include "juk.moc"
