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

#ifndef MEDIAAPPLET_H
#define MEDIAAPPLET_H

#ifdef HAVE_CONFIG_H
        #include <config.h>
#endif

#include <QLinkedList>
#include <QString>

#include <kpanelapplet.h>
#include <kconfig.h>
#include <kurl.h>
#include <kfileitem.h>
#include <kdirlister.h>

#include "mediumbutton.h"
typedef QLinkedList<MediumButton*> MediumButtonList;


class MediaApplet : public KPanelApplet
{
Q_OBJECT

public:
	MediaApplet(const QString& configFile, Plasma::Type t = Plasma::Normal, int actions = 0,
	              QWidget *parent = 0, const char *name = 0);
	~MediaApplet();

	int widthForHeight(int height) const;
	int heightForWidth(int width) const;
	void about();
	void preferences();

protected:
	void arrangeButtons();
	void resizeEvent(QResizeEvent *e);
	void positionChange(Plasma::Position p);
	void reloadList();
	void loadConfig();
	void saveConfig();
	void mousePressEvent(QMouseEvent *e);

protected Q_SLOTS:
	void slotClear();
	void slotStarted(const KUrl &url);
	void slotCompleted();
	void slotNewItems(const KFileItemList &entries);
	void slotDeleteItem(KFileItem *fileItem);
	void slotRefreshItems(const KFileItemList &entries);

private:
	KDirLister *mpDirLister;
	MediumButtonList mButtonList;
	QStringList mExcludedTypesList;
	QStringList mExcludedList;
	KFileItemList mMedia;
        int mButtonSizeSum;
};

#endif
