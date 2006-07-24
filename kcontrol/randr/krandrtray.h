/*
 * Copyright (c) 2002 Hamish Rodda <rodda@kde.org>
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef KRANDRTRAY_H
#define KRANDRTRAY_H

#include <QMouseEvent>

#include <ksystemtrayicon.h>

#include "randr.h"

class KHelpMenu;
class QMenu;

class KRandRSystemTray :  public KSystemTrayIcon, public RandRDisplay
{
	Q_OBJECT

public:
	KRandRSystemTray(QWidget* parent = 0);

	void configChanged();

protected Q_SLOTS:
	void slotScreenActivated();
	void slotResolutionChanged(int parameter);
	void slotOrientationChanged(int parameter);
	void slotRefreshRateChanged(int parameter);
	void slotPrefs();
	void slotActivated(QSystemTrayIcon::ActivationReason reason);

protected:
	void prepareMenu();

private:
	void populateMenu(KMenu* menu);

	bool m_popupUp;
	KHelpMenu* m_help;
	QList<KMenu*> m_screenPopups;
};

#endif
