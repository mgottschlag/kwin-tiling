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
#ifndef XMMS_P_H
#define XMMS_P_H

#include "xmms.h"
#include "player.h"

class Xmms : public Player
{
public:
    explicit Xmms(int session = 0, PlayerFactory* factory = 0);
    ~Xmms();

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
    void setVolume(float volume);

    bool canSeek() { return state() != Stopped; }
    void seek(int time);

private:
    int m_session;
};

#endif // XMMS_P_H
