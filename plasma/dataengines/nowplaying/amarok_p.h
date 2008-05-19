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
#ifndef AMAROK_P_H
#define AMAROK_P_H

#include "amarok.h"
#include "player.h"

#include <QObject>

class OrgKdeAmarokPlayerInterface;



class Amarok : public Player
{
public:
    Amarok(PlayerFactory* factory = 0);
    ~Amarok();

    bool isRunning();
    State state();
    QString artist();
    QString album();
    QString title();
    int trackNumber();
    QString comment();
    QString genre();
    int length();
    int position();
    float volume();
    //FIXME: Amarok should (doesn't yet) provide a coverImage

    bool canPlay() { return state() != Playing; }
    void play();
    bool canPause() { return true; }
    void pause();
    bool canStop() { return state() != Stopped; }
    void stop();
    bool canGoPrevious() { return true; }
    void previous();
    bool canGoNext() { return true; }
    void next();

    bool canSetVolume() { return true; }
    void setVolume(float volume);

    bool canSeek() { return state() != Stopped; }
    void seek(int time);

private:
    typedef OrgKdeAmarokPlayerInterface AmarokPlayer;
    AmarokPlayer* amarokPlayer;
};

#endif // AMAROK_P_H
