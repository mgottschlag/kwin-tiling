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
#include <qcombobox.h>
#include <qcursor.h>
#include <qmessagebox.h>
#include <qbuttongroup.h>
#include <qiconset.h>
#include <qpopupmenu.h>

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
#include <dmctl.h>

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
  : QDialog( parent, 0, TRUE, WType_Popup ), targets(0)
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
    hbox->addWidget( lfrm, AlignCenter );

    QLabel* icon = new QLabel( lfrm );
    icon->setPixmap( UserIcon( "shutdownkonq" ) );
    lfrm->setFixedSize( icon->sizeHint());
    icon->setFixedSize( icon->sizeHint());

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
        KSMDelayedPushButton* btnReboot = new KSMDelayedPushButton( KGuiItem( i18n("&Restart Computer"), "reload"), frame );
        btnReboot->setFont( btnFont );
        buttonlay->addWidget( btnReboot );

        connect(btnReboot, SIGNAL(clicked()), SLOT(slotReboot()));
        if ( sdtype == KApplication::ShutdownTypeReboot )
            btnReboot->setFocus();

        int def, cur;
        if ( DM().bootOptions( rebootOptions, def, cur ) ) {
	  targets = new QPopupMenu( frame );
	  if ( cur == -1 )
	    cur = def;

	  int index = 0;
	  for (QStringList::ConstIterator it = rebootOptions.begin(); it != rebootOptions.end(); ++it, ++index)
	    {
	      if (index == cur)
		targets->insertItem( *it + i18n("current option in boot loader", " (current)"), index);
	      else
		targets->insertItem( *it, index );
	    }

	  btnReboot->setPopup(targets);
	  connect( targets, SIGNAL(activated(int)), SLOT(slotReboot(int)) );
	}
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
    // no boot option selected -> current
    m_bootOption = QString::null;
    m_shutdownType = KApplication::ShutdownTypeReboot;
    accept();
}

void KSMShutdownDlg::slotReboot(int opt)
{
    if (int(rebootOptions.size()) > opt)
        m_bootOption = rebootOptions[opt];
    m_shutdownType = KApplication::ShutdownTypeReboot;
    accept();
}


void KSMShutdownDlg::slotHalt()
{
    m_bootOption = QString::null;
    m_shutdownType = KApplication::ShutdownTypeHalt;
    accept();
}


bool KSMShutdownDlg::confirmShutdown( bool maysd, KApplication::ShutdownType& sdtype, QString& bootOption )
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
    bootOption = l->m_bootOption;

    delete l;

    kapp->disableStyles();
    return result;
}

KSMDelayedPushButton::KSMDelayedPushButton( const KGuiItem &item,
					    QWidget *parent,
					    const char *name)
  : KPushButton( item, parent, name), pop(0), popt(0)
{
  connect(this, SIGNAL(pressed()), SLOT(slotPressed()));
  connect(this, SIGNAL(released()), SLOT(slotReleased()));
  popt = new QTimer(this);
  connect(popt, SIGNAL(timeout()), SLOT(slotTimeout()));
}

void KSMDelayedPushButton::setPopup(QPopupMenu *p)
{
  pop = p;
  setIsMenuButton(p != 0);
}

void KSMDelayedPushButton::slotPressed()
{
  if (pop)
    popt->start(QApplication::startDragTime());
}

void KSMDelayedPushButton::slotReleased()
{
  popt->stop();
}

void KSMDelayedPushButton::slotTimeout()
{
  QPoint bl = mapToGlobal(rect().bottomLeft());
  QWidget *par = (QWidget*)parent();
  QPoint br = par->mapToGlobal(par->rect().bottomRight());
  // we must avoid painting over the dialog's limits
  // as the feedback area isn't repainted when the popup disappears
  bl.setX( QMIN( bl.x(), br.x() - pop->sizeHint().width()));
  pop->popup( bl );
  popt->stop();
  setDown(false);
}
