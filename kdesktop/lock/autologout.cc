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
#include <kmessagebox.h>
#include <kdialog.h>

#include <QLayout>
#include <QMessageBox>
#include <QLabel>
#include <QStyle>
#include <QApplication>
#include <QDialog>
#include <QAbstractEventDispatcher>
#include <qprogressbar.h>
#include <dbus/qdbus.h>

#define COUNTDOWN 30

AutoLogout::AutoLogout(LockProcess *parent) : QDialog(parent, "password dialog", true,Qt::WX11BypassWM)
{
    frame = new QFrame(this);
    frame->setFrameStyle(QFrame::Panel | QFrame::Raised);
    frame->setLineWidth(2);

    QLabel *pixLabel = new QLabel( frame );
    pixLabel->setObjectName( "pixlabel" );
    pixLabel->setPixmap(DesktopIcon("exit"));

    QLabel *greetLabel = new QLabel(i18n("<nobr><qt><b>Automatic Log Out</b></qt><nobr>"), frame);
    QLabel *infoLabel = new QLabel(i18n("<qt>To prevent being logged out, resume using this session by moving the mouse or pressing a key.</qt>"), frame);

    mStatusLabel = new QLabel("<b> </b>", frame);
    mStatusLabel->setAlignment(Qt::AlignCenter);

    QLabel *mProgressLabel = new QLabel("Time Remaining:", frame);
    mProgressRemaining = new QProgressBar(frame);
    mProgressRemaining->setTextVisible(false);

    QVBoxLayout *unlockDialogLayout = new QVBoxLayout( this );
    unlockDialogLayout->addWidget( frame );

    frameLayout = new QGridLayout(frame);
    frameLayout->setSpacing(KDialog::spacingHint());
    frameLayout->setMargin(KDialog::marginHint());
    frameLayout->addWidget(pixLabel, 0, 0, 3, 1, Qt::AlignCenter | Qt::AlignTop);
    frameLayout->addWidget(greetLabel, 0, 1);
    frameLayout->addWidget(mStatusLabel, 1, 1);
    frameLayout->addWidget(infoLabel, 2, 1);
    frameLayout->addWidget(mProgressLabel, 3, 1);
    frameLayout->addWidget(mProgressRemaining, 4, 1);

    // get the time remaining in seconds for the status label
    mRemaining = COUNTDOWN * 25;

    mProgressRemaining->setMaximum(COUNTDOWN * 25);

    updateInfo(mRemaining);

    mCountdownTimerId = startTimer(1000/25);

    connect(qApp, SIGNAL(activity()), SLOT(slotActivity()));
}

AutoLogout::~AutoLogout()
{
    hide();
}

void AutoLogout::updateInfo(int timeout)
{
    mStatusLabel->setText(i18np("<nobr><qt>You will be automatically logged out in 1 second</qt></nobr>",
                               "<nobr><qt>You will be automatically logged out in %n seconds</qt></nobr>",
                               timeout / 25) );
    mProgressRemaining->setValue(timeout);
}

void AutoLogout::timerEvent(QTimerEvent *ev)
{
    if (ev->timerId() == mCountdownTimerId)
    {
        updateInfo(mRemaining);
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
    QAbstractEventDispatcher::instance()->unregisterTimers(this);
    QDBusInterfacePtr ksmserver( "org.kde.ksmserver", "/KSMServer", "org.kde.KSMServerInterface" );
    if ( ksmserver->isValid() )
        ksmserver->call( "logout", 0, 2, 0 );
}

void AutoLogout::show()
{
    QDialog::show();
    QApplication::flush();
}

#include "autologout.moc"
