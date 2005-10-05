/* vi: ts=8 sts=4 sw=4

   This file is part of the KDE project, module kcmbackground.

   Copyright (C) 2002 Laurent Montel <montell@club-internet.fr>
   Copyright (C) 2003 Waldo Bastian <bastian@kde.org>
  
   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License 
   version 2 as published by the Free Software Foundation.
   
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.
   
   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
 */

#include <k3urldrag.h>

#include "bgmonitor.h"
//Added by qt3to4:
#include <QDragEnterEvent>
#include <QDropEvent>

BGMonitor::BGMonitor(QWidget *parent, const char *name)
    : QWidget(parent, name)
{
    setAcceptDrops(true);
}


void BGMonitor::dropEvent(QDropEvent *e)
{
    if (!K3URLDrag::canDecode(e))
        return;

    KURL::List uris;
    if (K3URLDrag::decode(e, uris) && (uris.count() > 0)) {
        // TODO: Download remote file
        if (uris.first().isLocalFile())
           emit imageDropped(uris.first().path());
    }
}

void BGMonitor::dragEnterEvent(QDragEnterEvent *e)
{
    if (K3URLDrag::canDecode(e))
        e->accept(rect());
    else
        e->ignore(rect());
}

#include "bgmonitor.moc"
