/* This file is part of the KDE project
   Copyright 2003 Oswald Buddenhagen <ossi@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef _MAIN_H
#define _MAIN_H

#include <kapplication.h>

#include <time.h>

class MyApp : public KApplication {
    Q_OBJECT
public:
    MyApp() : KApplication(), lastTick( 0 ) {}
protected:
    bool x11EventFilter( XEvent * );
Q_SIGNALS:
    void activity();
private:
    time_t lastTick;
};

#endif
