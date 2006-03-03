/* kastaskitem.h
**
** Copyright (C) 2001-2004 Richard Moore <rich@kde.org>
** Contributor: Mosfet
**     All rights reserved.
**
** KasBar is dual-licensed: you can choose the GPL or the BSD license.
** Short forms of both licenses are included below.
*/

/*
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program in a file called COPYING; if not, write to
** the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
** MA 02110-1301, USA.
*/

/*
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions
** are met:
** 1. Redistributions of source code must retain the above copyright
**    notice, this list of conditions and the following disclaimer.
** 2. Redistributions in binary form must reproduce the above copyright
**    notice, this list of conditions and the following disclaimer in the
**    documentation and/or other materials provided with the distribution.
**
** THIS SOFTWARE IS PROVIDED BY THE AUTHORS AND CONTRIBUTORS ``AS IS'' AND
** ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
** IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
** ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
** FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
** DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
** OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
** HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
** LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
** OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
** SUCH DAMAGE.
*/

/*
** Bug reports and questions can be sent to kde-devel@kde.org
*/
// -*- c++ -*-


#ifndef KASTASKITEM_H
#define KASTASKITEM_H

#include <qpixmap.h>
//Added by qt3to4:
#include <QMouseEvent>
#include "kasitem.h"

class Task;
class KasPopup;
class KasTasker;
class KPixmap;

/**
 * A KasItem that represents a single Task.
 */
class KasTaskItem : public KasItem
{
    Q_OBJECT

 public:
    KasTaskItem( KasTasker *parent, Task::TaskPtr task );
    virtual ~KasTaskItem();

    QPixmap icon();

    /** Reimplemented to paint the item. */
    virtual void paint( QPainter *p );

    /** Returns the task the item is displaying. */
    Task::TaskPtr task() const { return task_; }

    /** Returns the parent KasTasker object. */
    KasTasker *kasbar() const;

    QString expandMacros( const QString &format, QObject *data );

public Q_SLOTS:
    void updateTask();

    /** Create a thumbnail for this task (does nothing if they're disabled). */
    void refreshThumbnail();

    void startAutoThumbnail();
    void stopAutoThumbnail();

    void iconChanged();
    void checkAttention();

    void showWindowMenuAt( QPoint pos );
    void sendToTray();
    void showPropertiesDialog();

    void toggleActivateAction();
    void showWindowMenuAt( QMouseEvent *ev );

protected:
    /**
     * Reimplemented to create a KasTaskPopup.
     */
    virtual KasPopup *createPopup();

    QWidget *createTaskProps( QObject *target, QWidget *parent=0, bool recursive=true );
    QWidget *createX11Props( QWidget *tabbed );
    QWidget *createNETProps( QWidget *tabbed );

    /**
     * Reimplemented to activate the task.
     */
    void dragOverAction();

private:
    Task::TaskPtr task_;
    QTimer *thumbTimer;
    bool usedIconLoader;
    bool iconHasChanged;
    QTimer *attentionTimer;
};

#endif // KASTASKITEM_H

