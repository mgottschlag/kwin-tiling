/*
 * Copyright (c) 2002,2003 Hamish Rodda <rodda@kde.org>
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

#include <qtimer.h>

#include <kaction.h>
#include <kapplication.h>
#include <kcmultidialog.h>
#include <kdebug.h>
#include <khelpmenu.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kpopupmenu.h>
#include <kstdaction.h>
#include <kstdguiitem.h>

#include "krandrtray.h"
#include "krandrpassivepopup.h"
#include "krandrtray.moc"

KRandRSystemTray::KRandRSystemTray(QWidget* parent, const char *name)
	: KSystemTray(parent, name)
	, m_popupUp(false)
	, m_help(new KHelpMenu(this, KGlobal::instance()->aboutData(), false, actionCollection()))
{
	setPixmap(SmallIcon("kscreensaver"));
	setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
	connect(contextMenu(), SIGNAL(activated(int)), SLOT(slotSwitchScreen()));
	connect(this, SIGNAL(quitSelected()), kapp, SLOT(quit()));
}

void KRandRSystemTray::mousePressEvent(QMouseEvent* e)
{
	// Popup the context menu with left-click
	if (e->button() == LeftButton) {
		contextMenuAboutToShow(contextMenu());
		contextMenu()->popup(e->globalPos());
		e->accept();
		return;
	}

	KSystemTray::mousePressEvent(e);
}

void KRandRSystemTray::contextMenuAboutToShow(KPopupMenu* menu)
{
	int lastIndex = 0;

	menu->clear();
	menu->setCheckable(true);

	if (!isValid()) {
		lastIndex = menu->insertItem(i18n("Required X Extension Not Available"));
		menu->setItemEnabled(lastIndex, false);

	} else {
		for (int s = 0; s < numScreens() && numScreens() > 1; s++) {
			setCurrentScreen(s);
			if (s == screenIndexOfWidget(this)) {
				lastIndex = menu->insertItem(i18n("Screen %1").arg(s+1));
				menu->setItemEnabled(lastIndex, false);
			} else {
				KPopupMenu* subMenu = new KPopupMenu(menu, QString("screen%1").arg(s+1).latin1());
				populateMenu(subMenu);
				menu->insertItem(i18n("Screen %1").arg(s+1), subMenu);
				connect(subMenu, SIGNAL(activated(int)), SLOT(slotActivated(int)));
			}
		}

		setCurrentScreen(screenIndexOfWidget(this));
		populateMenu(menu);
	}

	menu->insertSeparator();

	KAction *actPrefs = new KAction( i18n( "Configure Display..." ),
		SmallIconSet( "configure" ), KShortcut(), this, SLOT( slotPrefs() ),
		actionCollection() );
	actPrefs->plug( menu );

	menu->insertItem(KStdGuiItem::help().text(), m_help->menu());
	KAction *quitAction = actionCollection()->action(KStdAction::name(KStdAction::Quit));
	quitAction->plug(menu);
}

void KRandRSystemTray::configChanged()
{
	refresh();

  static bool first = true;

  if (!first)
	KRandrPassivePopup::message(
	    i18n("Screen configuration has changed"),
	    currentScreen()->changedMessage(), SmallIcon("window_fullscreen"),
	    this, "ScreenChangeNotification");

  first = false;
}

void KRandRSystemTray::populateMenu(KPopupMenu* menu)
{
	int lastIndex = 0;

	menu->insertTitle(SmallIcon("window_fullscreen"), i18n("Screen Size"));

	for (int i = 0; i < (int)currentScreen()->numSizes(); i++) {
		lastIndex = menu->insertItem(i18n("%1 x %2").arg(currentScreen()->pixelSize(i).width()).arg(currentScreen()->pixelSize(i).height()));

		if (currentScreen()->proposedSize() == i)
			menu->setItemChecked(lastIndex, true);

		menu->setItemParameter(lastIndex, i);
		menu->connectItem(lastIndex, this, SLOT(slotResolutionChanged(int)));
	}

	// Don't display the rotation options if there is no point (ie. none are supported)
	// XFree86 4.3 does not include rotation support.
	if (currentScreen()->rotations() != RandRScreen::Rotate0) {
		menu->insertTitle(SmallIcon("reload"), i18n("Orientation"));

		for (int i = 0; i < 6; i++) {
			if ((1 << i) & currentScreen()->rotations()) {
				lastIndex = menu->insertItem(currentScreen()->rotationIcon(1 << i), RandRScreen::rotationName(1 << i));

				if (currentScreen()->proposedRotation() & (1 << i))
					menu->setItemChecked(lastIndex, true);

				menu->setItemParameter(lastIndex, 1 << i);
				menu->connectItem(lastIndex, this, SLOT(slotOrientationChanged(int)));
			}
		}
	}

	QStringList rr = currentScreen()->refreshRates(currentScreen()->proposedSize());

	if (rr.count())
		menu->insertTitle(SmallIcon("clock"), i18n("Refresh Rate"));

	int i = 0;
	for (QStringList::Iterator it = rr.begin(); it != rr.end(); it++, i++) {
		lastIndex = menu->insertItem(*it);

		if (currentScreen()->proposedRefreshRate() == i)
			menu->setItemChecked(lastIndex, true);

		menu->setItemParameter(lastIndex, i);
		menu->connectItem(lastIndex, this, SLOT(slotRefreshRateChanged(int)));
	}
}

void KRandRSystemTray::slotSwitchScreen()
{
	if (sender() == contextMenu()) return;

	for (int i = 0; i < (int)contextMenu()->count(); i++) {
		//kdDebug() << contextMenu()->find(contextMenu()->idAt(i)) << endl;
		if (sender() == contextMenu()->find(contextMenu()->idAt(i))) {
			setCurrentScreen(i);
			return;
		}
	}

	Q_ASSERT(false);
}

void KRandRSystemTray::slotResolutionChanged(int parameter)
{
	if (currentScreen()->currentSize() == parameter)
		return;

	currentScreen()->proposeSize(parameter);

	currentScreen()->proposeRefreshRate(-1);

	if (currentScreen()->applyProposedAndConfirm()) {
		KConfig config("kcmrandrrc");
		if (syncTrayApp(config))
			currentScreen()->save(config);
	}
}

void KRandRSystemTray::slotOrientationChanged(int parameter)
{
	int propose = currentScreen()->currentRotation();

	if (parameter & RandRScreen::RotateMask)
		propose &= RandRScreen::ReflectMask;

	propose ^= parameter;

	if (currentScreen()->currentRotation() == propose)
		return;

	currentScreen()->proposeRotation(propose);

	if (currentScreen()->applyProposedAndConfirm()) {
		KConfig config("kcmrandrrc");
		if (syncTrayApp(config))
			currentScreen()->save(config);
	}
}

void KRandRSystemTray::slotRefreshRateChanged(int parameter)
{
	if (currentScreen()->currentRefreshRate() == parameter)
		return;

	currentScreen()->proposeRefreshRate(parameter);

	if (currentScreen()->applyProposedAndConfirm()) {
		KConfig config("kcmrandrrc");
		if (syncTrayApp(config))
			currentScreen()->save(config);
	}
}

void KRandRSystemTray::slotPrefs()
{
	KCMultiDialog *kcm = new KCMultiDialog( KDialogBase::Plain, i18n( "Configure" ), this );

	kcm->addModule( "display" );
	kcm->setPlainCaption( i18n( "Configure Display" ) );
	kcm->exec();
}
