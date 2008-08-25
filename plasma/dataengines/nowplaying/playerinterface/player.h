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

#ifndef PLAYER_H
#define PLAYER_H

#include <QSharedData>
#include <KSharedPtr>
#include <QString>
#include <QPixmap>

class PlayerFactory;

/**
 * Interface for getting information from and controlling a
 * media player
 */
class Player : public QSharedData
{
public:
    typedef KSharedPtr<Player> Ptr;

    Player(PlayerFactory* factory = 0);
    virtual ~Player();
    /**
      * a pointer to the factory that created this player object
      */
    PlayerFactory* factory() const;
    /**
     * The name of this player
     */
    QString name() const;
    /**
     * Current state of the player
     */
    enum State {
        Playing,
        Paused,
        Stopped
    };
    /**
     * Whether the player is running and accessible
     */
    virtual bool isRunning() = 0;
    /**
     * Current state of the player
     *
     * Undefined if !running()
     *
     * See State
     */
    virtual State state() = 0;
    /** Artist for current track.  May be empty */
    virtual QString artist();
    /** Album for current track.  May be empty */
    virtual QString album();
    /** Title of current track.  May be empty */
    virtual QString title();
    /**
     * Track number of current track.
     *
     * Note that this is the track on a CD or in an album, not
     * the playlist position
     *
     * 0 if no track defined, or unknown.
     */
    virtual int trackNumber();
    /** Comment for current track.  May be empty */
    virtual QString comment();
    /** Genre of current track.  May be empty */
    virtual QString genre();
    /**
     * Length of current track in seconds.
     *
     * 0 if unknown.
     */
    virtual int length();
    /**
     * Position of current track in seconds.
     *
     * 0 if unknown, not defined if Stopped.
     */
    virtual int position();
    /**
     * Current volume
     *
     * Value should be between 0 and 1
     *
     * -1 if unknown
     */
    virtual float volume();
    /**
     * Album artwork
     *
     * Null (pm.isNull()) if none available
     */
    virtual QPixmap artwork();

    // Commands

    /**
     * Play a track
     */
    virtual bool canPlay();
    virtual void play();
    /**
     * Pause the currently playing track
     */
    virtual bool canPause();
    virtual void pause();
    /**
     * Stop the currently playing track
     *
     * canStop() should usually be state() != Stopped if
     * stop() is implemented
     */
    virtual bool canStop();
    virtual void stop();
    /**
     * Move to the previous track
     */
    virtual bool canGoPrevious();
    virtual void previous();
    /**
     * Move to the next track
     */
    virtual bool canGoNext();
    virtual void next();

    /**
     * Set the volume
     *
     * Must be between 0 and 1
     */
    virtual bool canSetVolume();
    virtual void setVolume(qreal volume);

    /**
     * Set the position (in seconds)
     *
     * Should be <= length()
     */
    virtual bool canSeek();
    virtual void seek(int time);

protected:
    void setName(const QString& name);

private:
    QString m_name;
    PlayerFactory* m_factory;
};

#endif // PLAYER_H
