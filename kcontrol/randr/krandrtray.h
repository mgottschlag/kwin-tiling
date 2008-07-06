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

#include <KSystemTrayIcon>

#include "randrdisplay.h"

class KHelpMenu;
class QAction;
class QActionGroup;
class KMenu;

class KRandRSystemTray : public KSystemTrayIcon
{
	Q_OBJECT

public:
	explicit KRandRSystemTray(RandRDisplay *dpy, QWidget* parent = 0);

	void configChanged();

protected Q_SLOTS:
	void slotActivated(QSystemTrayIcon::ActivationReason reason);
	void slotScreenActivated();
	void slotResolutionChanged(QAction *action);
	void slotOrientationChanged(QAction *action);
	void slotRefreshRateChanged(QAction *action);
	void slotPrefs();

	void slotPrepareMenu();

private:
	void populateMenu(KMenu* menu);
	void populateLegacyMenu(KMenu* menu);

	// helper functions
	QActionGroup *populateRotations(KMenu *menu, int rotations, int rotation);
	QActionGroup *populateSizes(KMenu *menu, const SizeList &sizes, const QSize &size);
	QActionGroup *populateRates(KMenu *menu, const RateList &rates, float rate);

	KHelpMenu* m_help;
	bool m_popupUp;
	QList<KMenu*> m_screenPopups;
	KMenu* m_menu;
	RandRDisplay *m_display;
};

#endif
