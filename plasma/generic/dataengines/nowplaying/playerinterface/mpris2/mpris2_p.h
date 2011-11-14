/*
 * Copyright 2011  Alex Merry <alex.merry@kdemail.net>
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
#ifndef MPRIS2_P_H
#define MPRIS2_P_H

#include "mpris2.h"
#include "../player.h"

#include <QTime>
#include <QVariantMap>
class QDBusInterface;

class Mpris2 : public QObject, public Player
{
    Q_OBJECT

public:
    /**
     * @param name     the media player name.
     */
    explicit Mpris2(const QString& name,
                   PlayerFactory* factory = 0);
    ~Mpris2();

    bool isRunning();
    State state();
    QString artist();
    QString album();
    QString title();
    int trackNumber();
    QString comment();
    QString genre();
    QString lyrics();
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
    void Seeked(qint64 position);
    void PropertiesChanged(const QString& interface,
                           const QVariantMap& changedProperties,
                           const QStringList& invalidatedProperties);

private:
    qint64 positionMs();
    void setup();
    bool getAllProps();
    void updatePosition(qreal rate);
    QVariant getPlayerProp(const QString& prop);
    bool updateBoolProp(const QString &name, 
                        const QVariantMap& changedProperties,
                        const QStringList& invalidatedProperties,
                        bool  currentVal);

    QDBusInterface* rootIface;
    QDBusInterface* playerIface;
    QDBusInterface* propsIface;

    qint64     m_pos; // milliseconds
    qreal      m_rate;
    qreal      m_currentRate; // 0.0 if not playing, Rate otherwise
    QDateTime  m_posLastUpdated;

    QString m_playerName;
    QString m_identity;
    QVariantMap m_metadata;
    float m_volume;
    State m_state;
    bool m_canControl;
    bool m_canPlay;
    bool m_canPause;
    bool m_canGoPrevious;
    bool m_canGoNext;
    bool m_canSeek;
    QMap<QString,QString> m_artfiles;
    bool m_artworkLoaded;
    QPixmap m_artwork;
};

#endif // MPRIS2_P_H
// vim:et:sts=4:sw=4
