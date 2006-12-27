/*****************************************************************
ksmserver - the KDE session management server

Copyright (C) 2000 Matthias Ettrich <ettrich@kde.org>
******************************************************************/

#include <config.h>

#include "shutdowndlg.h"

#include <QApplication>
#include <QCursor>
#include <QLabel>
#include <QLayout>
#include <QPainter>
#include <QMenu>
#include <QStyle>
#include <QTimer>

#include <kdebug.h>
#include <kdialog.h>
#include <kglobalsettings.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kpixmapeffect.h>
#include <QPixmap>
#include <kseparator.h>
#include <kstandardguiitem.h>
#include <kuser.h>

#include <sys/types.h>
#include <sys/utsname.h>
#include <unistd.h>
#include <stdlib.h>
#include <dmctl.h>

#include <X11/Xlib.h>

#include "shutdowndlg.moc"
#include <QX11Info>
#include <QDesktopWidget>

KSMShutdownFeedback * KSMShutdownFeedback::s_pSelf = 0L;

KSMShutdownFeedback::KSMShutdownFeedback()
 : QWidget( 0L, Qt::Popup ),
   m_currentY( 0 )
{
    setObjectName( "feedbackwidget" );
    setAttribute( Qt::WA_NoSystemBackground );
    setGeometry( QApplication::desktop()->geometry() );
    QTimer::singleShot( 10, this, SLOT( slotPaintEffect() ) );
}


void KSMShutdownFeedback::paintEvent( QPaintEvent* )
{
    if ( m_currentY >= height() )
        return;

    QPixmap pixmap;
    pixmap = QPixmap::grabWindow( QX11Info::appRootWindow(), 0, m_currentY, width(), 10 );
    pixmap = KPixmapEffect::fade( pixmap, 0.4, Qt::black );
    pixmap = KPixmapEffect::toGray( pixmap, true );

    QPainter painter( this );
    painter.drawPixmap( 0, m_currentY, pixmap );
}


void KSMShutdownFeedback::slotPaintEffect()
{
    if ( m_currentY >= height() )
        return;

    m_currentY += 10;

    // don't use update, as paint events could be merged
    repaint();

    QTimer::singleShot( 1, this, SLOT( slotPaintEffect() ) );
}

//////

KSMShutdownDlg::KSMShutdownDlg( QWidget* parent,
                                bool maysd, KWorkSpace::ShutdownType sdtype )
  : QDialog( parent, Qt::Popup ), targets(0)
    // this is a WType_Popup on purpose. Do not change that! Not
    // having a popup here has severe side effects.
{
    setModal( true );
    QVBoxLayout* vbox = new QVBoxLayout( this );
    vbox->setMargin( 0 );
    QFrame* frame = new QFrame( this );
    frame->setFrameStyle( QFrame::StyledPanel | QFrame::Raised );
    frame->setLineWidth( style()->pixelMetric( QStyle::PM_DefaultFrameWidth, 0, frame ) );
    vbox->addWidget( frame );
    vbox = new QVBoxLayout( frame );
    vbox->setMargin( 2 * KDialog::marginHint() );
    vbox->setSpacing( 2 * KDialog::spacingHint() );

    QLabel* label = new QLabel( i18n("End Session for \"%1\"", KUser().loginName()), frame );
    QFont fnt = label->font();
    fnt.setBold( true );
    fnt.setPointSize( fnt.pointSize() * 3 / 2 );
    label->setFont( fnt );
    vbox->addWidget( label, 0, Qt::AlignHCenter );

    QHBoxLayout* hbox = new QHBoxLayout( );
    vbox->addItem(hbox);
    hbox->setSpacing( 2 * KDialog::spacingHint() );

    // konqy
    QFrame* lfrm = new QFrame( frame );
    lfrm->setFrameStyle( QFrame::Panel | QFrame::Sunken );
    hbox->addWidget( lfrm, Qt::AlignCenter );

    QLabel* icon = new QLabel( lfrm );
    icon->setPixmap( UserIcon( "shutdownkonq" ) );
    lfrm->setFixedSize( icon->sizeHint());
    icon->setFixedSize( icon->sizeHint());

    // right column (buttons)
    QVBoxLayout* buttonlay = new QVBoxLayout( );
    hbox->addItem( buttonlay );
    buttonlay->setSpacing( 2 * KDialog::spacingHint() );
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
        if ( sdtype == KWorkSpace::ShutdownTypeHalt )
            btnHalt->setFocus();

        // Reboot
        KSMDelayedPushButton* btnReboot = new KSMDelayedPushButton( KGuiItem( i18n("&Restart Computer"), "reload"), frame );
        btnReboot->setFont( btnFont );
        buttonlay->addWidget( btnReboot );

        connect(btnReboot, SIGNAL(clicked()), SLOT(slotReboot()));
        if ( sdtype == KWorkSpace::ShutdownTypeReboot )
            btnReboot->setFocus();

        int def, cur;
        if ( DM().bootOptions( rebootOptions, def, cur ) ) {
        targets = new QMenu( frame );
        if ( cur == -1 )
            cur = def;

        int index = 0;
        for (QStringList::ConstIterator it = rebootOptions.begin(); it != rebootOptions.end(); ++it, ++index)
            {
            QString label = (*it);
            label=label.replace('&',"&&");
            if (index == cur)
                targets->insertItem( label + i18nc("current option in boot loader", " (current)"), index);
            else
                targets->insertItem( label, index );
            }

        btnReboot->setPopup(targets);
        connect( targets, SIGNAL(activated(int)), SLOT(slotReboot(int)) );
        }
    }

    buttonlay->addStretch( 1 );

    // Separator
    buttonlay->addWidget( new KSeparator( frame ) );

    // Back to Desktop
    KPushButton* btnBack = new KPushButton( KStandardGuiItem::cancel(), frame );
    buttonlay->addWidget( btnBack );
    connect(btnBack, SIGNAL(clicked()), SLOT(reject()));

}


void KSMShutdownDlg::slotLogout()
{
    m_shutdownType = KWorkSpace::ShutdownTypeNone;
    accept();
}


void KSMShutdownDlg::slotReboot()
{
    // no boot option selected -> current
    m_bootOption.clear();
    m_shutdownType = KWorkSpace::ShutdownTypeReboot;
    accept();
}

void KSMShutdownDlg::slotReboot(int opt)
{
    if (int(rebootOptions.size()) > opt)
        m_bootOption = rebootOptions[opt];
    m_shutdownType = KWorkSpace::ShutdownTypeReboot;
    accept();
}


void KSMShutdownDlg::slotHalt()
{
    m_bootOption.clear();
    m_shutdownType = KWorkSpace::ShutdownTypeHalt;
    accept();
}


bool KSMShutdownDlg::confirmShutdown( bool maysd, KWorkSpace::ShutdownType& sdtype, QString& bootOption )
{
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

    return result;
}

KSMDelayedPushButton::KSMDelayedPushButton( const KGuiItem &item,
                                            QWidget *parent )
  : KPushButton( item, parent), pop(0), popt(0)
{
  connect(this, SIGNAL(pressed()), SLOT(slotPressed()));
  connect(this, SIGNAL(released()), SLOT(slotReleased()));
  popt = new QTimer(this);
  connect(popt, SIGNAL(timeout()), SLOT(slotTimeout()));
}

void KSMDelayedPushButton::setPopup(QMenu *p)
{
    pop = p;
    if ( p!=0 )
            setMenu(p);
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
    bl.setX( qMin( bl.x(), br.x() - pop->sizeHint().width()));
    pop->popup( bl );
    popt->stop();
    setDown(false);
}
