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

#include <KActionCollection>
#include <KApplication>
#include <KCMultiDialog>
#include <KComponentData>
#include <KHelpMenu>
#include <KIcon>
#include <KIconLoader>
#include <KLocale>
#include <KMenu>
#include <KAction>
#include <QMouseEvent>
#include <QVariant>

#include <config-randr.h>
#include "krandrtray.h"
#include "krandrpassivepopup.h"
#include "krandrtray.moc"
#include "legacyrandrscreen.h"
#ifdef HAS_RANDR_1_2
#include "randrscreen.h"
#include "randroutput.h"
#endif
#include "randrmode.h"

KRandRSystemTray::KRandRSystemTray(RandRDisplay *dpy, QWidget* parent)
: KSystemTrayIcon("randr", parent)
, m_help(new KHelpMenu(parent, KGlobal::mainComponent().aboutData(), false, actionCollection()))
, m_popupUp(false), m_display(dpy)
{
	setIcon(KSystemTrayIcon::loadIcon("randr"));
	connect(this, SIGNAL(quitSelected()), kapp, SLOT(quit()));
	this->setToolTip( i18n("Screen resize & rotate"));

	m_menu = new KMenu(parentWidget());
	setContextMenu(m_menu);

	connect(m_menu, SIGNAL(aboutToShow()), this, SLOT(slotPrepareMenu()));
}

void KRandRSystemTray::slotPrepareMenu()
{
	QAction *action;

	m_menu->clear();

	if (!m_display->isValid()) 
	{
		action = m_menu->addAction(i18n("Required X Extension Not Available"));
		action->setEnabled(false);

	} 
	else 
	{
		m_screenPopups.clear();
		for (int s = 0; s < m_display->numScreens(); s++) 
		{
			m_display->setCurrentScreen(s);
			if (s == m_display->screenIndexOfWidget(parentWidget())) 
			{
				/*lastIndex = menu->insertItem(i18n("Screen %1").arg(s+1));
				menu->setItemEnabled(lastIndex, false);*/
			} 
			else 
			{
#ifdef HAS_RANDR_1_2
			    if (RandR::has_1_2)
				    m_display->screen(s)->loadSettings(true);
#endif
					
                            KMenu* subMenu = new KMenu(i18n("Screen %1", s+1), m_menu );
                            subMenu->setObjectName( QString("screen%1").arg(s+1) );
                            m_screenPopups.append(subMenu);
                            populateMenu(subMenu);
                            action = m_menu->addMenu(subMenu);
                            connect(subMenu, SIGNAL(activated(int)), SLOT(slotScreenActivated()));
			}
		}

		m_display->setCurrentScreen(m_display->screenIndexOfWidget(parentWidget()));
#ifdef HAS_RANDR_1_2
		if (RandR::has_1_2)
			m_display->currentScreen()->loadSettings(true);
#endif
		populateMenu(m_menu);
	}

	m_menu->addSeparator();

	KAction *actPrefs = actionCollection()->addAction( QString() );
        actPrefs->setIcon( KIcon( "configure" ) );
        actPrefs->setText( i18n( "Configure Display..." ) );
        connect( actPrefs, SIGNAL( triggered( bool ) ), SLOT( slotPrefs() ) );
	m_menu->addAction( actPrefs );

	m_menu->addMenu(/*SmallIcon("help-contents"),KStandardGuiItem::help().text(),*/ m_help->menu());
	QAction *quitAction = actionCollection()->action(KStandardAction::name(KStandardAction::Quit));
	m_menu->addAction( quitAction );
}

void KRandRSystemTray::slotScreenActivated()
{
	m_display->setCurrentScreen(m_screenPopups.indexOf(static_cast<KMenu*>(sender())));
}

void KRandRSystemTray::configChanged()
{
	m_display->refresh();

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
			message = m_display->currentLegacyScreen()->changedMessage();

		KRandrPassivePopup::message(
		i18n("Screen configuration has changed"),
		message, SmallIcon("view-fullscreen"),
		parentWidget());
	}

	first = false;
}

