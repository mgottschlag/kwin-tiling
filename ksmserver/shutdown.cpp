/*****************************************************************
ksmserver - the KDE session management server

Copyright (C) 2000 Matthias Ettrich <ettrich@kde.org>
******************************************************************/

#include <config.h>

#include "shutdown.h"
#include <qapplication.h>
#include <qlayout.h>
#include <qgroupbox.h>
#include <qcheckbox.h>
#include <qradiobutton.h>
#include <qvbuttongroup.h>
#include <qlabel.h>
#include <qvbox.h>
#include <qpainter.h>
#include <qtimer.h>
#include <qstyle.h>
#include <qcursor.h>

#include <klocale.h>
#include <kapplication.h>
#include <kdebug.h>
#include <kpushbutton.h>
#include <kstdguiitem.h>
#include <kwin.h>

#include <X11/Xlib.h>

#include "shutdown.moc"

KSMShutdownFeedback * KSMShutdownFeedback::s_pSelf = 0L;

KSMShutdownFeedback::KSMShutdownFeedback()
 : QWidget( 0L, "feedbackwidget", WStyle_Customize  | WStyle_NoBorder | WStyle_StaysOnTop )
{
    setBackgroundMode( QWidget::NoBackground );
    setGeometry( QApplication::desktop()->geometry() );
}

void KSMShutdownFeedback::paintEvent( QPaintEvent* )
{
    QPainter p;
    QBrush b( Qt::Dense4Pattern );
    p.begin( this );
    p.fillRect( rect(), b);
    p.end();
}

//////

KSMShutdownDlg::KSMShutdownDlg( QWidget* parent,
  bool saveSession,
  bool maysd, bool maynuke,
  KApplication::ShutdownType sdtype, KApplication::ShutdownMode sdmode )
    : QDialog( parent, 0, TRUE, WStyle_Customize | WStyle_NoBorder | WStyle_StaysOnTop )
{
    QVBoxLayout* vbox = new QVBoxLayout( this );
    QFrame* frame = new QFrame( this );
    frame->setFrameStyle( QFrame::StyledPanel | QFrame::Raised );
#if QT_VERSION < 300
    frame->setLineWidth( style().defaultFrameWidth() );
#else
    frame->setLineWidth( style().pixelMetric( QStyle::PM_DefaultFrameWidth, frame ) );
#endif
    vbox->addWidget( frame );
    vbox = new QVBoxLayout( frame, 15, 5 );

    QLabel* label = new QLabel( i18n("End KDE Session?"), frame );
    QFont fnt = label->font();
    fnt.setBold( true );
    fnt.setPixelSize( fnt.pixelSize() * 3 / 2 );
    label->setFont( fnt );
    vbox->addWidget( label, 0, AlignHCenter );

    if (maysd)
    {
        QHBoxLayout* hbox = new QHBoxLayout( vbox );
        QVButtonGroup *tgrp = new QVButtonGroup( i18n("Action"), frame );
        rLogout = new QRadioButton( i18n("&Logout"), tgrp );
        rHalt = new QRadioButton( i18n("&Halt"), tgrp );
        rReboot = new QRadioButton( i18n("&Reboot"), tgrp );
        hbox->addWidget( tgrp, AlignTop );
        connect( tgrp, SIGNAL( clicked(int) ), SLOT( slotSdMode(int) ) );
        mgrp = new QVButtonGroup( i18n("Shutdown mode"), frame );
        rSched = new QRadioButton( i18n("Sch&edule"), mgrp );
        if (maynuke)
            rForce = new QRadioButton( i18n("&Force Now"), mgrp );
        rTry = new QRadioButton( i18n("&Try Now"), mgrp );
        hbox->addWidget( mgrp, AlignTop );
    }

    checkbox = new QCheckBox( i18n("&Save session for future logins"), frame );
    vbox->addWidget( checkbox, 0, AlignCenter  );
    vbox->addStretch();

#if 0
    QFrame *line = new QFrame( frame );
    line->setFrameShape( QFrame::HLine );
    line->setFrameShadow( QFrame::Sunken );
    vbox->addWidget( line );
#endif

    QHBoxLayout* hbox = new QHBoxLayout( vbox );
    hbox->addStretch();
    KPushButton* yes = new KPushButton( maysd ?
                                         KStdGuiItem::ok() :
                                         KGuiItem( i18n( "&Logout" ) ),
                                        frame );
    connect( yes, SIGNAL( clicked() ), SLOT( accept() ) );
    yes->setDefault( TRUE );
    hbox->addWidget( yes );
    hbox->addStretch();
    KPushButton* cancel = new KPushButton( KStdGuiItem::cancel(), frame );
    connect( cancel, SIGNAL( clicked() ), SLOT( reject() ) );
    hbox->addWidget( cancel );
    hbox->addStretch();

    checkbox->setFocus();

    checkbox->setChecked( saveSession );
    if (maysd)
    {
        if (sdtype == KApplication::ShutdownTypeHalt)
            rHalt->setChecked( true );
        else if (sdtype == KApplication::ShutdownTypeReboot)
            rReboot->setChecked( true );
        else
            rLogout->setChecked( true );
	slotSdMode(0);

        if (sdmode == KApplication::ShutdownModeSchedule)
            rSched->setChecked( true );
        else if (sdmode == KApplication::ShutdownModeTryNow)
            rTry->setChecked( true );
        else
            rForce->setChecked( true );
    }
}

