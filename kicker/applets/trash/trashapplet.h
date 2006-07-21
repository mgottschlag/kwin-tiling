/* This file is part of the KDE project
   Copyright (c) 2004 Kevin Ottens <ervin ipsquad net>

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

#ifndef TRASHAPPLET_H
#define TRASHAPPLET_H

#include <config.h>

#include <kpanelapplet.h>
#include <QString>
#include <kurl.h>
#include <kdirlister.h>

#include "trashbutton.h"

class TrashApplet : public KPanelApplet
{
Q_OBJECT

public:
	TrashApplet(const QString& configFile, Plasma::Type t = Plasma::Normal, int actions = 0,
	            QWidget *parent = 0);
	~TrashApplet();

	int widthForHeight(int height) const;
	int heightForWidth(int width) const;
	void about();

protected:
	void resizeEvent(QResizeEvent *e);
	void positionChange(Plasma::Position p);

protected Q_SLOTS:
	void slotClear();
	void slotCompleted();
	void slotDeleteItem(KFileItem *);

private:
	KDirLister *mpDirLister;
	TrashButton *mButton;
	int mCount;
};

#endif
