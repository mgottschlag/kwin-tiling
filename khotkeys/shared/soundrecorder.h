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
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.             *
 ***************************************************************************/
#ifndef RECORDER_H
#define RECORDER_H

#include <qobject.h>
#include "sound.h"
#include <kdemacros.h>

class KLibrary;

namespace KHotKeys
{



/**
@author Olivier Goffart
*/
class KDE_EXPORT SoundRecorder : public QObject
{
Q_OBJECT
public:
    static SoundRecorder* create( QObject* parent = 0, const char* name = 0 );
    virtual ~SoundRecorder();

    virtual void start();
    virtual void stop();
    virtual void abort();
    virtual Sound sound();

    static bool init( KLibrary* );
signals:
    void recorded(const Sound&);

protected:
    SoundRecorder(QObject *parent = 0, const char *name = 0);
    typedef SoundRecorder* (*create_ptr)( QObject*, const char* );
private:
    static create_ptr create_fun;
};

}

#endif
