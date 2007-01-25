/*****************************************************************
ksmserver - the KDE session management server

Copyright (C) 2000 Matthias Ettrich <ettrich@kde.org>
******************************************************************/

#include <config.h>

#include "shutdowndlg.h"
#include "../plasma/lib/theme.h"

#include <QApplication>
#include <QCursor>
#include <QLabel>
#include <QLayout>
#include <QPainter>
#include <QMenu>
#include <QStyle>
#include <QTimer>
#include <QSvgRenderer>
#include <QPainter>
#include <QPaintEvent>

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
//     QTimer::singleShot( 10, this, SLOT( slotPaintEffect() ) );
}


void KSMShutdownFeedback::paintEvent( QPaintEvent* )
{
    if ( m_currentY >= height() )
        return;

    QPixmap pixmap;
    pixmap = QPixmap::grabWindow( QX11Info::appRootWindow(), 0, 0, width(), height() );
    pixmap = KPixmapEffect::fade( pixmap, 0.4, Qt::black );
    pixmap = KPixmapEffect::toGray( pixmap, true );

    QPainter painter( this );
    painter.drawPixmap( 0, 0, pixmap );
}


void KSMShutdownFeedback::slotPaintEffect()
{
    if ( m_currentY >= height() )
        return;

    m_currentY += 10;

    // don't use update, as paint events could be merged
    repaint();

//     QTimer::singleShot( 1, this, SLOT( slotPaintEffect() ) );
}

////////////

KSMPushButton::KSMPushButton( const QString &text, QWidget *parent )
 : QPushButton( parent ), m_highlight( false ), m_popupMenu(0), m_popupTimer(0)
{
    setAttribute(Qt::WA_Hover, true);
    m_text = text;
    setFixedSize( 85, 85 );
    connect( this, SIGNAL(pressed()), SLOT(slotPressed()) );
    connect( this, SIGNAL(released()), SLOT(slotReleased()) );
}

void KSMPushButton::paintEvent( QPaintEvent * e )
{
  QPainter p( this );
  p.setClipRect( e->rect() );
  p.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
  QPen pen;
  if( m_highlight ) {
    p.setBrush( QColor(255,255,255,150) );
    pen.setColor(QColor(Qt::black));
  }
  else {
    p.setBrush( QColor(255,255,255,40) );
    pen.setColor(QColor(150,150,150,200));
  }

  pen.setWidth(1);
  p.setPen( pen );
  p.drawRect( rect() );

  p.drawPixmap( width()/2 - 16, 9, m_pixmap );

  p.translate( 0, 50 );
  QFont fnt;
  fnt.setBold( true );
  p.setFont( fnt );
  p.setPen( QPen( QColor( Qt::black ) ) );
  p.drawText( 0, 0, width(), height() - 50, Qt::AlignHCenter|Qt::AlignTop|Qt::TextWordWrap, m_text );
}

void KSMPushButton::setPixmap( const QPixmap &p )
{
  m_pixmap = p;
  if( m_pixmap.size().width() != 32 || m_pixmap.size().height () != 32 )
    m_pixmap = m_pixmap.scaled( 32, 32 );
  update();
}

void KSMPushButton::setPopupMenu( QMenu *m )
{
  m_popupMenu = m;
  if( !m_popupTimer ) {
    m_popupTimer = new QTimer( this );
    connect( m_popupTimer, SIGNAL(timeout()), this, SLOT(slotTimeout()));
  }
}

void KSMPushButton::slotPressed()
{
  if( m_popupTimer )
    m_popupTimer->start( QApplication::startDragTime() );
}

void KSMPushButton::slotReleased()
{
  if( m_popupTimer )
    m_popupTimer->stop();
}

void KSMPushButton::slotTimeout()
{
  m_popupTimer->stop();
  if( m_popupMenu ) {
    m_popupMenu->popup( mapToGlobal(rect().bottomLeft()) );
    m_highlight = false;
    update();
  }
}

bool KSMPushButton::event( QEvent *e )
{
  if( e->type() == QEvent::HoverEnter )
  {
    m_highlight = true;
    update();
    return true;
  }
  else if( e->type() == QEvent::HoverLeave )
  {
    m_highlight = false;
    update();
    return true;
  }
  else
    return QWidget::event( e );
}

//////

