/*
 * Copyright (c) 2007 Gustavo Pichorim Boiko <gustavo.boiko@kdemail.net>
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

#include "legacyrandrconfig.h"

#include <QRadioButton>
#include "randrdisplay.h"
#include "legacyrandrscreen.h"

LegacyRandRConfig::LegacyRandRConfig(QWidget *parent, RandRDisplay *display)
: QWidget(parent), Ui::LegacyRandRConfigBase()
{
	setupUi(this);
	m_display = display;
	Q_ASSERT(m_display);

	if (!m_display->isValid())
		return;

	for (int s = 0; s < m_display->numScreens(); ++s)
		screenCombo->addItem(i18n("Screen %1", s+1));
	screenCombo->setCurrentIndex(m_display->currentScreenIndex());

	if (m_display->numScreens() <= 1)
		screenCombo->setEnabled(false);

	new QGridLayout(rotationGroup);
	// Create rotations
	for (int i = 0; i < RandR::OrientationCount; i++)
		addRotationButton(1 << i, i > RandR::RotationCount - 1);


	connect(screenCombo, SIGNAL(activated(int)), SLOT(slotScreenChanged(int)));
	connect(sizeCombo, SIGNAL(activated(int)), SLOT(slotSizeChanged(int)));
	connect(rateCombo, SIGNAL(activated(int)), SLOT(slotRefreshChanged(int)));
	connect(applyOnStartup, SIGNAL(clicked()), SLOT(setChanged()));
	connect(syncTrayApp, SIGNAL(clicked()), SLOT(setChanged()));

	load();
	syncTrayApp->setEnabled(applyOnStartup->isChecked());

	slotScreenChanged(m_display->currentScreenIndex());
}

LegacyRandRConfig::~LegacyRandRConfig()
{
}

void LegacyRandRConfig::load()
{
	if (!m_display->isValid())
		return;

	// Don't load screen configurations:
	// It will be correct already if they wanted to retain their settings over KDE restarts,
	// and if it isn't correct they have changed a) their X configuration, b) the screen
	// with another program, or c) their hardware.

	KConfig config("krandrrc");
	m_oldApply = m_display->loadDisplay(config, false);
	m_oldSyncTrayApp = m_display->syncTrayApp(config);
	applyOnStartup->setChecked(m_oldApply);
	syncTrayApp->setChecked(m_oldSyncTrayApp);

	setChanged();
}

void LegacyRandRConfig::save()
{
	if (!m_display->isValid())
		return;

	apply();

	m_oldApply = applyOnStartup->isChecked();
	m_oldSyncTrayApp = syncTrayApp->isChecked();
	KConfig config("krandrrc");
	m_display->saveDisplay(config, m_oldApply, m_oldSyncTrayApp);

	setChanged();
}

void LegacyRandRConfig::defaults()
{
	LegacyRandRScreen *screen = m_display->currentLegacyScreen();
	Q_ASSERT(screen);

	if (screen->changedFromOriginal()) {
		screen->proposeOriginal();
		screen->applyProposed();
	} else {
		screen->proposeOriginal();
	}

	update();
}

void LegacyRandRConfig::slotScreenChanged(int screenId)
{	
	m_display->setCurrentScreen(screenId);

	// Clear resolutions
	sizeCombo->clear();

	LegacyRandRScreen *screen = m_display->currentLegacyScreen();
	Q_ASSERT(screen);

	// Add new resolutions
	for (int i = 0; i < screen->numSizes(); i++) {
		sizeCombo->addItem(QString("%1 x %2").arg(screen->pixelSize(i).width()).arg(screen->pixelSize(i).height()));

		// Aspect ratio
		/* , aspect ratio %5)*/
		/*.arg((double)currentScreen()->size(i).mwidth / (double)currentScreen()->size(i).mheight))*/
	}

	// configure the possible rotations
	for (int i = 0; i < RandR::OrientationCount; i++)
		m_rotationGroup.button(1 << i)->setEnabled( (1 << i) & screen->rotations());

	m_rotationGroup.button(screen->rotation())->setChecked(true);
	populateRefreshRates();

	update();

	setChanged();
}
        