void KRandRSystemTray::populateMenu(KMenu* menu)
{
#ifdef HAS_RANDR_1_2
	if (RandR::has_1_2) 
	{
		QAction *action;
		QActionGroup *actionGroup;

		OutputMap outputs = m_display->currentScreen()->outputs();
		if (outputs.count() <= 0)
			return;

		RandRScreen *screen = m_display->currentScreen();
		Q_ASSERT(screen);

		// if the outputs are unified, do not show output-specific size 
		// changing options in the tray.
		if (screen->outputsUnified() && screen->connectedCount() > 1)
		{
			SizeList sizes = screen->unifiedSizes();
			
			if (sizes.count())
			{
				// populate unified sizes
				QSize currentSize;
				currentSize = screen->rect().size();

				menu->addTitle(SmallIcon("view-fullscreen"), i18n("Screen Size"));
				actionGroup = populateSizes(menu, sizes, currentSize);	
				connect(actionGroup, SIGNAL(triggered(QAction*)), screen, SLOT(slotResizeUnified(QAction*)));

				// if the outputs are unified, we can rotate the screen on all outputs
				int rotations = screen->unifiedRotations();
				if (rotations != RandR::Rotate0)
				{
					menu->addTitle(SmallIcon("view-refresh"), i18n("Orientation"));
					int rotation = RandR::Rotate0;
					foreach(RandROutput *output, screen->outputs())
						if (output->isActive())
						{
							rotation = output->rotation();
							break;
						}

					actionGroup = populateRotations(menu, rotations, rotation);
					connect(actionGroup, SIGNAL(triggered(QAction*)), screen, SLOT(slotRotateUnified(QAction*)));
				}
			}
		}
		else
		{
			if (screen->connectedCount() != 1)
				menu->addTitle(SmallIcon("view-fullscreen"), i18n("Outputs"));

			foreach(RandROutput *output, outputs)
			{
				if (output->isConnected()) 
				{
					KMenu *outputMenu;
					if (screen->connectedCount() == 1)
						outputMenu = menu;
					else
						outputMenu = new KMenu(output->name());
					outputMenu->setIcon(SmallIcon(output->icon()));
					outputMenu->addTitle(SmallIcon("view-fullscreen"), i18n("%1 - Screen Size", output->name()));

					QSize currentSize = output->rect().size();

					// if the output is rotated 90 or 270, the returned rect is inverted
					// so we need to invert the size before comparing
					if (output->rotation() & (RandR::Rotate90 | RandR::Rotate270))
						currentSize = QSize(currentSize.height(), currentSize.width());

					actionGroup = populateSizes(outputMenu, output->sizes(), currentSize);
					connect(actionGroup, SIGNAL(triggered(QAction*)), output, SLOT(slotChangeSize(QAction*)));
					
					// if there is only one output active, do not show the disable option
					// this prevents the user from doing wrong things ;)
					if (screen->activeCount() != 1)
					{
						action = outputMenu->addAction(i18n("Disable"));
						if (output->crtc() == None)
						{
							QFont font = action->font();
							font.setBold(true);
							action->setFont(font);
						}
						connect(action, SIGNAL(triggered(bool)), output, SLOT(slotDisable()));
					}

					// Display the rotations
					int rotations = output->rotations();
					// Don't display the rotation options if there is no point (ie. none are supported)
					// XFree86 4.3 does not include rotation support.
					if (rotations != RandR::Rotate0) 
					{
						outputMenu->addTitle(SmallIcon("view-refresh"), i18n("Orientation"));
						actionGroup = populateRotations(outputMenu, rotations, output->rotation());
						connect(actionGroup, SIGNAL(triggered(QAction*)), 
							output, SLOT(slotChangeRotation(QAction*)));
					}

					// refresh rate
					RateList rates = output->refreshRates();
					if (rates.count())
					{
						outputMenu->addTitle(SmallIcon("chronometer"), i18n("Refresh Rate"));
						actionGroup = populateRates(outputMenu, rates, output->refreshRate());
						connect(actionGroup, SIGNAL(triggered(QAction*)), 
							output, SLOT(slotChangeRefreshRate(QAction*)));
					}
					
					if (screen->connectedCount() != 1)
						menu->addMenu(outputMenu);
				} 
			}
		}
		// if there is more than one output connected, give the option to unify the outputs
		if (screen->connectedCount() != 1)
		{
			menu->addSeparator();
			action = menu->addAction( i18n("Unify Outputs"), screen, SLOT(slotUnifyOutputs(bool)) );
			action->setCheckable(true);
			action->setChecked(screen->outputsUnified());
		}
	}
	else
#endif
		populateLegacyMenu(menu);
}

