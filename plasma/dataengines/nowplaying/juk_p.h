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
#ifndef JUK_P_H
#define JUK_P_H

#include "juk.h"
#include "player.h"

#include <QObject>

class OrgKdeJukPlayerInterface;



class Juk : public Player
{
public:
    Juk(PlayerFactory* factory = 0);
    ~Juk();

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
    void setVolume(qreal volume);

    bool canSeek() { return state() != Stopped; }
    void seek(int time);

private:
    typedef OrgKdeJukPlayerInterface JukPlayer;
    JukPlayer* jukPlayer;
};

#endif // JUK_P_H
