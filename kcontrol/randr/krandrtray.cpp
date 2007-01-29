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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <QTimer>
#include <QToolTip>
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
#include <kactioncollection.h>
#include "krandrtray.h"
#include "krandrpassivepopup.h"
#include "krandrtray.moc"

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
		for (int s = 0; s < numScreens() /*&& numScreens() > 1 */; s++) {
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

	menu->insertItem(SmallIcon("help"),KStandardGuiItem::help().text(), m_help->menu());
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
		KRandrPassivePopup::message(
		i18n("Screen configuration has changed"),
		currentScreen()->changedMessage(), SmallIcon("window_fullscreen"),
		parentWidget());

	first = false;
}

void KRandRSystemTray::populateMenu(KMenu* menu)
{
	int lastIndex = 0;

	menu->addTitle(SmallIcon("window_fullscreen"), i18n("Screen Size"));

	int numSizes = currentScreen()->numSizes();
	int* sizeSort = new int[numSizes];

	for (int i = 0; i < numSizes; i++) {
		sizeSort[i] = currentScreen()->pixelCount(i);
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

		lastIndex = menu->insertItem(i18n("%1 x %2", currentScreen()->pixelSize(highestIndex).width(), currentScreen()->pixelSize(highestIndex).height()));

		if (currentScreen()->proposedSize() == highestIndex)
			menu->setItemChecked(lastIndex, true);

		menu->setItemParameter(lastIndex, highestIndex);
		menu->connectItem(lastIndex, this, SLOT(slotResolutionChanged(int)));
	}
	delete [] sizeSort;
	sizeSort = 0L;

	// Don't display the rotation options if there is no point (ie. none are supported)
	// XFree86 4.3 does not include rotation support.
	if (currentScreen()->rotations() != RandRScreen::Rotate0) {
		menu->addTitle(SmallIcon("reload"), i18n("Orientation"));

		for (int i = 0; i < 6; i++) {
			if ((1 << i) & currentScreen()->rotations()) {
				lastIndex = menu->insertItem(QIcon(currentScreen()->rotationIcon(1 << i)), RandRScreen::rotationName(1 << i));

				if (currentScreen()->proposedRotation() & (1 << i))
					menu->setItemChecked(lastIndex, true);

				menu->setItemParameter(lastIndex, 1 << i);
				menu->connectItem(lastIndex, this, SLOT(slotOrientationChanged(int)));
			}
		}
	}

	QStringList rr = currentScreen()->refreshRates(currentScreen()->proposedSize());

	if (rr.count())
		menu->addTitle(SmallIcon("clock"), i18n("Refresh Rate"));

	int i = 0;
	for (QStringList::Iterator it = rr.begin(); it != rr.end(); ++it, i++) {
		lastIndex = menu->insertItem(*it);

		if (currentScreen()->proposedRefreshRate() == i)
			menu->setItemChecked(lastIndex, true);

		menu->setItemParameter(lastIndex, i);
		menu->connectItem(lastIndex, this, SLOT(slotRefreshRateChanged(int)));
	}
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
	KCMultiDialog *kcm = new KCMultiDialog( parentWidget() );
	kcm->setFaceType( KCMultiDialog::Plain );
	kcm->setPlainCaption( i18n( "Configure Display" ) );
	kcm->addModule( "display" );
	kcm->exec();
}
