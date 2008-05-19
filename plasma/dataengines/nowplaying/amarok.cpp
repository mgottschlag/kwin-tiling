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

#include "amarok.h"
#include "amarok_p.h"
#include <amarok_interface.h>

#include <KDebug>

#include <QtDBus>


AmarokFactory::AmarokFactory(QObject* parent)
    : DBusPlayerFactory(parent)
{
}

Player::Ptr AmarokFactory::create(const QVariantList& args)
{
    Q_UNUSED(args);

    Amarok* player = new Amarok(this);
    if (!player->isRunning()) {
        delete player;
        player = 0;
    }
    return Player::Ptr(player);
}

bool AmarokFactory::matches(const QString& serviceName)
{
    return serviceName == "org.kde.amarok";
}





Amarok::Amarok(PlayerFactory* factory)
    : Player(factory),
      amarokPlayer(new AmarokPlayer("org.kde.amarok", "/Player",
                              QDBusConnection::sessionBus()))
{
    setName("Amarok");
}

Amarok::~Amarok()
{
    delete amarokPlayer;
}

bool Amarok::isRunning()
{
    if (!amarokPlayer->isValid()) {
        delete amarokPlayer;
        amarokPlayer = new AmarokPlayer("org.kde.amarok", "/Player",
                                    QDBusConnection::sessionBus());
    }
    return amarokPlayer->isValid();
}

Player::State Amarok::state()
{
    if (amarokPlayer->isValid()) {
        switch (amarokPlayer->status()) {
            case 2:
                return Playing;
            case 1:
                return Paused;
            default:
                return Stopped;
        }
    }
    return Stopped;
}

QString Amarok::artist()
{
    if (amarokPlayer->isValid()) {
        return amarokPlayer->artist();
    }
    return QString();
}

QString Amarok::album()
{
    if (amarokPlayer->isValid()) {
        return amarokPlayer->album();
    }
    return QString();
}

QString Amarok::title()
{
    if (amarokPlayer->isValid()) {
        return amarokPlayer->title();
    }
    return QString();
}

int Amarok::trackNumber()
{
    if (amarokPlayer->isValid()) {
        return amarokPlayer->track().value().toInt();
    }
    return 0;
}

QString Amarok::comment()
{
    if (amarokPlayer->isValid()) {
        return amarokPlayer->comment();
    }
    return QString();
}

QString Amarok::genre()
{
    if (amarokPlayer->isValid()) {
        return amarokPlayer->genre();
    }
    return QString();
}

int Amarok::length()
{
    if (amarokPlayer->isValid()) {
        return amarokPlayer->trackTotalTime();
    }
    return 0;
}

int Amarok::position()
{
    if (amarokPlayer->isValid()) {
        // this doesn't look right - is Amarok giving the wrong info?
        return amarokPlayer->trackCurrentTime();
    }
    return 0;
}

float Amarok::volume()
{
    if (amarokPlayer->isValid()) {
        return float(amarokPlayer->getVolume()) / 100;
    }
    return 0;
}

void Amarok::play()
{
    if (amarokPlayer->isValid()) {
        amarokPlayer->play();
    }
}

void Amarok::pause()
{
    if (amarokPlayer->isValid()) {
        amarokPlayer->pause();
    }
}

void Amarok::stop()
{
    if (amarokPlayer->isValid()) {
        amarokPlayer->stop();
    }
}

void Amarok::previous()
{
    if (amarokPlayer->isValid()) {
        amarokPlayer->prev();
    }
}

void Amarok::next()
{
    if (amarokPlayer->isValid()) {
        amarokPlayer->next();
    }
}

void Amarok::setVolume(float volume) {
    if (amarokPlayer->isValid()) {
        amarokPlayer->setVolume(volume * 100);
    }
}

void Amarok::seek(int time)
{
    if (amarokPlayer->isValid()) {
        amarokPlayer->seek(time);
    }
}


#include "amarok.moc"
