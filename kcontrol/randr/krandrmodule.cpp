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
 
#include <qbuttongroup.h>
#include <qlabel.h>
#include <qradiobutton.h>
#include <qlayout.h>
#include <qvbuttongroup.h>
#include <qcheckbox.h>
#include <qhbox.h>

#include <kdebug.h>
#include <klocale.h>
#include <kcmodule.h>
#include <kglobal.h>
#include <kgenericfactory.h>
#include <kcombobox.h>
#include <kactivelabel.h>

#include "krandrmodule.h"
#include "krandrmodule.moc"

// DLL Interface for kcontrol
typedef KGenericFactory<KRandRModule, QWidget > KSSFactory;
K_EXPORT_COMPONENT_FACTORY (libkcm_randr, KSSFactory("randr") );


extern "C"
{
	void init_randr()
	{
		// Load settings and apply appropriate config
		RandRDisplay* display = new RandRDisplay();
		KConfig randrConfig("kcmrandrrc", true);
		if (display->loadDisplay(randrConfig))
			display->applyProposed(false);
		delete display;
	}
}


KRandRModule::KRandRModule(QWidget *parent, const char *name, const QStringList&)
    : KCModule(parent, name)
	, m_changed(false)
{
	if (!isValid()) {
		QVBoxLayout *topLayout = new QVBoxLayout(this);
		topLayout->addWidget(new KActiveLabel(i18n("Sorry, your X server does not support resizing and rotating the display. Please update to version 4.3 or greater.  You need the X Resize And Rotate extension (RANDR) version 1.1 or greater to use this feature.  Error code: %1").arg(errorCode()), this));
		return;
	}

	QVBoxLayout *topLayout = new QVBoxLayout(this);
	topLayout->setAutoAdd(true);

	QHBox* screenBox = new QHBox(this);
	new QLabel(i18n("Settings for screen:"), screenBox);
	m_screenSelector = new KComboBox(screenBox);

	for (int s = 0; s < numScreens(); s++) {
		m_screenSelector->insertItem(i18n("Screen %1").arg(s+1));
	}

	m_screenSelector->setCurrentItem(currentScreenIndex());

	connect(m_screenSelector, SIGNAL(activated(int)), SLOT(slotScreenChanged(int)));

	if (numScreens() <= 1)
		m_screenSelector->setEnabled(false);

	QHBox* sizeBox = new QHBox(this);
	new QLabel(i18n("Screen size:"), sizeBox);
	m_sizeCombo = new KComboBox(sizeBox);
	connect(m_sizeCombo, SIGNAL(activated(int)), SLOT(slotSizeChanged(int)));

	m_rotationGroup = new QButtonGroup(2, Qt::Horizontal, i18n("Orientation (degrees anticlockwise)"), this);
	m_rotationGroup->setRadioButtonExclusive(true);

	QHBox* refreshBox = new QHBox(this);
	new QLabel(i18n("Refresh rate:"), refreshBox);
	m_refreshRates = new KComboBox(refreshBox);
	connect(m_refreshRates, SIGNAL(activated(int)), SLOT(slotRefreshChanged(int)));

	m_applyOnStartup = new QCheckBox(i18n("Apply settings on KDE startup"), this);
	connect(m_applyOnStartup, SIGNAL(clicked()), SLOT(setChanged()));

	slotScreenChanged(DefaultScreen(qt_xdisplay()));

	layout()->addItem(new QSpacerItem(0,0));

	setButtons(KCModule::Apply);

	// just set the "apply settings on startup" box
	load();
}

void KRandRModule::addRotationButton(int thisRotation, bool checkbox)
{
	Q_ASSERT(m_rotationGroup);
	if (!checkbox) {
		QRadioButton* thisButton = new QRadioButton(RandRScreen::rotationName(thisRotation), m_rotationGroup);
		thisButton->setEnabled(thisRotation & currentScreen()->rotations());
		connect(thisButton, SIGNAL(clicked()), SLOT(slotRotationChanged()));
	} else {
		QCheckBox* thisButton = new QCheckBox(RandRScreen::rotationName(thisRotation), m_rotationGroup);
		thisButton->setEnabled(thisRotation & currentScreen()->rotations());
		connect(thisButton, SIGNAL(clicked()), SLOT(slotRotationChanged()));
	}
}

void KRandRModule::slotScreenChanged(int screen)
{
	setCurrentScreen(screen);

	// Clear resolutions
	m_sizeCombo->clear();

	// Add new resolutions
	for (int i = 0; i < currentScreen()->numSizes(); i++) {
		m_sizeCombo->insertItem(i18n("%1 x %2 (%3mm x %4mm)").arg(currentScreen()->size(i).width).arg(currentScreen()->size(i).height).arg(currentScreen()->size(i).mwidth).arg(currentScreen()->size(i).mheight));

		// Aspect ratio
		/* , aspect ratio %5)*/
		/*.arg((double)currentScreen()->size(i).mwidth / (double)currentScreen()->size(i).mheight))*/
	}

	// Clear rotations
	for (int i = m_rotationGroup->count() - 1; i >= 0; i--) {
		m_rotationGroup->remove(m_rotationGroup->find(i));
	}
	
	// Create rotations
	for (int i = 0; i < 6; i++)
		addRotationButton(1 << i, i > 3);

	populateRefreshRates();
	
	update();
	
	setChanged();
}