void LegacyRandRConfig::slotRotationChanged()
{
	LegacyRandRScreen *screen = m_display->currentLegacyScreen();
	Q_ASSERT(screen);

	//FIXME: need to check this later
	int id = m_rotationGroup.checkedId();
	
	screen->proposeRotation(id);	
	setChanged();
}

void LegacyRandRConfig::slotSizeChanged(int index)
{
	LegacyRandRScreen *screen = m_display->currentLegacyScreen();
	Q_ASSERT(screen);

	int oldProposed = screen->proposedSize();

	screen->proposeSize(index);

	if (screen->proposedSize() != oldProposed) {
		screen->proposeRefreshRate(0);

		populateRefreshRates();

		// Item with index zero is already selected
	}

	setChanged();
}

void LegacyRandRConfig::slotRefreshChanged(int index)
{
	LegacyRandRScreen *screen = m_display->currentLegacyScreen();
	Q_ASSERT(screen);

	screen->proposeRefreshRate(index);

	setChanged();
}

void LegacyRandRConfig::setChanged()
{
	bool isChanged = (m_oldApply != applyOnStartup->isChecked()) || (m_oldSyncTrayApp != syncTrayApp->isChecked());
	syncTrayApp->setEnabled(applyOnStartup->isChecked());

	if (!isChanged)
		for (int screenIndex = 0; screenIndex < m_display->numScreens(); screenIndex++) {
			if (m_display->legacyScreen(screenIndex)->proposedChanged()) {
				isChanged = true;
				break;
			}
		}

	if (isChanged != m_changed) {
		m_changed = isChanged;
		emit changed(m_changed);
	}
}

void LegacyRandRConfig::apply()
{
	if (m_changed) {
		m_display->applyProposed();

		update();
	}
}

void LegacyRandRConfig::update()
{
	LegacyRandRScreen *screen = m_display->currentLegacyScreen();
	Q_ASSERT(screen);

	sizeCombo->blockSignals(true);
	sizeCombo->setCurrentIndex(screen->proposedSize());
	sizeCombo->blockSignals(false);

	m_rotationGroup.blockSignals(true);
	m_rotationGroup.button(screen->proposedRotation())->setChecked(true);
	
	//m_rotationGroup.button(4)->setDown(screen->proposedRotation() & RandR::ReflectX);
	//m_rotationGroup.button(5)->setDown(screen->proposedRotation() & RandR::ReflectY);
	m_rotationGroup.blockSignals(false);

	rateCombo->blockSignals(true);
	rateCombo->setCurrentIndex(screen->proposedRefreshRate());
	rateCombo->blockSignals(false);
}

void LegacyRandRConfig::addRotationButton(int thisRotation, bool checkbox)
{
	LegacyRandRScreen *screen = m_display->currentLegacyScreen();
	Q_ASSERT(screen);

	if (!checkbox) {
		QRadioButton* thisButton = new QRadioButton(RandR::rotationName(thisRotation), rotationGroup);
		 m_rotationGroup.addButton( thisButton, thisRotation );
		thisButton->setEnabled(thisRotation & screen->rotations());
		connect(thisButton, SIGNAL(clicked()), SLOT(slotRotationChanged()));
		rotationGroup->layout()->addWidget(thisButton);
	} else {
		QCheckBox* thisButton = new QCheckBox(RandR::rotationName(thisRotation), rotationGroup);
		m_rotationGroup.addButton( thisButton, thisRotation );
		thisButton->setEnabled(thisRotation & screen->rotations());
		connect(thisButton, SIGNAL(clicked()), SLOT(slotRotationChanged()));
		rotationGroup->layout()->addWidget(thisButton);
	}
}

void LegacyRandRConfig::populateRefreshRates()
{
	LegacyRandRScreen *screen = m_display->currentLegacyScreen();
	Q_ASSERT(screen);

	rateCombo->clear();

	RateList rr = screen->refreshRates(screen->proposedSize());

	rateCombo->setEnabled(rr.count());

	foreach(float rate, rr)
	{
		rateCombo->addItem(ki18n("%1 Hz").subs(rate, 0, 'f', 1).toString(), rate);
	}
}

#include "legacyrandrconfig.moc"
