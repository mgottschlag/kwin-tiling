/*****************************************************************
ksmserver - the KDE session management server

Copyright (C) 2000 Matthias Ettrich <ettrich@kde.org>
******************************************************************/

#include <config.h>

#include "shutdown.h"
#include <qapplication.h>
#include <qlayout.h>
#include <qgroupbox.h>
#include <qvbuttongroup.h>
#include <qlabel.h>
#include <qvbox.h>
#include <qtimer.h>
#include <qstyle.h>
#include <qcursor.h>
#include <qmessagebox.h>
#include <qbuttongroup.h>
#include <qiconset.h>

#include <klocale.h>
#include <kapplication.h>
#include <kdebug.h>
#include <kpushbutton.h>
#include <kstdguiitem.h>
#include <kiconloader.h>
#include <kglobalsettings.h>
#include <kwin.h>
#include <kuser.h>
#include <kpixmap.h>
#include <kpixmapeffect.h>
#include <kdialog.h>
#include <kseparator.h>

#include <sys/types.h>
#include <sys/utsname.h>
#include <unistd.h>
#include <stdlib.h>

#include <X11/Xlib.h>

#include "shutdown.moc"

KSMShutdownFeedback * KSMShutdownFeedback::s_pSelf = 0L;

KSMShutdownFeedback::KSMShutdownFeedback()
 : QWidget( 0L, "feedbackwidget", WType_Popup ),
   m_currentY( 0 )
{
    setBackgroundMode( QWidget::NoBackground );
    setGeometry( QApplication::desktop()->geometry() );
    QTimer::singleShot( 10, this, SLOT( slotPaintEffect() ) );
}


void KSMShutdownFeedback::slotPaintEffect()
{
    if ( m_currentY >= height() )
        return;

    KPixmap pixmap;
    pixmap = QPixmap::grabWindow( qt_xrootwin(), 0, m_currentY, width(), 10 );
    pixmap = KPixmapEffect::fade( pixmap, 0.4, Qt::black );
    pixmap = KPixmapEffect::toGray( pixmap, true );
    bitBlt( this, 0, m_currentY, &pixmap );
    m_currentY += 10;
    QTimer::singleShot( 1, this, SLOT( slotPaintEffect() ) );
}

//////

KSMShutdownDlg::KSMShutdownDlg( QWidget* parent,
                                bool maysd, KApplication::ShutdownType sdtype )
    : QDialog( parent, 0, TRUE, WType_Popup )
    // this is a WType_Popup on purpose. Do not change that! Not
    // having a popup here has severe side effects.
{
    QVBoxLayout* vbox = new QVBoxLayout( this );
    QFrame* frame = new QFrame( this );
    frame->setFrameStyle( QFrame::StyledPanel | QFrame::Raised );
    frame->setLineWidth( style().pixelMetric( QStyle::PM_DefaultFrameWidth, frame ) );
    vbox->addWidget( frame );
    vbox = new QVBoxLayout( frame, 2 * KDialog::marginHint(),
                            2 * KDialog::spacingHint() );

    QLabel* label = new QLabel( i18n("End Session for \"%1\"").arg(KUser().loginName()), frame );
    QFont fnt = label->font();
    fnt.setBold( true );
    fnt.setPointSize( fnt.pointSize() * 3 / 2 );
    label->setFont( fnt );
    vbox->addWidget( label, 0, AlignHCenter );

    QHBoxLayout* hbox = new QHBoxLayout( vbox, 2 * KDialog::spacingHint() );

    // konqy
    QFrame* lfrm = new QFrame( frame );
    lfrm->setFrameStyle( QFrame::Panel | QFrame::Sunken );
    lfrm->setPaletteBackgroundColor( lfrm->colorGroup().midlight() );
    hbox->addWidget( lfrm, AlignCenter );
    QVBoxLayout* iconlay = new QVBoxLayout(
        lfrm, KDialog::marginHint(), KDialog::spacingHint() );
    QLabel* icon = new QLabel( lfrm );
    icon->setPixmap( UserIcon( "shutdownkonq" ) );
    iconlay->addWidget( icon );

    // right column (buttons)
    QVBoxLayout* buttonlay = new QVBoxLayout( hbox, 2 * KDialog::spacingHint() );
    buttonlay->setAlignment( Qt::AlignHCenter );

    buttonlay->addStretch( 1 );

    // End session
    KPushButton* btnLogout = new KPushButton( KGuiItem( i18n("&End Current Session"), "undo"), frame );
    QFont btnFont = btnLogout->font();
    buttonlay->addWidget( btnLogout );
    connect(btnLogout, SIGNAL(clicked()), SLOT(slotLogout()));

    if (maysd) {

        // Shutdown
        KPushButton* btnHalt = new KPushButton( KGuiItem( i18n("&Turn Off Computer"), "exit"), frame );
        btnHalt->setFont( btnFont );
        buttonlay->addWidget( btnHalt );
        connect(btnHalt, SIGNAL(clicked()), SLOT(slotHalt()));
        if ( sdtype == KApplication::ShutdownTypeHalt )
            btnHalt->setFocus();

        // Reboot
        KPushButton* btnReboot = new KPushButton( KGuiItem( i18n("&Restart Computer"), "reload"), frame );
        btnReboot->setFont( btnFont );
        buttonlay->addWidget( btnReboot );
        connect(btnReboot, SIGNAL(clicked()), SLOT(slotReboot()));
        if ( sdtype == KApplication::ShutdownTypeReboot )
            btnReboot->setFocus();

    }

    buttonlay->addStretch( 1 );

    // Separator
    buttonlay->addWidget( new KSeparator( frame ) );

    // Back to Desktop
    KPushButton* btnBack = new KPushButton( KStdGuiItem::cancel(), frame );
    buttonlay->addWidget( btnBack );
    connect(btnBack, SIGNAL(clicked()), SLOT(reject()));

}


void KSMShutdownDlg::slotLogout()
{
    m_shutdownType = KApplication::ShutdownTypeNone;
    accept();
}


void KSMShutdownDlg::slotReboot()
{
    m_shutdownType = KApplication::ShutdownTypeReboot;
    accept();
}


void KSMShutdownDlg::slotHalt()
{
    m_shutdownType = KApplication::ShutdownTypeHalt;
    accept();
}


bool KSMShutdownDlg::confirmShutdown( bool maysd, KApplication::ShutdownType& sdtype )
{
    kapp->enableStyles();
    KSMShutdownDlg* l = new KSMShutdownDlg( 0,
                                            //KSMShutdownFeedback::self(),
                                            maysd, sdtype );

    // Show dialog (will save the background in showEvent)
    QSize sh = l->sizeHint();
    QRect rect = KGlobalSettings::desktopGeometry(QCursor::pos());

    l->move(rect.x() + (rect.width() - sh.width())/2,
            rect.y() + (rect.height() - sh.height())/2);
    bool result = l->exec();
    sdtype = l->m_shutdownType;

    delete l;

    kapp->disableStyles();
    return result;
}