void KSMShutdownDlg::slotSdMode(int)
{
    mgrp->setEnabled( !rLogout->isChecked() );
}

bool KSMShutdownDlg::confirmShutdown( bool& saveSession,
				      bool maysd, bool maynuke,
				      KApplication::ShutdownType& sdtype, KApplication::ShutdownMode& sdmode )
{
    kapp->enableStyles();
    KSMShutdownDlg* l = new KSMShutdownDlg( KSMShutdownFeedback::self(),
                                            saveSession,
					    maysd, maynuke, sdtype, sdmode );

    // Show dialog (will save the background in showEvent)
    QSize sh = l->sizeHint();
#if QT_VERSION < 300
    KDesktopWidget *desktop = KApplication::desktop();
#else
    QDesktopWidget *desktop = KApplication::desktop();
#endif
    QRect rect = desktop->screenGeometry(desktop->screenNumber(QCursor::pos()));
    l->move(rect.x() + (rect.width() - sh.width())/2,
    	    rect.y() + (rect.height() - sh.height())/2);
    l->show();
    XSetInputFocus( qt_xdisplay(), l->winId(), RevertToParent, CurrentTime );
    bool result = l->exec();
    l->hide();

    if (maysd)
    {
        sdtype = l->rHalt->isChecked()   ? KApplication::ShutdownTypeHalt :
                 l->rReboot->isChecked() ? KApplication::ShutdownTypeReboot :
                                           KApplication::ShutdownTypeNone;
	sdmode = l->rSched->isChecked() ? KApplication::ShutdownModeSchedule :
                 l->rTry->isChecked()   ? KApplication::ShutdownModeTryNow :
                                          KApplication::ShutdownModeForceNow;
    }
    saveSession = l->checkbox->isChecked();

    delete l;

    kapp->disableStyles();
    return result;
}

void KSMShutdownDlg::showEvent( QShowEvent * )
{
    // Save background
    //kdDebug() << "showEvent => grabWindow "
    //          << x() << "," << y() << " " << width() << "x" << height() << endl;
    pm = QPixmap::grabWindow( KSMShutdownFeedback::self()->winId(),
                              x(), y(), width(), height() );
}

void KSMShutdownDlg::hideEvent( QHideEvent * )
{
    // Restore background
    bitBlt( KSMShutdownFeedback::self(), x(), y(), &pm );
}
