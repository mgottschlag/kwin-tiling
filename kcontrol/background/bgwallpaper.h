/* vi: ts=8 sts=4 sw=4

   This file is part of the KDE project, module kcmbackground.

   Copyright (C) 1999 Geert Jansen <g.t.jansen@stud.tue.nl>
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

#ifndef _BGWALLPAPER_H_
#define _BGWALLPAPER_H_

#include <q3listbox.h>
#include <QStringList>
//Added by qt3to4:
#include <QDragEnterEvent>
#include <QDropEvent>

#include <kdialogbase.h>

class BGMultiWallpaperBase;
class KBackgroundSettings;

class BGMultiWallpaperList : public Q3ListBox
{
public:
   BGMultiWallpaperList(QWidget *parent, const char *name = 0);

   void dragEnterEvent(QDragEnterEvent *ev);
   void dropEvent(QDropEvent *ev);
   bool hasSelection();
   void ensureSelectionVisible();
};

class BGMultiWallpaperDialog : public KDialogBase
{
   Q_OBJECT
public:
   BGMultiWallpaperDialog(KBackgroundSettings *settings, QWidget *parent, const char *name=0);

public Q_SLOTS:
   void slotAdd();
   void slotRemove();
   void slotMoveUp();
   void slotMoveDown();
   void slotOk();
   void slotItemSelected( Q3ListBoxItem * );
private:
   void setEnabledMoveButtons();

   KBackgroundSettings *m_pSettings;

   BGMultiWallpaperBase *dlg;
};

#endif
