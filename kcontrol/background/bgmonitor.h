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

#ifndef _BGMONITOR_H_
#define _BGMONITOR_H_

#include <qwidget.h>
//Added by qt3to4:
#include <QDragEnterEvent>
#include <QDropEvent>

/**
 * This class handles drops on the preview monitor.
 */
class BGMonitor : public QWidget
{
    Q_OBJECT
public:

    BGMonitor(QWidget *parent, const char *name=0L);

Q_SIGNALS:
    void imageDropped(const QString &);

protected:
    virtual void dropEvent(QDropEvent *);
    virtual void dragEnterEvent(QDragEnterEvent *);
};


#endif
