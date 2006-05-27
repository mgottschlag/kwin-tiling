#include <klocale.h>
#include <kstandarddirs.h>
#include <QComboBox>
//Added by qt3to4:
#include <QPixmap>
#include <kdebug.h>

#include <QString>

#include <config.h>

#include "advanceddialog.h"
#include "advanceddialogimpl.h"
#include "stdlib.h"

#include "advanceddialog.moc"

KScreenSaverAdvancedDialog::KScreenSaverAdvancedDialog(QWidget *parent, const char* name)
 : KDialogBase( parent, name, true, i18n( "Advanced Options" ),
                Ok | Cancel, Ok, true )
{
	
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
	KConfig *config = new KConfig("kdesktoprc");
	config->setGroup("ScreenSaver");
	
	mPriority = config->readEntry("Priority", 19);
	if (mPriority < 0) mPriority = 0;
	if (mPriority > 19) mPriority = 19;
	
	dialog->qcbTopLeft->setCurrentIndex(config->readEntry("ActionTopLeft", 0));    
	dialog->qcbTopRight->setCurrentIndex(config->readEntry("ActionTopRight", 0));
	dialog->qcbBottomLeft->setCurrentIndex(config->readEntry("ActionBottomLeft", 0));
	dialog->qcbBottomRight->setCurrentIndex(config->readEntry("ActionBottomRight", 0));

	switch(mPriority)
	{
		case 19: // Low
			dialog->qcbPriority->setCurrentItem(0);
			kDebug() << "setting low" << endl;
			break;
		case 10: // Medium
			dialog->qcbPriority->setCurrentItem(1);
			kDebug() << "setting medium" << endl;
			break;
		case 0: // High
			dialog->qcbPriority->setCurrentItem(2);
			kDebug() << "setting high" << endl;
			break;
	}

	mChanged = false;
	delete config;
}

void KScreenSaverAdvancedDialog::slotPriorityChanged(int val)
{
	switch (val)
	{
		case 0: // Low
			mPriority = 19;
			kDebug() << "low priority" << endl;
			break;
		case 1: // Medium
			mPriority = 10;
			kDebug() << "medium priority" << endl;
			break;
		case 2: // High
			mPriority = 0;
			kDebug() << "high priority" << endl;
			break;
	}
	mChanged = true;
}

void KScreenSaverAdvancedDialog::slotOk()
{
	if (mChanged)
	{
		KConfig *config = new KConfig("kdesktoprc");
		config->setGroup( "ScreenSaver" );
	
		config->writeEntry("Priority", mPriority);
		config->writeEntry(
		"ActionTopLeft", dialog->qcbTopLeft->currentIndex());
		config->writeEntry(
		"ActionTopRight", dialog->qcbTopRight->currentIndex());
		config->writeEntry(
		"ActionBottomLeft", dialog->qcbBottomLeft->currentIndex());
		config->writeEntry(
		"ActionBottomRight", dialog->qcbBottomRight->currentIndex());
		config->sync();
		delete config;
	}
	accept();
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

AdvancedDialog::AdvancedDialog(QWidget *parent, const char *name) : AdvancedDialogImpl(parent, name)
{
	monitorLabel->setPixmap(QPixmap(locate("data", "kcontrol/pics/monitor.png")));
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
