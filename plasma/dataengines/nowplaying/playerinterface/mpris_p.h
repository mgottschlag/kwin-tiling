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
#ifndef MPRIS_P_H
#define MPRIS_P_H

#include "mpris.h"
#include "player.h"

#include <QVariantMap>

class OrgFreedesktopMediaPlayerInterface;

class Mpris : public QObject, public Player
{
    Q_OBJECT

public:
    /**
     * @param name     the media player name.
     */
    explicit Mpris(const QString& name,
                   PlayerFactory* factory = 0);
    ~Mpris();

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
    QPixmap artwork();

    bool canPlay();
    void play();
    bool canPause();
    void pause();
    bool canStop();
    void stop();
    bool canGoPrevious();
    void previous();
    bool canGoNext();
    void next();

    bool canSetVolume();
    void setVolume(qreal volume);

    bool canSeek();
    void seek(int time);

private Q_SLOTS:
    void trackChanged(const QVariantMap& metadata);
    void stateChanged(int state);
    void capsChanged(int caps);

private:
    enum Caps {
        NO_CAPS               = 0,
        CAN_GO_NEXT           = 1,
        CAN_GO_PREV           = 2,
        CAN_PAUSE             = 4,
        CAN_PLAY              = 8,
        CAN_SEEK              = 16,
        CAN_PROVIDE_METADATA  = 32,
        CAN_HAS_TRACKLIST     = 64
    };
    void setup();
    OrgFreedesktopMediaPlayerInterface* m_player;

    QString m_playerName;
    QVariantMap m_metadata;
    State m_state;
    Caps m_caps;
    QMap<QString,QString> m_artfiles;
};

#endif // MPRIS_P_H
