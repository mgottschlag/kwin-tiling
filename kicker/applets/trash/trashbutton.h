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

#ifndef TRASHBUTTON_H
#define TRASHBUTTON_H

#include <panelbutton.h>
#include <qpoint.h>
#include <qstring.h>
#include <qpixmap.h>
#include <qcursor.h>
#include <qtimer.h>
#include <kfileitem.h>
#include <kpanelapplet.h>
#include <kactioncollection.h>

class TrashButton : public PanelPopupButton
{
Q_OBJECT

public:
	TrashButton(QWidget *parent);
	~TrashButton();
	void setItemCount(int count);
	void setPanelPosition(Plasma::Position position);

protected:
	QString tileName();
	void initPopup();
	void dragEnterEvent(QDragEnterEvent *e);
	void dropEvent(QDropEvent *e);

protected Q_SLOTS:
        // Activate this code only if we find a way to have both an
	// action and a popup menu for the same kicker button
	//void slotClicked();
	void slotPaste();

private:
	KActionCollection mActions;
	KFileItem mFileItem;
};

#endif
