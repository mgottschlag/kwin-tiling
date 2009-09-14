/***************************************************************************
 *   x11embedpainter.h                                                     *
 *                                                                         *
 *   Copyright (C) 2008 Jason Stubbs <jasonbstubbs@gmail.com>              *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#ifndef X11EMBEDPAINTER_H
#define X11EMBEDPAINTER_H

#include "x11embedcontainer.h"

#include <QtCore/QObject>


namespace SystemTray
{

class X11EmbedPainter : public QObject
{
    Q_OBJECT

public:
    X11EmbedPainter();
    ~X11EmbedPainter();

    void updateContainer(X11EmbedContainer *container);

private slots:
    void performUpdates();
    void removeContainer(QObject *container);

private:
    class Private;
    Private* const d;
};

}

#endif
