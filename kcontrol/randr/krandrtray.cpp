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
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <qtimer.h>

#include <kdebug.h>
#include <klocale.h>
#include <kpopupmenu.h>
#include <kiconloader.h>
#include <kaction.h>
#include <kapplication.h>
#include <kpassivepopup.h>

#include "krandrtray.h"
#include "krandrtray.moc"

KRandRSystemTray::KRandRSystemTray(QWidget* parent, const char *name)
	: KSystemTray(parent, name)
	, RandRDisplay(true)
	, m_popupUp(false)
	, m_resizeCount(-1)
{
	installEventFilter(this);
	setPixmap(SmallIcon("kscreensaver"));
	connect(contextMenu(), SIGNAL(activated(int)), SLOT(slotSwitchScreen()));
}

void KRandRSystemTray::mousePressEvent(QMouseEvent* e)
{
	// Popup the context menu with left-click
	if (e->button() == LeftButton) {
		contextMenuAboutToShow(contextMenu());
		contextMenu()->popup(e->globalPos());
		e->accept();
	} else {
		KSystemTray::mousePressEvent(e);
	}
}

void KRandRSystemTray::contextMenuAboutToShow(KPopupMenu* menu)
{
	int lastIndex = 0;
	
	menu->clear();
	menu->setCheckable(true);
	menu->insertTitle(SmallIcon("kscreensaver"), i18n("Display Configuration"), 0, 0);

	if (!isValid()) {
		lastIndex = menu->insertItem("Required extension not avaliable");
		menu->setItemEnabled(lastIndex, false);
		return;
	}
	
	for (int s = 0; s < m_numScreens; s++) {
		setScreen(s);
		if (s == widgetScreen(this)) {
			lastIndex = menu->insertItem(i18n("Screen %1").arg(s+1));
			menu->setItemEnabled(lastIndex, false);
		} else {
			KPopupMenu* subMenu = new KPopupMenu(menu, QString("screen%1").arg(s+1).latin1());
			populateMenu(subMenu);
			menu->insertItem(i18n("Screen %1").arg(s+1), subMenu);
			connect(subMenu, SIGNAL(activated(int)), SLOT(slotActivated(int)));
		}
	}
	
	setScreen(widgetScreen(this));
	populateMenu(menu);
	
	menu->insertSeparator();
	KStdAction::quit(KApplication::kApplication(), SLOT(quit()), actionCollection())->plug(menu);
}

void KRandRSystemTray::configChanged()
{
	refresh();
	
	// A bit of a hack.. kicker gets restarted, we get resized twice, we wait a bit for kicker to be ready, and after that we're ready to display the change info.
	m_resizeCount = 2;
}

void KRandRSystemTray::slotDisplayInformation()
{
	KPassivePopup::message(i18n("Screen configuration has changed"), m_currentScreen->changedMessage(), SmallIcon("window_fullscreen"), this, "ScreenChangeNotification");
}

void KRandRSystemTray::populateMenu(KPopupMenu* menu)
{
	menu->insertSeparator();
	int lastIndex = 0;
	
	menu->insertTitle(SmallIcon("window_fullscreen"), i18n("Screen Size"));
	
	for (int i = 0; i < (int)m_currentScreen->sizes.count(); i++) {
		lastIndex = menu->insertItem(i18n("%1 x %2").arg(m_currentScreen->sizes[i].width).arg(m_currentScreen->sizes[i].height));
		
		if (m_currentScreen->proposedSize == i) {
			menu->setItemChecked(lastIndex, true);
			menu->setItemEnabled(lastIndex, false);
		}
		
		menu->setItemParameter(lastIndex, i);
		menu->connectItem(lastIndex, this, SLOT(slotResolutionChanged(int)));
		
		if (m_currentScreen->sizes.count() == 1) menu->setItemEnabled(lastIndex, false);
	}
	
	menu->insertSeparator();
	menu->insertTitle(SmallIcon("reload"), i18n("Orientation"));

	for (int i = 0; i < 6; i++) {
		if ((1 << i) & m_currentScreen->rotations) {
			lastIndex = menu->insertItem(RandRScreen::rotationIcon(1 << i), RandRScreen::rotationName(1 << i));
			
			if (m_currentScreen->proposedRotation & (1 << i)) {
				menu->setItemChecked(lastIndex, true);
				if (i < 4)
					menu->setItemEnabled(lastIndex, false);
			}
			
			menu->setItemParameter(lastIndex, 1 << i);
			menu->connectItem(lastIndex, this, SLOT(slotOrientationChanged(int)));
		}
	}
		
	menu->insertSeparator();
	menu->insertTitle(SmallIcon("clock"), i18n("Refresh rate"));
	
	QStringList rr = m_currentScreen->refreshRates(m_currentScreen->proposedSize);
	
	int i = 0;
	for (QStringList::Iterator it = rr.begin(); it != rr.end(); it++, i++) {
		lastIndex = menu->insertItem(*it);
		
		if (m_currentScreen->proposedRefreshRateIndex == i) {
			menu->setItemChecked(lastIndex, true);
			menu->setItemEnabled(lastIndex, false);
		}
		
		menu->setItemParameter(lastIndex, i);
		menu->connectItem(lastIndex, this, SLOT(slotRefreshRateChanged(int)));
	}
	
	if (rr.count() < 2)
		menu->setItemEnabled(lastIndex, false);
}

void KRandRSystemTray::slotSwitchScreen()
{
	if (sender() == contextMenu()) return;
	
	for (int i = 0; i < (int)contextMenu()->count(); i++) {
		kdDebug() << contextMenu()->find(contextMenu()->idAt(i)) << endl;
		if (sender() == contextMenu()->find(contextMenu()->idAt(i))) {
			setScreen(i);
			return;
		}
	}
	
	Q_ASSERT(false);
}

void KRandRSystemTray::slotResolutionChanged(int parameter)
{
	m_currentScreen->proposedSize = parameter;
	
	m_currentScreen->applyProposedAndConfirm();
}

void KRandRSystemTray::slotOrientationChanged(int parameter)
{
	m_currentScreen->proposedRotation ^= parameter;
	
	m_currentScreen->applyProposedAndConfirm();
}

void KRandRSystemTray::slotRefreshRateChanged(int parameter)
{
	m_currentScreen->proposedRefreshRate = parameter;
	
	m_currentScreen->applyProposedAndConfirm();
}

bool KRandRSystemTray::eventFilter(QObject* watched, QEvent* e)
{
	if (e->type() == 14 && m_resizeCount > 0) {
		m_resizeCount--;
		if (!m_resizeCount) {
			QTimer::singleShot(1000, this, SLOT(slotDisplayInformation()));
			m_resizeCount = -1;
		}
	}
	return false;
}
