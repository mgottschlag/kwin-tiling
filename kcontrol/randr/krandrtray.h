/*
 * Copyright (c) 2002 Hamish Rodda <meddie@yoyo.its.monash.edu.au>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#ifndef KRANDRTRAY_H
#define KRANDRTRAY_H

#include <ksystemtray.h>

#include "randr.h"

class KRandRSystemTray :  public KSystemTray, public RandRDisplay
{
	Q_OBJECT

public:
	KRandRSystemTray(QWidget* parent = 0, const char *name = 0);

	virtual void mouseReleaseEvent(QMouseEvent* e);
	virtual void contextMenuAboutToShow(KPopupMenu* menu);

	void configChanged();

protected slots:
	void slotSwitchScreen();
	void slotResolutionChanged(int parameter);
	void slotOrientationChanged(int parameter);
	void slotRefreshRateChanged(int parameter);

private:
	void populateMenu(KPopupMenu* menu);

	bool m_popupUp;
};

#endif
