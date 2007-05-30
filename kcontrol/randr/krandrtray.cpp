/*
 * Copyright (c) 2007      Gustavo Pichorim Boiko <gustavo.boiko@kdemail.net>
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <QTimer>

//Added by qt3to4:
#include <QMouseEvent>

#include <kactioncollection.h>
#include <kaction.h>
#include <kapplication.h>
#include <kcmultidialog.h>
#include <kdebug.h>
#include <khelpmenu.h>
#include <kicon.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmenu.h>
#include <kstandardaction.h>
#include <kstandardguiitem.h>
#include "krandrtray.h"
#include "krandrpassivepopup.h"
#include "krandrtray.moc"
#include "legacyrandrscreen.h"

KRandRSystemTray::KRandRSystemTray(QWidget* parent)
	: KSystemTrayIcon(parent)
	, m_popupUp(false)
	, m_help(new KHelpMenu(parent, KGlobal::mainComponent().aboutData(), false, actionCollection()))
{
	setIcon(KSystemTrayIcon::loadIcon("randr"));
	connect(this, SIGNAL(quitSelected()), kapp, SLOT(quit()));
	this->setToolTip( i18n("Screen resize & rotate"));
        prepareMenu();
}

void KRandRSystemTray::slotActivated(QSystemTrayIcon::ActivationReason reason)
{
	if (reason == QSystemTrayIcon::Trigger)
		contextMenu()->show();
}

void KRandRSystemTray::prepareMenu()
{
	int lastIndex = 0;
        KMenu *menu = new KMenu(parentWidget());
        setContextMenu(menu);

	menu->clear();

	if (!isValid()) {
		lastIndex = menu->insertItem(i18n("Required X Extension Not Available"));
		menu->setItemEnabled(lastIndex, false);

	} else {
		m_screenPopups.clear();
		for (int s = 0; s < numScreens(); s++) {
			setCurrentScreen(s);
			if (s == screenIndexOfWidget(parentWidget())) {
				/*lastIndex = menu->insertItem(i18n("Screen %1").arg(s+1));
				menu->setItemEnabled(lastIndex, false);*/
			} else {
                            KMenu* subMenu = new KMenu(menu );
                            subMenu->setObjectName( QString("screen%1").arg(s+1) );
                            m_screenPopups.append(subMenu);
                            populateMenu(subMenu);
                            lastIndex = menu->insertItem(i18n("Screen %1", s+1), subMenu);
                            connect(subMenu, SIGNAL(activated(int)), SLOT(slotScreenActivated()));
			}
		}

		setCurrentScreen(screenIndexOfWidget(parentWidget()));
		populateMenu(menu);
	}

	menu->addSeparator();

	QAction *actPrefs = actionCollection()->addAction( QString() );
        actPrefs->setIcon( KIcon( "configure" ) );
        actPrefs->setText( i18n( "Configure Display..." ) );
        connect( actPrefs, SIGNAL( triggered( bool ) ), SLOT( slotPrefs() ) );
	menu->addAction( actPrefs );

	menu->insertItem(SmallIcon("help-contents"),KStandardGuiItem::help().text(), m_help->menu());
	QAction *quitAction = actionCollection()->action(KStandardAction::name(KStandardAction::Quit));
	menu->addAction( quitAction );
}

void KRandRSystemTray::slotScreenActivated()
{
	setCurrentScreen(m_screenPopups.indexOf(static_cast<KMenu*>(sender())));
}

void KRandRSystemTray::configChanged()
{
	refresh();

	static bool first = true;

	if (!first)
	{
		QString message;
#ifdef HAS_RANDR_1_2
		if (RandR::has_1_2)
			// TODO: display config changed message
			message = "Screen config changed";
		else
#endif
			message = currentLegacyScreen()->changedMessage();

		KRandrPassivePopup::message(
		i18n("Screen configuration has changed"),
		message, SmallIcon("view-fullscreen"),
		parentWidget());
	}

	first = false;
}

void KRandRSystemTray::populateMenu(KMenu* menu)
{
#if HAS_RANDR_1_2
	if (RandR::has_1_2)
		//TODO: populate the menu with the required items
		currentScreenIndex();
	else
#endif
		populateLegacyMenu(menu);
}

