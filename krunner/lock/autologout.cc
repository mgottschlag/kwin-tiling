//===========================================================================
//
// This file is part of the KDE project
//
// Copyright (c) 2004 Chris Howells <howells@kde.org>

#include "lockprocess.h"
#include "autologout.h"

#include <kapplication.h>
#include <klocale.h>
#include <kglobalsettings.h>
#include <kconfig.h>
#include <kiconloader.h>
#include <kdebug.h>
#include <dcopref.h>
#include <kmessagebox.h>
#include <kdialog.h>

#include <qlayout.h>
#include <qmessagebox.h>
#include <qlabel.h>
#include <qstyle.h>
#include <qapplication.h>
#include <qdialog.h>

//FIXME don't hardcode the time
#define COUNTDOWN 30*1000 

AutoLogout::AutoLogout(LockProcess *parent) : QDialog(parent, "password dialog", true, WX11BypassWM)
{
    frame = new QFrame(this);
    frame->setFrameStyle(QFrame::Panel | QFrame::Raised);
    frame->setLineWidth(2);

    QLabel *pixLabel = new QLabel( frame, "pixlabel" );
    pixLabel->setPixmap(DesktopIcon("exit"));

    QLabel *greetLabel = new QLabel(i18n("<nobr><qt><b>An automatic logout has been configured</b></qt><nobr>"), frame);
    QLabel *infoLabel = new QLabel(i18n("<qt>If you do not want to be logged out, resume using this session before the timeout expires</qt>"), frame);

    mStatusLabel = new QLabel("<b> </b>", frame);
    mStatusLabel->setAlignment(QLabel::AlignCenter);

    QVBoxLayout *unlockDialogLayout = new QVBoxLayout( this );
    unlockDialogLayout->addWidget( frame );

    frameLayout = new QGridLayout(frame, 1, 1, KDialog::marginHint(), KDialog::spacingHint());
    frameLayout->addWidget(pixLabel, 0, 0);
    frameLayout->addWidget(greetLabel, 0, 1);
    frameLayout->addMultiCellWidget(mStatusLabel, 1, 1, 0, 1, AlignTop);
    frameLayout->addMultiCellWidget(infoLabel, 2, 2, 0, 1, AlignTop);

    // get the time remaining in seconds for the status label
    mRemaining = COUNTDOWN/1000;

    updateLabel(mRemaining);

    // event second we want to update the label
    mCountdownTimerId = startTimer(1000);

    // we found some activity so dismiss the dialog
    connect(qApp, SIGNAL(activity()), SLOT(slotActivity()));
}

AutoLogout::~AutoLogout()
{
    hide();
}

void AutoLogout::updateLabel(int timeout)
{
    mStatusLabel->setText(i18n("<nobr><qt>You will be automatically logged out in %1 seconds</qt></nobr>").arg(timeout));
}

void AutoLogout::timerEvent(QTimerEvent *ev)
{
    if (ev->timerId() == mCountdownTimerId)
    {
        updateLabel(mRemaining);
	--mRemaining;
	if (mRemaining < 0)
	{
		logout();
	}
    }
}

void AutoLogout::slotActivity()
{
    accept();
}

void AutoLogout::logout()
{
	killTimers();
	DCOPRef("ksmserver","ksmserver").send("logout", 0, 2, 0);
}

void AutoLogout::show()
{
    QDialog::show();
    QApplication::flushX();
}

#include "autologout.moc"
