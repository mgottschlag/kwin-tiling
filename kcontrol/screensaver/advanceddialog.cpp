#include <klocale.h>
#include <kstandarddirs.h>
#include <qcombobox.h>
#include <kdebug.h>

#include <qtooltip.h>
#include <qstring.h>

#include "advanceddialog.h"
#include "advanceddialogimpl.h"
#include "stdlib.h"

KScreenSaverAdvancedDialog::KScreenSaverAdvancedDialog(QWidget *parent, const char* name)
 : KDialogBase( parent, name, true, i18n( "Advanced Options" ),
                Ok | Cancel, Ok, true )
{
	readSettings();

	dialog = new AdvancedDialog(this);
	setMainWidget(dialog);

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
    dialog->qcbPriority->setEnabled(false);
#endif
}


void KScreenSaverAdvancedDialog::readSettings()
{
    KConfig *config = new KConfig( "kdesktoprc");
    config->setGroup( "ScreenSaver" );

    mPriority = config->readNumEntry("Priority", 19);
    if (mPriority < 0) mPriority = 0;
    if (mPriority > 19) mPriority = 19;

    
       dialog->setTopLeftMode(2);
//    dialog->setMode(dialog->qcbPriority, 0);
    //int foo = config->readNumEntry("ActionTopLeft", 0);
    // kdDebug() << "setting active " << dialog->qcbTopLeft->currentItem() << endl;
//     dialog->qcbTopLeft->setCurrentItem(foo);
//     dialog->qcbTopRight->setCurrentItem(config->readNumEntry("ActionTopRight", 0));
//     dialog->qcbBottomLeft->setCurrentItem(config->readNumEntry("ActionBottomLeft", 0));
//     dialog->qcbBottomRight->setCurrentItem(config->readNumEntry("ActionBottomRight", 0));*/
    mChanged = false;
    delete config;
}

void KScreenSaverAdvancedDialog::slotPriorityChanged(int val)
{
	switch (val)
	{
		case 1: // Low
			mPriority = 19;
			kdDebug() << "low priority" << endl;
			break;
		case 2: // Medium
			mPriority = 10;
			kdDebug() << "medium priority" << endl;
			break;
		case 3: // High
			mPriority = 0;
			kdDebug() << "high priority" << endl;
			break;
	}
	mChanged = true;
}

void KScreenSaverAdvancedDialog::slotOk()
{
    if (mChanged) {
        KConfig *config = new KConfig("kdesktoprc");
        config->setGroup( "ScreenSaver" );

        config->writeEntry("Priority", mPriority);
        /*config->writeEntry(
            "ActionTopLeft", dialog->qcbTopLeft->currentItem());
        config->writeEntry(
            "ActionBottomLeft", dialog->qcbTopRight->currentItem());
        config->writeEntry(
            "ActionTopRight", dialog->qcbBottomLeft->currentItem());
        config->writeEntry(
            "ActionBottomRight", dialog->qcbBottomRight->currentItem());
        config->sync();*/
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
	qcbTopLeft->setCurrentItem(2);
	monitorLabel->setPixmap(QPixmap(locate("data", "kcontrol/pics/monitor.png")));
	QToolTip::add(qcbPriority, "Specify the priority that the screensaver will run at. A higher priority may mean that the screensaver runs more slowly, though may reduce the speed that other programs run at while the screensacer is active.");
}

AdvancedDialog::~AdvancedDialog()
{
 
}

/*FIXME -- this should work but crashes so needs valgrinding on an x86 box
void AdvancedDialog::setMode(QComboBox *box, int i)
{
	box->setCurrentItem(i);
}

int AdvancedDialog::mode(QComboBox *box)
{
	return box->currentItem();
}*/


void AdvancedDialog::setTopLeftMode(int i)
{
	kdDebug() << "setting mode to " << i << endl;
	abort();
	qcbTopLeft->setCurrentItem(i);
}

void AdvancedDialog::setTopRightMode(int i)
{
	qcbTopRight->setCurrentItem(i);
}

void AdvancedDialog::setBottomLeftMode(int i)
{
	qcbBottomLeft->setCurrentItem(i);
}

void AdvancedDialog::setBottomRightMode(int i)
{
	qcbBottomRight->setCurrentItem(i);
}

int AdvancedDialog::topLeftMode()
{
	return qcbTopLeft->currentItem();
}

int AdvancedDialog::topRightMode()
{
	return qcbTopLeft->currentItem();
}

int AdvancedDialog::bottomLeftMode()
{
	return qcbTopLeft->currentItem();
}

int AdvancedDialog::bottomRightMode()
{
	return qcbTopLeft->currentItem();
}