void KRandRModule::slotRotationChanged()
{
	if (m_rotationGroup->find(0)->isOn())
		currentScreen()->proposeRotation(RR_Rotate_0);
	else if (m_rotationGroup->find(1)->isOn())
		currentScreen()->proposeRotation(RR_Rotate_90);
	else if (m_rotationGroup->find(2)->isOn())
		currentScreen()->proposeRotation(RR_Rotate_180);
	else {
		Q_ASSERT(m_rotationGroup->find(3)->isOn());
		currentScreen()->proposeRotation(RR_Rotate_270);
	}

	if (m_rotationGroup->find(4)->isOn())
		currentScreen()->proposeRotation(RR_Reflect_X);

	if (m_rotationGroup->find(5)->isOn())
		currentScreen()->proposeRotation(RR_Reflect_Y);

	setChanged();
}

void KRandRModule::slotSizeChanged(int index)
{
	int oldProposed = currentScreen()->proposedSize();

	currentScreen()->proposeSize(static_cast<SizeID>(index));

	if (currentScreen()->proposedSize() != oldProposed) {
		currentScreen()->proposeRefreshRate(0);

		populateRefreshRates();

		// Item with index zero is already selected
	}

	setChanged();
}

void KRandRModule::slotRefreshChanged(int index)
{
	currentScreen()->proposeRefreshRate(index);

	setChanged();
}

void KRandRModule::populateRefreshRates()
{
	m_refreshRates->clear();

	QStringList rr = currentScreen()->refreshRates(currentScreen()->proposedSize());

	for (QStringList::Iterator it = rr.begin(); it != rr.end(); it++) {
		m_refreshRates->insertItem(*it);
	}

	if (rr.count() < 2)
		m_refreshRates->setEnabled(false);
	else
		m_refreshRates->setEnabled(true);
}


void KRandRModule::defaults()
{
	if (currentScreen()->changedFromOriginal()) {
		currentScreen()->proposeOriginal();
		currentScreen()->applyProposed();
	} else {
		currentScreen()->proposeOriginal();
	}

	update();
}

void KRandRModule::load()
{
	KConfig config("kcmrandrrc");

	// Don't load screen configurations:
	// It will be correct already if they wanted to retain their settings over KDE restarts,
	// and if it isn't correct they have changed a) their X configuration or b) the screen
	// with another program.
	m_oldApply = loadDisplay(config, false);
	m_applyOnStartup->setChecked(m_oldApply);

	setChanged();
}

void KRandRModule::save()
{
	KConfig config("kcmrandrrc");
	m_oldApply = m_applyOnStartup->isChecked();
	saveDisplay(config, m_oldApply);
	apply();
	setChanged();
}

void KRandRModule::setChanged()
{
	bool isChanged = (m_oldApply != m_applyOnStartup->isChecked());

	if (!isChanged)
		for (int screenIndex = 0; screenIndex < numScreens(); screenIndex++) {
			if (screen(screenIndex)->proposedChanged()) {
				isChanged = true;
				break;
			}
		}

	if (isChanged != m_changed) {
		m_changed = isChanged;
		emit changed(m_changed);
	}
}

void KRandRModule::apply()
{
	if (m_changed) {
		applyProposed();

		update();
	}
}


void KRandRModule::update()
{
	m_sizeCombo->blockSignals(true);
	m_sizeCombo->setCurrentItem(currentScreen()->proposedSize());
	m_sizeCombo->blockSignals(false);

	m_rotationGroup->blockSignals(true);
	switch (currentScreen()->proposedRotation() & 15) {
		case RR_Rotate_0:
			m_rotationGroup->setButton(0);
			break;
		case RR_Rotate_90:
			m_rotationGroup->setButton(1);
			break;
		case RR_Rotate_180:
			m_rotationGroup->setButton(2);
			break;
		case RR_Rotate_270:
			m_rotationGroup->setButton(3);
			break;
		default:
			// Shouldn't hit this one
			Q_ASSERT(currentScreen()->proposedRotation() & 15);
			break;
	}
	m_rotationGroup->find(4)->setDown(currentScreen()->proposedRotation() & RR_Reflect_X);
	m_rotationGroup->find(5)->setDown(currentScreen()->proposedRotation() & RR_Reflect_Y);
	m_rotationGroup->blockSignals(false);

	m_refreshRates->blockSignals(true);
	m_refreshRates->setCurrentItem(currentScreen()->proposedRefreshRate());
	m_refreshRates->blockSignals(false);
}
