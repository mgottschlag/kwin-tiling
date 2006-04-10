/*
 *  This file is part of the KDE Libraries
 *  Copyright (C) 2002 Hamish Rodda <rodda@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 *
 */


#include <qlayout.h>

#include <qtimer.h>
#include <qprogressbar.h>
#include <qlabel.h>

#include <kwin.h>
#include <kiconloader.h>

#include <klocale.h>
#include <kdebug.h>
#include <kvbox.h>

#include "ktimerdialog.h"
#include "ktimerdialog.moc"

KTimerDialog::KTimerDialog( int msec, TimerStyle style, QWidget *parent,
                 const char *name, bool modal,
                 const QString &caption,
                 int buttonMask, ButtonCode defaultButton,
                 bool separator,
                 const KGuiItem &user1,
                 const KGuiItem &user2,
                 const KGuiItem &user3 )
    : KDialogBase(parent, name, modal, caption, buttonMask, defaultButton,
                 separator, user1, user2, user3 )
{
    totalTimer = new QTimer( this );
    updateTimer = new QTimer( this );
    msecTotal = msecRemaining = msec;
    updateInterval = 1000;
    tStyle = style;
	KWin::setIcons( winId(), DesktopIcon("randr"), SmallIcon("randr") );
    // default to cancelling the dialog on timeout
    if ( buttonMask & Cancel )
        buttonOnTimeout = Cancel;

    connect( totalTimer, SIGNAL( timeout() ), SLOT( slotInternalTimeout() ) );
    connect( updateTimer, SIGNAL( timeout() ), SLOT( slotUpdateTime() ) );

    // create the widgets
    mainWidget = new KVBox( this );
    timerWidget = new KHBox( mainWidget );
    timerLabel = new QLabel( timerWidget );
    timerProgress = new QProgressBar( timerWidget );
    timerProgress->setRange( 0, msecTotal );
    timerProgress->setTextVisible( false );

    KDialogBase::setMainWidget( mainWidget );

    slotUpdateTime( false );
}

KTimerDialog::~KTimerDialog()
{
}

void KTimerDialog::show()
{
    KDialogBase::show();
    totalTimer->start( msecTotal, true );
    updateTimer->start( updateInterval, false );
}

int KTimerDialog::exec()
{
    totalTimer->start( msecTotal, true );
    updateTimer->start( updateInterval, false );
    return KDialogBase::exec();
}

void KTimerDialog::setMainWidget( QWidget *widget )
{
    // yuck, here goes.
    KVBox *newWidget = new KVBox( this );

    if ( widget->parentWidget() != mainWidget ) {
        widget->reparent( newWidget, 0, QPoint(0,0) );
    } else {
        newWidget->insertChild( widget );
    }

    timerWidget->reparent( newWidget, 0, QPoint(0, 0) );

    delete mainWidget;
    mainWidget = newWidget;
    KDialogBase::setMainWidget( mainWidget );
}

void KTimerDialog::setRefreshInterval( int msec )
{
    updateInterval = msec;
    if ( updateTimer->isActive() )
        updateTimer->start( updateInterval );
}

int KTimerDialog::timeoutButton() const
{
    return buttonOnTimeout;
}

void KTimerDialog::setTimeoutButton( const ButtonCode newButton )
{
    buttonOnTimeout = newButton;
}

int KTimerDialog::timerStyle() const
{
    return tStyle;
}

void KTimerDialog::setTimerStyle( const TimerStyle newStyle )
{
    tStyle = newStyle;
}

void KTimerDialog::slotUpdateTime( bool update )
{
    if ( update )
        switch ( tStyle ) {
            case CountDown:
                msecRemaining -= updateInterval;
                break;
            case CountUp:
                msecRemaining += updateInterval;
                break;
            case Manual:
                break;
        }

    timerProgress->setValue( msecRemaining );

    timerLabel->setText( i18np("1 second remaining:","%n seconds remaining:",msecRemaining/1000) );
}

void KTimerDialog::slotInternalTimeout()
{
    emit timerTimeout();
    switch ( buttonOnTimeout ) {
        case Help:
            slotButtonClicked(KDialog::Help);
            break;
        case Default:
            slotButtonClicked(KDialog::Default);
            break;
        case Ok:
            slotButtonClicked(KDialog::Ok);
            break;
        case Apply:
            slotButtonClicked(KDialog::Apply);
            break;
        case Try:
            slotButtonClicked(KDialog::Try);
            break;
        case Cancel:
            slotCancel();
            break;
        case Close:
            slotButtonClicked(KDialog::Close);
            break;
        /*case User1:
            slotUser1();
            break;
        case User2:
            slotUser2();
            break;*/
        case User3:
            slotButtonClicked(KDialog::User3);
            break;
        case No:
            slotButtonClicked(KDialog::No);
            break;
        case Yes:
            slotButtonClicked(KDialog::Cancel);
            break;
        case Details:
            slotButtonClicked(KDialog::Details);
            break;
        case Filler:
        case Stretch:
            kDebug() << "Cannot execute button code " << buttonOnTimeout << endl;
            break;
    }
}