void KRandRSystemTray::populateLegacyMenu(KMenu* menu)
{
	int lastIndex = 0;

	menu->addTitle(SmallIcon("view-fullscreen"), i18n("Screen Size"));

	LegacyRandRScreen *screen = currentLegacyScreen();
	Q_ASSERT(screen);

	int numSizes = screen->numSizes();
	int* sizeSort = new int[numSizes];

	for (int i = 0; i < numSizes; i++) {
		sizeSort[i] = screen->pixelCount(i);
	}

	for (int j = 0; j < numSizes; j++) {
		int highest = -1, highestIndex = -1;

		for (int i = 0; i < numSizes; i++) {
			if (sizeSort[i] && sizeSort[i] > highest) {
				highest = sizeSort[i];
				highestIndex = i;
			}
		}
		sizeSort[highestIndex] = -1;
		Q_ASSERT(highestIndex != -1);

		lastIndex = menu->insertItem(i18n("%1 x %2", screen->pixelSize(highestIndex).width(), screen->pixelSize(highestIndex).height()));

		if (screen->proposedSize() == highestIndex)
			menu->setItemChecked(lastIndex, true);

		menu->setItemParameter(lastIndex, highestIndex);
		menu->connectItem(lastIndex, this, SLOT(slotResolutionChanged(int)));
	}
	delete [] sizeSort;
	sizeSort = 0L;

	// Don't display the rotation options if there is no point (ie. none are supported)
	// XFree86 4.3 does not include rotation support.
	if (screen->rotations() != RandR::Rotate0) {
		menu->addTitle(SmallIcon("view-refresh"), i18n("Orientation"));

		for (int i = 0; i < 6; i++) {
			if ((1 << i) & screen->rotations()) {
				lastIndex = menu->insertItem(	QIcon(RandR::rotationIcon(1 << i, screen->currentRotation())), 
								RandR::rotationName(1 << i));

				if (screen->proposedRotation() & (1 << i))
					menu->setItemChecked(lastIndex, true);

				menu->setItemParameter(lastIndex, 1 << i);
				menu->connectItem(lastIndex, this, SLOT(slotOrientationChanged(int)));
			}
		}
	}

	QStringList rr = screen->refreshRates(screen->proposedSize());

	if (rr.count())
		menu->addTitle(SmallIcon("clock"), i18n("Refresh Rate"));

	int i = 0;
	for (QStringList::Iterator it = rr.begin(); it != rr.end(); ++it, i++) {
		lastIndex = menu->insertItem(*it);

		if (screen->proposedRefreshRate() == i)
			menu->setItemChecked(lastIndex, true);

		menu->setItemParameter(lastIndex, i);
		menu->connectItem(lastIndex, this, SLOT(slotRefreshRateChanged(int)));
	}
}

void KRandRSystemTray::slotResolutionChanged(int parameter)
{
#ifdef HAS_RANDR_1_2
	if (RandR::has_1_2)
		//TODO: this will become screen size changed, but for now it is ok
		currentScreenIndex();
	else
#endif
	{
		LegacyRandRScreen *screen = currentLegacyScreen();
		Q_ASSERT(screen);

		if (screen->currentSize() == parameter)
			return;

		screen->proposeSize(parameter);

		screen->proposeRefreshRate(-1);

		if (screen->applyProposedAndConfirm()) {
			KConfig config("kcmrandrrc");
			if (syncTrayApp(config))
				screen->save(config);
		}
	}
}

void KRandRSystemTray::slotOrientationChanged(int parameter)
{
#ifdef HAS_RANDR_1_2
	if (RandR::has_1_2)
		//TODO: I guess this won't be used in randr1.2, but later we'll see
		currentScreenIndex();
	else
#endif
	{
		LegacyRandRScreen *screen = currentLegacyScreen();
		Q_ASSERT(screen);
		
		int propose = screen->currentRotation();

		if (parameter & RandR::RotateMask)
			propose &= RandR::ReflectMask;

		propose ^= parameter;

		if (screen->currentRotation() == propose)
			return;

		screen->proposeRotation(propose);

		if (screen->applyProposedAndConfirm()) {
			KConfig config("kcmrandrrc");
			if (syncTrayApp(config))
				screen->save(config);
		}
	}
}

void KRandRSystemTray::slotRefreshRateChanged(int parameter)
{
#ifdef HAS_RANDR_1_2
	if (RandR::has_1_2)
		//TODO: this will probably have to change as refresh rate is differect for 
		//      each crtc
		currentScreenIndex();
	else
#endif
	{
		LegacyRandRScreen *screen = currentLegacyScreen();
		Q_ASSERT(screen);

		if (screen->currentRefreshRate() == parameter)
			return;

		screen->proposeRefreshRate(parameter);

		if (screen->applyProposedAndConfirm()) {
			KConfig config("kcmrandrrc");
			if (syncTrayApp(config))
				screen->save(config);
		}
	}
}

void KRandRSystemTray::slotPrefs()
{
	KCMultiDialog *kcm = new KCMultiDialog( parentWidget() );
	kcm->setFaceType( KCMultiDialog::Plain );
	kcm->setPlainCaption( i18n( "Configure Display" ) );
	kcm->addModule( "display" );
	kcm->exec();
}