void KRandRSystemTray::populateLegacyMenu(KMenu* menu)
{
	QAction *action;

	menu->addTitle(SmallIcon("view-fullscreen"), i18n("Screen Size"));

	LegacyRandRScreen *screen = m_display->currentLegacyScreen();
	Q_ASSERT(screen);

	// add sizes
	QActionGroup *screenSizeGroup = populateSizes(menu, RandR::sortSizes(screen->pixelSizes()), screen->currentPixelSize());
	connect(screenSizeGroup, SIGNAL(triggered(QAction*)), SLOT(slotResolutionChanged(QAction*)));

	// Don't display the rotation options if there is no point (ie. none are supported)
	// XFree86 4.3 does not include rotation support.
	if (screen->rotations() != RandR::Rotate0) 
	{
		menu->addTitle(SmallIcon("view-refresh"), i18n("Orientation"));

		QActionGroup *rotationGroup = populateRotations(menu, screen->rotations(), screen->rotation());
		connect(rotationGroup, SIGNAL(triggered(QAction*)), SLOT(slotOrientationChanged(QAction*)));
	}

	RateList rr = screen->refreshRates(screen->proposedSize());
	if (rr.count())
	{
		menu->addTitle(SmallIcon("clock"), i18n("Refresh Rate"));

		QActionGroup *rateGroup = populateRates(menu, rr, screen->refreshRate());
		connect(rateGroup, SIGNAL(triggered(QAction*)), SLOT(slotRefreshRateChanged(QAction*)));
	}
}

QActionGroup *KRandRSystemTray::populateRotations(KMenu *menu, int rotations, int rotation)
{
	QAction *action;
	QActionGroup *rotateGroup = new QActionGroup(menu);


	for (int i = 0; i < 6; i++) 
	{
		if ((1 << i) & rotations) 
		{
			action = menu->addAction(QIcon(RandR::rotationIcon(1 << i, rotation)), 
							   RandR::rotationName(1 << i));

			action->setData(1 << i);
			if (rotation & (1 << i))
			{
				QFont font = action->font();
				font.setBold(true);
				action->setFont(font);
			}
			rotateGroup->addAction(action);
		}
	}
	return rotateGroup;
}

QActionGroup *KRandRSystemTray::populateSizes(KMenu *menu, const SizeList &sizes, const QSize &size)
{
	QAction *action;
	QActionGroup *sizeGroup = new QActionGroup(menu);
	for (int i = 0; i < sizes.count(); ++i) 
	{
		QSize s = sizes[i];
		action = menu->addAction(QString("%1 x %2").arg(s.width()).arg(s.height()));
		action->setData(s);
		if (s == size) 
		{
			QFont font = action->font();
			font.setBold(true);
			action->setFont(font);
		}
		sizeGroup->addAction(action);
	}
	return sizeGroup;
}

QActionGroup *KRandRSystemTray::populateRates(KMenu *menu, const RateList &rates, float rate)
{
	QAction *action;
	QActionGroup *rateGroup = new QActionGroup(menu);

	foreach(float r, rates)
	{
		action = menu->addAction(i18n("%1 Hz", QString::number(r, 'f', 1)));
		action->setData(r);
		if (r == rate)
		{
			QFont f = action->font();
			f.setBold(true);
			action->setFont(f);
		}
		rateGroup->addAction(action);
	}
	return rateGroup;
}

void KRandRSystemTray::slotResolutionChanged(QAction *action)
{
	LegacyRandRScreen *screen = m_display->currentLegacyScreen();
	Q_ASSERT(screen);

	QSize s = action->data().toSize();
	int index = 0;
	SizeList pixelSizes = screen->pixelSizes();
	for (int i = 0; i < pixelSizes.count(); ++i)
		if (pixelSizes[i] == s)
		{
			index = i;
			break;
		}

	if (screen->size() == index)
		return;

	screen->proposeSize(index);

	screen->proposeRefreshRate(-1);

	if (screen->applyProposedAndConfirm()) 
	{
		KConfig config("krandrrc");
		if (m_display->syncTrayApp(config))
			screen->save(config);
	}
}

void KRandRSystemTray::slotOrientationChanged(QAction *action)
{
	LegacyRandRScreen *screen = m_display->currentLegacyScreen();
	Q_ASSERT(screen);
	
	int propose = screen->rotation();
	int rotate = action->data().toInt();

	if (rotate & RandR::RotateMask)
		propose &= RandR::ReflectMask;

	propose ^= rotate;

	if (screen->rotation() == propose)
		return;

	screen->proposeRotation(propose);

	if (screen->applyProposedAndConfirm()) 
	{
		KConfig config("krandrrc");
		if (m_display->syncTrayApp(config))
			screen->save(config);
	}
}

void KRandRSystemTray::slotRefreshRateChanged(QAction *action)
{
	LegacyRandRScreen *screen = m_display->currentLegacyScreen();
	Q_ASSERT(screen);
	
	int index = action->data().toInt();

	if (screen->refreshRate() == index)
		return;

	screen->proposeRefreshRate(index);

	if (screen->applyProposedAndConfirm()) 
	{
		KConfig config("krandrrc");
		if (m_display->syncTrayApp(config))
			screen->save(config);
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