KSMShutdownDlg::KSMShutdownDlg( QWidget* parent,
                                bool maysd, KWorkSpace::ShutdownType sdtype )
  : QDialog( parent, Qt::Popup ), m_bgRenderer(0), m_renderDirty(true)
    // this is a WType_Popup on purpose. Do not change that! Not
    // having a popup here has severe side effects.
{
    m_theme = new Plasma::Theme(this);
    themeChanged();
    connect(m_theme, SIGNAL(changed()), this, SLOT(themeChanged()));
    setModal( true );
    resize(420, 180);
    KDialog::centerOnScreen(this);

    int space = maysd ? 15 : 80;
    QVBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->setMargin( 0 );
    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->setSpacing( space );

    QFont fnt;
    fnt.setBold( true );
    fnt.setPointSize( 12 );
    QLabel *topLabel = new QLabel( i18n("End Session for %1", KUser().loginName()), this );
    topLabel->setFixedHeight( 30 );
    topLabel->setSizePolicy( QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed ) );
    topLabel->setAlignment( Qt::AlignCenter );
    topLabel->setFont( fnt );

    mainLayout->addWidget( topLabel, 0 );
    mainLayout->addStretch();
    mainLayout->addLayout( btnLayout );
    mainLayout->addSpacing( 5 );

    KSMPushButton* btnLogout = new KSMPushButton( i18n("End Current Session"), this );
    btnLogout->setPixmap( KIconLoader::global()->loadIcon( "undo", K3Icon::NoGroup, 32 ) );
    connect(btnLogout, SIGNAL(clicked()), SLOT(slotLogout()));
    btnLayout->addWidget( btnLogout, 0 );

    if (maysd) {
        // Shutdown
        KSMPushButton* btnHalt = new KSMPushButton( i18n("Turn Off Computer"), this );
        btnHalt->setPixmap( KIconLoader::global()->loadIcon( "exit", K3Icon::NoGroup, 32 ) );
        btnLayout->addWidget( btnHalt, 0 );
        connect(btnHalt, SIGNAL(clicked()), SLOT(slotHalt()));
        if ( sdtype == KWorkSpace::ShutdownTypeHalt )
            btnHalt->setFocus();

        // Reboot
        KSMPushButton* btnReboot = new KSMPushButton( i18n("Restart Computer"), this );
        btnReboot->setPixmap( KIconLoader::global()->loadIcon( "reload", K3Icon::NoGroup, 32 ) );
        connect(btnReboot, SIGNAL(clicked()), SLOT(slotReboot()));
        btnLayout->addWidget( btnReboot, 0 );
        if ( sdtype == KWorkSpace::ShutdownTypeReboot )
            btnReboot->setFocus();

        int def, cur;
        if ( DM().bootOptions( rebootOptions, def, cur ) ) {
        if ( cur == -1 )
            cur = def;

        QMenu *rebootMenu = new QMenu( btnReboot );
        connect( rebootMenu, SIGNAL(activated(int)), SLOT(slotReboot(int)) );
        btnReboot->setPopupMenu( rebootMenu );

        int index = 0;
        for (QStringList::ConstIterator it = rebootOptions.begin(); it != rebootOptions.end(); ++it, ++index)
            {
            QString label = (*it);
            label=label.replace('&',"&&");
            if (index == cur) {
                rebootMenu->insertItem( label + i18nc("current option in boot loader", " (current)"), index );
            }
            else {
                rebootMenu->insertItem( label, index );
            }
        }
        }
    }

    KSMPushButton* btnBack = new KSMPushButton( i18n("Cancel"), this );
    btnBack->setPixmap( KIconLoader::global()->loadIcon( "cancel", K3Icon::NoGroup, 32 ) );
    btnLayout->addWidget( btnBack, 0 );
    connect(btnBack, SIGNAL(clicked()), SLOT(reject()));

    btnLayout->insertStretch( 0, 1 );
    btnLayout->insertStretch( -1, 1 );
    setLayout( mainLayout );
}

void KSMShutdownDlg::themeChanged()
{
    delete m_bgRenderer;
    m_bgRenderer = new QSvgRenderer(m_theme->imagePath("/background/shutdowndlg"), this);
}

void KSMShutdownDlg::paintEvent(QPaintEvent *e)
{
    QPainter p(this);
    p.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
    p.setClipRect(e->rect());
    p.save();
    p.setCompositionMode(QPainter::CompositionMode_Source);
    p.fillRect(rect(), Qt::transparent);
    p.restore();

    if (m_renderDirty)
    {
        m_renderedSvg.fill(Qt::transparent);
        QPainter p(&m_renderedSvg);
        p.setRenderHints(QPainter::Antialiasing);
        m_bgRenderer->render(&p);
        m_renderDirty = false;
    }
    p.drawPixmap(0, 0, m_renderedSvg);
}

void KSMShutdownDlg::resizeEvent(QResizeEvent *e)
{
    if (e->size() != m_renderedSvg.size())
    {
        QSize s( e->size().width(), e->size().height() );
        m_renderedSvg = QPixmap( s );
        m_renderDirty = true;
    }
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
    bool result = l->exec();
    sdtype = l->m_shutdownType;
    bootOption = l->m_bootOption;

    delete l;

    return result;
}
