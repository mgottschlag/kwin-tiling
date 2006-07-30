/***************************************************************************
 *   Copyright (C) 2005 by Olivier Goffart   *
 *   ogoffart@kde.org   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#ifndef RECORDER_ARTS_H
#define RECORDER_ARTS_H

#include "soundrecorder.h"
#include <qmemarray.h>
#include "sound.h"

class  KAudioRecordStream ;
class KArtsServer;

namespace KHotKeys
{



/**
@author Olivier Goffart
*/
class SoundRecorderArts : public SoundRecorder
{
Q_OBJECT
public:
    SoundRecorderArts(QObject *parent = 0, const char *name = 0);
    virtual ~SoundRecorderArts();

    virtual void start();
    virtual void stop();
    virtual void abort();
    virtual Sound sound();

private slots:
    void slotDataReceived(QByteArray &data);
    void slotEmitSignal();

private:
    QByteArray m_data;
    QObject *m_dis;
    KArtsServer *m_server;
    KAudioRecordStream *m_recStream;
};

}

#endif
