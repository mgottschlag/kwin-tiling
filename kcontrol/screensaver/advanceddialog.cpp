/*
* advanceddialog.cpp
* Copyright 1997       Matthias Hoelzer
* Copyright 1996,1999,2002    Martin R. Jones
* Copyright 2004       Chris Howells
* Copyright 2007-2008  Benjamin Meyer <ben@meyerhome.net>
* Copyright 2007-2008  Hamish Rodda <rodda@kde.org>
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License as
* published by the Free Software Foundation; either version 2 of
* the License or (at your option) version 3 or any later version
* accepted by the membership of KDE e.V. (or its successor approved
* by the membership of KDE e.V.), which shall act as a proxy
* defined in Section 14 of version 3 of the license.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <config-workspace.h>

#include "advanceddialog.h"

#include <QComboBox>
#include <QPixmap>

#include <KLocale>
#include <KStandardDirs>
#include <KDebug>

#include "stdlib.h"

#include "advanceddialog.moc"

KScreenSaverAdvancedDialog::KScreenSaverAdvancedDialog(QWidget *parent)
 : KDialog( parent )
{
  setModal( true );
  setCaption( i18n( "Advanced Options" ) );
  setButtons( Ok | Cancel );
  showButtonSeparator( true );

	dialog = new AdvancedDialog(this);
	setMainWidget(dialog);

	readSettings();

	connect(dialog->qcbPriority, SIGNAL(activated(int)),
		this, SLOT(slotPriorityChanged(int)));

	connect(dialog->qcbTopLeft, SIGNAL(activated(int)),
		this, SLOT(slotChangeTopLeftCorner(int)));
	connect(dialog->qcbTopRight, SIGNAL(activated(int)),
		this, SLOT(slotChangeTopLeftCorner(int)));
	connect(dialog->qcbBottomLeft, SIGNAL(activated(int)),
		this, SLOT(slotChangeTopLeftCorner(int)));
	connect(dialog->qcbBottomRight, SIGNAL(activated(int)),
		this, SLOT(slotChangeTopLeftCorner(int)));

#ifndef HAVE_SETPRIORITY
    dialog->qgbPriority->setEnabled(false);
#endif
}

void KScreenSaverAdvancedDialog::readSettings()
{
        KConfigGroup config(KSharedConfig::openConfig("kscreensaverrc"), "ScreenSaver");

	mPriority = config.readEntry("Priority", 19);
	if (mPriority < 0) mPriority = 0;
	if (mPriority > 19) mPriority = 19;

	dialog->qcbTopLeft->setCurrentIndex(config.readEntry("ActionTopLeft", 0));
	dialog->qcbTopRight->setCurrentIndex(config.readEntry("ActionTopRight", 0));
	dialog->qcbBottomLeft->setCurrentIndex(config.readEntry("ActionBottomLeft", 0));
	dialog->qcbBottomRight->setCurrentIndex(config.readEntry("ActionBottomRight", 0));

	switch(mPriority)
	{
		case 19: // Low
			dialog->qcbPriority->setCurrentIndex(0);
			kDebug() << "setting low";
			break;
		case 10: // Medium
			dialog->qcbPriority->setCurrentIndex(1);
			kDebug() << "setting medium";
			break;
		case 0: // High
			dialog->qcbPriority->setCurrentIndex(2);
			kDebug() << "setting high";
			break;
	}

	mChanged = false;
}

void KScreenSaverAdvancedDialog::slotPriorityChanged(int val)
{
	switch (val)
	{
		case 0: // Low
			mPriority = 19;
			kDebug() << "low priority";
			break;
		case 1: // Medium
			mPriority = 10;
			kDebug() << "medium priority";
			break;
		case 2: // High
			mPriority = 0;
			kDebug() << "high priority";
			break;
	}
	mChanged = true;
}

void KScreenSaverAdvancedDialog::accept()
{
	if (mChanged)
  {
	KConfig *config = new KConfig("kscreensaverrc");
	KConfigGroup group= config->group("ScreenSaver");

 	group.writeEntry("Priority", mPriority);
  	group.writeEntry(
 		"ActionTopLeft", dialog->qcbTopLeft->currentIndex());
  	group.writeEntry(
 		"ActionTopRight", dialog->qcbTopRight->currentIndex());
  	group.writeEntry(
		"ActionBottomLeft", dialog->qcbBottomLeft->currentIndex());
  	group.writeEntry(
 		"ActionBottomRight", dialog->qcbBottomRight->currentIndex());
  	group.sync();
 	delete config;
 	}

  KDialog::accept();
}

void KScreenSaverAdvancedDialog::slotChangeBottomRightCorner(int)
{
	mChanged = true;
}

void KScreenSaverAdvancedDialog::slotChangeBottomLeftCorner(int)
{
	mChanged = true;
}

void KScreenSaverAdvancedDialog::slotChangeTopRightCorner(int)
{
	mChanged = true;
}

void KScreenSaverAdvancedDialog::slotChangeTopLeftCorner(int)
{
	mChanged = true;
}

/* =================================================================================================== */

AdvancedDialog::AdvancedDialog(QWidget *parent) : AdvancedDialogImpl(parent)
{
	monitorLabel->setPixmap(QPixmap(KStandardDirs::locate("data", "kcontrol/pics/monitor.png")));
	qcbPriority->setWhatsThis( "<qt>" + i18n("Specify the priority that the screensaver will run at. A higher priority may mean that the screensaver runs faster, though may reduce the speed that other programs run at while the screensaver is active.") + "</qt>");
	QString qsTopLeft("<qt>" +  i18n("The action to take when the mouse cursor is located in the top left corner of the screen for 15 seconds.") + "</qt>");
        QString qsTopRight("<qt>" +  i18n("The action to take when the mouse cursor is located in the top right corner of the screen for 15 seconds.") + "</qt>");
        QString qsBottomLeft("<qt>" +  i18n("The action to take when the mouse cursor is located in the bottom left corner of the screen for 15 seconds.") + "</qt>");
        QString qsBottomRight("<qt>" +  i18n("The action to take when the mouse cursor is located in the bottom right corner of the screen for 15 seconds.") + "</qt>");
	qlTopLeft->setWhatsThis( qsTopLeft);
	qcbTopLeft->setWhatsThis( qsTopLeft);
	qlTopRight->setWhatsThis( qsTopRight);
	qcbTopRight->setWhatsThis( qsTopRight);
	qlBottomLeft->setWhatsThis( qsBottomLeft);
	qcbBottomLeft->setWhatsThis( qsBottomLeft);
	qlBottomRight->setWhatsThis( qsBottomRight);
	qcbBottomRight->setWhatsThis( qsBottomRight);
}

AdvancedDialog::~AdvancedDialog()
{

}

void AdvancedDialog::setMode(QComboBox *box, int i)
{
	box->setCurrentIndex(i);
}

int AdvancedDialog::mode(QComboBox *box)
{
	return box->currentIndex();
}
