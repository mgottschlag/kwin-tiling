/*****************************************************************
ksmserver - the KDE session management server

Copyright 2000 Matthias Ettrich <ettrich@kde.org>
Copyright 2007 Urs Wolfer <uwolfer @ kde.org>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

******************************************************************/

#include "shutdowndlg.h"
#include "plasma/svg.h"
#include "plasma/theme.h"

#include <QBitmap>
#include <QDesktopWidget>
#include <QLabel>
#include <QPainter>
#include <QMenu>
#include <QTimer>
#include <QTimeLine>
#include <QPaintEvent>
#include <QVBoxLayout>
#include <QHBoxLayout>

#include <KApplication>
#include <kdialog.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kuser.h>
#include <solid/control/powermanager.h>
#include <kwindowsystem.h>
#include <netwm.h>

#include <stdio.h>
#include <kxerrorhandler.h>

#include <kdisplaymanager.h>

#include <config-workspace.h>

#include "logouteffect.h"
#include "shutdowndlg.moc"

#include <kjob.h>

#define FONTCOLOR "#bfbfbf"

KSMShutdownFeedback * KSMShutdownFeedback::s_pSelf = 0L;

KSMShutdownFeedback::KSMShutdownFeedback()
 : QWidget( 0L, Qt::Popup ),
    m_currentY( 0 ), initialized( false )
{
    setObjectName( "feedbackwidget" );
    setAttribute( Qt::WA_NoSystemBackground );
    setAttribute( Qt::WA_PaintOnScreen );
    setGeometry( QApplication::desktop()->geometry() );
    m_pixmap = QPixmap( size() );
    QTimer::singleShot( 10, this, SLOT( slotPaintEffect() ) );
}


void KSMShutdownFeedback::paintEvent( QPaintEvent* )
{
    if ( !initialized )
        return;

    QPainter painter( this );
    painter.setCompositionMode( QPainter::CompositionMode_Source );
    painter.drawPixmap( 0, 0, m_pixmap );
}

void KSMShutdownFeedback::slotPaintEffect()
{
    effect = LogoutEffect::create(this, &m_pixmap);
    effect->start();

    initialized = true;
}

void KSMShutdownFeedback::start()
{
    if( KWindowSystem::compositingActive()) {
        // HACK do properly
        Display* dpy = QX11Info::display();
        char net_wm_cm_name[ 100 ];
        sprintf( net_wm_cm_name, "_NET_WM_CM_S%d", DefaultScreen( dpy ));
        Atom net_wm_cm = XInternAtom( dpy, net_wm_cm_name, False );
        Window sel = XGetSelectionOwner( dpy, net_wm_cm );
        Atom hack = XInternAtom( dpy, "_KWIN_LOGOUT_EFFECT", False );
        bool wmsupport = false;
        if( sel != None ) {
            KXErrorHandler handler;
            int cnt;
            Atom* props = XListProperties( dpy, sel, &cnt );
            if( !handler.error( false ) && props != NULL && qFind( props, props + cnt, hack ) != props + cnt )
                wmsupport = true;
            if( props != NULL )
                XFree( props );
        }
        if( wmsupport )
            return;
    }
    s_pSelf = new KSMShutdownFeedback();
    s_pSelf->show();
}

void KSMShutdownFeedback::stop()
{
    delete s_pSelf;
    s_pSelf = NULL;
}

////////////

KSMPushButton::KSMPushButton( const QString &text, QWidget *parent, bool smallButton )
 : QPushButton( parent ),  m_highlight( false ), m_text( text ), m_popupMenu(0), m_popupTimer(0),
   m_glowOpacity( 0.0 ), m_smallButton( smallButton )
{
    setAttribute(Qt::WA_Hover, true);
    m_text = text;
    init();
}

void KSMPushButton::init()
{
    if (m_smallButton) {
        setMinimumSize(88, 22);
        setFixedHeight(22); // workaround: force correct height
    } else {
        setMinimumSize(165, 38);
        setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    }

    connect( this, SIGNAL(pressed()), SLOT(slotPressed()) );
    connect( this, SIGNAL(released()), SLOT(slotReleased()) );

    m_glowSvg = new Plasma::Svg(this);
    m_glowSvg->setImagePath("dialogs/shutdowndialog");
    connect( m_glowSvg, SIGNAL(repaintNeeded()), this, SLOT(update()) );

    m_glowTimeLine = new QTimeLine( 150, this );
    connect( m_glowTimeLine, SIGNAL(valueChanged(qreal)),
            this, SLOT(animateGlow(qreal)) );

    QFont fnt;
    fnt.setPixelSize(12);

    // Calculate the width of the text when splitted on two lines and
    // properly resize the button.
    if (QFontMetrics(fnt).width(m_text) > width() - 4 - (m_smallButton ? 16 : 32) ||
        (2 * QFontMetrics(fnt).lineSpacing() > height() && !m_smallButton) ) {
        int w, h;
        int i = m_text.length()/2;
        int fac = 1;
        int diff = 1;
        while( i && i < m_text.length() && m_text[i] != ' ' ) {
            i = i + (diff * fac);
            fac *= -1;
            ++diff;
        }
        QString upper = m_text.left( i );
        QString lower = m_text.right( m_text.length() - i );

        w = qMax(QFontMetrics(fnt).width(upper) + 18 + (m_smallButton ? 16 : 32),
                 QFontMetrics(fnt).width(lower) + 18 + (m_smallButton ? 16 : 32));
        w = qMax(w, width());
        h = qMax(height(), ((upper.isEmpty() || lower.isEmpty()) ? 1 : 2) * QFontMetrics(fnt).lineSpacing());
        if (w > width() || h > height()) {
            setMinimumSize(w, h);
            if (m_smallButton)
                setFixedHeight(h);
            updateGeometry();
        }
    }
}

void KSMPushButton::paintEvent( QPaintEvent * e )
{
    QPainter p( this );
    p.setClipRect( e->rect() );
    p.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
    QPen pen;
    QFont fnt;
    QColor fntColor = Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor);
    fnt.setPixelSize(12);
    p.setFont( fnt );
    p.setCompositionMode(QPainter::CompositionMode_SourceOver);

    p.save();

    if (m_glowOpacity > 0) {
        p.setOpacity(m_glowOpacity); // fade in
        m_glowSvg->paint(&p, QRect(0, 0, width(), height()), m_smallButton ? "button-small-hover" : "button-hover");
        p.setOpacity(1.0 - m_glowOpacity); // fade normal background out
        m_glowSvg->paint(&p, QRect(0, 0, width(), height()), m_smallButton ? "button-small-normal" : "button-normal");
        p.setOpacity(1.0);
    } else {
        m_glowSvg->resize();
        m_glowSvg->paint(&p, QRect(0, 0, width(), height()), m_smallButton ? "button-small-normal" : "button-normal");
    }

    p.restore();

    p.setRenderHints( QPainter::Antialiasing, false);
    p.drawPixmap(width() - (m_smallButton ? 16 : 32) - 4, height() / 2 - (m_smallButton ? 8 : 16), m_pixmap);

    p.save();
    p.setPen(fntColor);
    p.drawText(10, 0, width() - (m_smallButton ? 16 : 32) - 8, height(),
               Qt::AlignVCenter | Qt::AlignLeft | Qt::TextWordWrap | Qt::TextShowMnemonic, m_text);
    p.restore();

    if( m_popupMenu ) {
        p.save();
        p.setBrush(fntColor);
        pen.setColor(QColor(fntColor));
        p.setPen( pen );
        QPoint points[3] = {
            QPoint(width() - 10 - 34, height() - 7),
            QPoint(width() - 4 - 34, height() - 7),
            QPoint(width() - 7 - 34, height() - 4) };
        p.drawPolygon(points, 3); // TODO: use QStyle
        p.restore();
    }
}

void KSMPushButton::resizeEvent(QResizeEvent *e)
{
    m_glowSvg->resize( e->size() );
    QPushButton::resizeEvent( e );
}

void KSMPushButton::animateGlow( qreal value )
{
    m_glowOpacity = value;
    update();
}

void KSMPushButton::setPixmap( const QPixmap &p )
{
    m_pixmap = p;
    int size = m_smallButton ? 16 : 32;
    if (m_pixmap.size().width() != size || m_pixmap.size().height() != size)
        m_pixmap = m_pixmap.scaled(size, size);
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
    if (e->type() == QEvent::HoverEnter || e->type() == QEvent::FocusIn)
    {
        if (m_glowOpacity > 0) // already hovered
            return true;
        m_highlight = true;
        m_glowTimeLine->setDirection( QTimeLine::Forward );
        if (m_glowTimeLine->state() == QTimeLine::Running)
            m_glowTimeLine->stop();
        m_glowTimeLine->start();
        update();
        return true;
    }
    else if (e->type() == QEvent::HoverLeave || e->type() == QEvent::FocusOut)
    {
        if (hasFocus())
            return true;
        m_highlight = false;
        m_glowTimeLine->setDirection( QTimeLine::Backward );
        if (m_glowTimeLine->state() == QTimeLine::Running)
            m_glowTimeLine->stop();
        m_glowTimeLine->start();
        update();
        return true;
    }
    else
        return QPushButton::event( e );
}

//////

Q_DECLARE_METATYPE(Solid::Control::PowerManager::SuspendMethod)

KSMShutdownDlg::KSMShutdownDlg( QWidget* parent,
                                bool maysd, KWorkSpace::ShutdownType sdtype )
  : QDialog( parent, Qt::Popup ), //krazy:exclude=qclasses
    m_automaticallyDoSeconds(60),
    m_btnLogout(0),
    m_btnHalt(0),
    m_btnReboot(0)
    // this is a WType_Popup on purpose. Do not change that! Not
    // having a popup here has severe side effects.
{
    winId(); // workaround for Qt4.3 setWindowRole() assert
    setWindowRole( "logoutdialog" );
//#if !(QT_VERSION >= QT_VERSION_CHECK(4, 3, 3) || defined(QT_KDE_QT_COPY))
// Qt doesn't set this on unmanaged windows
    QByteArray appName = qAppName().toLatin1();
    XClassHint class_hint;
    class_hint.res_name = appName.data(); // application name
    class_hint.res_class = const_cast<char *>(QX11Info::appClass());   // application class
    XSetWMProperties( QX11Info::display(), winId(), NULL, NULL, NULL, NULL, NULL, NULL, &class_hint );
    XChangeProperty( QX11Info::display(), winId(),
        XInternAtom( QX11Info::display(), "WM_WINDOW_ROLE", False ), XA_STRING, 8, PropModeReplace,
        (unsigned char *)"logoutdialog", strlen( "logoutdialog" ));

//#endif
    m_svg = new Plasma::Svg(this);
    m_svg->setImagePath("dialogs/shutdowndialog");
    connect( m_svg, SIGNAL(repaintNeeded()), this, SLOT(update()) );
    setModal( true );
    resize(400, 220);
    KDialog::centerOnScreen(this);

    QVBoxLayout *mainLayout = new QVBoxLayout();

    mainLayout->setContentsMargins(12, 9, 12, 7);
    QVBoxLayout *buttonLayout = new QVBoxLayout();
    QHBoxLayout *buttonMainLayout = new QHBoxLayout();
    if (m_svg->hasElement("picture")) {
        buttonMainLayout->addSpacing(m_svg->elementRect("picture").toRect().width() + 12);
    }
    buttonMainLayout->addStretch();
    buttonMainLayout->addLayout(buttonLayout);

    QHBoxLayout *bottomLayout = new QHBoxLayout();

    QFont fnt;
    fnt.setPixelSize(16);
    QColor fntColor = Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor);
    QPalette palette;
    palette.setColor(QPalette::WindowText, fntColor);

    m_btnLogout = new KSMPushButton( i18n("&Logout"), this );
    m_btnLogout->setPixmap(KIconLoader::global()->loadIcon("system-log-out", KIconLoader::NoGroup, 32));
    m_btnLogout->setFocus();
    connect(m_btnLogout, SIGNAL(clicked()), SLOT(slotLogout()));
    buttonLayout->addWidget(m_btnLogout, Qt::AlignRight | Qt::AlignTop);

    if (maysd) {
        // Shutdown
        m_btnHalt = new KSMPushButton( i18n("&Turn Off Computer"), this );
        m_btnHalt->setPixmap(KIconLoader::global()->loadIcon("system-shutdown", KIconLoader::NoGroup, 32));
        buttonLayout->addWidget(m_btnHalt, Qt::AlignTop | Qt::AlignRight);
        connect(m_btnHalt, SIGNAL(clicked()), SLOT(slotHalt()));
        if ( sdtype == KWorkSpace::ShutdownTypeHalt )
            m_btnHalt->setFocus();

        QMenu *shutdownMenu = new QMenu( m_btnHalt );
        QActionGroup* spdActionGroup = new QActionGroup(shutdownMenu);
        connect( spdActionGroup, SIGNAL(triggered(QAction*)), SLOT(slotSuspend(QAction*)) );
        m_btnHalt->setPopupMenu( shutdownMenu );
        Solid::Control::PowerManager::SuspendMethods spdMethods = Solid::Control::PowerManager::supportedSuspendMethods();
        if( spdMethods & Solid::Control::PowerManager::Standby ) {
            QAction* action = new QAction(i18n("&Standby"), spdActionGroup);
            action->setData(QVariant::fromValue(Solid::Control::PowerManager::Standby));
        }
        if( spdMethods & Solid::Control::PowerManager::ToRam ) {
            QAction* action = new QAction(i18n("Suspend to &RAM"), spdActionGroup);
            action->setData(QVariant::fromValue(Solid::Control::PowerManager::ToRam));
        }
        if( spdMethods & Solid::Control::PowerManager::ToDisk ) {
            QAction* action = new QAction(i18n("Suspend to &Disk"), spdActionGroup);
            action->setData(QVariant::fromValue(Solid::Control::PowerManager::ToDisk));
        }
        shutdownMenu->addActions(spdActionGroup->actions());

        // Reboot
        m_btnReboot = new KSMPushButton( i18n("&Restart Computer"), this );
        m_btnReboot->setPixmap(KIconLoader::global()->loadIcon("system-restart", KIconLoader::NoGroup, 32));
        connect(m_btnReboot, SIGNAL(clicked()), SLOT(slotReboot()));
        buttonLayout->addWidget(m_btnReboot, Qt::AlignTop | Qt::AlignRight);
        if ( sdtype == KWorkSpace::ShutdownTypeReboot )
            m_btnReboot->setFocus();

        int def, cur;
        if ( KDisplayManager().bootOptions( rebootOptions, def, cur ) ) {
            if ( cur == -1 )
                cur = def;

            QMenu *rebootMenu = new QMenu( m_btnReboot );
            QActionGroup* rebootActionGroup = new QActionGroup(rebootMenu);
            connect( rebootActionGroup, SIGNAL(triggered(QAction*)), SLOT(slotReboot(QAction*)) );
            m_btnReboot->setPopupMenu( rebootMenu );

            int index = 0;
            for (QStringList::ConstIterator it = rebootOptions.begin(); it != rebootOptions.end(); ++it, ++index) {
                QString label = (*it);
                label=label.replace('&',"&&");
                QAction* action = new QAction(label, rebootActionGroup);
                action->setData(index);
                if (index == cur) {
                    action->setText( label + i18nc("current option in boot loader", " (current)") );
                }
            }
            rebootMenu->addActions(rebootActionGroup->actions());
        }
    }

    KSMPushButton* btnBack = new KSMPushButton(i18n("&Cancel"), this, true);
    btnBack->setPixmap(KIconLoader::global()->loadIcon( "dialog-cancel", KIconLoader::NoGroup, 16));

    m_automaticallyDoLabel = new QLabel(this);
    m_automaticallyDoLabel->setPalette(palette);
    fnt.setPixelSize(11);
    m_automaticallyDoLabel->setFont(fnt);
    m_automaticallyDoLabel->setWordWrap(true);
    automaticallyDoTimeout();

    QTimer *automaticallyDoTimer = new QTimer(this);
    connect(automaticallyDoTimer, SIGNAL(timeout()), this, SLOT(automaticallyDoTimeout()));
    automaticallyDoTimer->start(1000);

    bottomLayout->addWidget(m_automaticallyDoLabel, 1, Qt::AlignBottom);
    bottomLayout->addWidget(btnBack);
    connect(btnBack, SIGNAL(clicked()), SLOT(reject()));

    mainLayout->addLayout(buttonMainLayout);
    mainLayout->addSpacing(9);
    mainLayout->addLayout(bottomLayout);

    setLayout( mainLayout );
}

void KSMShutdownDlg::automaticallyDoTimeout()
{
    QPushButton *focusedButton = qobject_cast<QPushButton *>(focusWidget());
    if (focusedButton) {
        if (m_automaticallyDoSeconds <= 0) { // timeout is at 0, do selected action
                focusedButton->click();
        // following code is required to provide a clean way to translate strings
        } else if (focusedButton == m_btnLogout) {
            m_automaticallyDoLabel->setText(i18np("Log out in 1 second.",
                                            "Log out in %1 seconds.", m_automaticallyDoSeconds));
        } else if (focusedButton == m_btnHalt) {
                m_automaticallyDoLabel->setText(i18np("Turn off computer in 1 second.",
                                                      "Turn off computer in %1 seconds.", m_automaticallyDoSeconds));
        } else if (focusedButton == m_btnReboot) {
                m_automaticallyDoLabel->setText(i18np("Reboot computer in 1 second.",
                                                      "Reboot computer in %1 seconds.", m_automaticallyDoSeconds));
        } else {
            m_automaticallyDoLabel->setText(QString());
        }

        if (m_automaticallyDoLabel > 0) {
            --m_automaticallyDoSeconds;
        }
    }
}

void KSMShutdownDlg::paintEvent(QPaintEvent *e)
{
    Q_UNUSED(e);
    QPainter p(this);
    p.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    p.setCompositionMode( QPainter::CompositionMode_Source );
    p.setClipRect(e->rect());

    p.fillRect(QRect(0, 0, width(), height()), Qt::transparent);
    m_svg->paint(&p, QRect(0, 0, width(), height()), "background");

    if (m_svg->hasElement("picture")) {
        QRect r = m_svg->elementRect("picture").toRect();
        r.moveTop(m_btnLogout->geometry().top());
        m_svg->paint(&p, r, "picture");
    }
}

void KSMShutdownDlg::resizeEvent(QResizeEvent *e)
{
    QDialog::resizeEvent( e );

    if( KWindowSystem::compositingActive()) {
        clearMask();
    } else {
        QBitmap mask(size());
        mask.fill(Qt::color0);

        QPainter p(&mask);
        m_svg->resize(size());
        m_svg->paint(&p, QRect(0, 0, width(), height()), "background");
        setMask(mask);
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

void KSMShutdownDlg::slotReboot(QAction* action)
{
    int opt = action->data().toInt();
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


void KSMShutdownDlg::slotSuspend(QAction* action)
{
    m_bootOption.clear();
    Solid::Control::PowerManager::SuspendMethod spdMethod = action->data().value<Solid::Control::PowerManager::SuspendMethod>();
    KJob *job = Solid::Control::PowerManager::suspend( spdMethod );
    if (job != 0)
       job->start();
    reject();
}

bool KSMShutdownDlg::confirmShutdown( bool maysd, KWorkSpace::ShutdownType& sdtype, QString& bootOption )
{
    KSMShutdownDlg* l = new KSMShutdownDlg( 0,
                                            //KSMShutdownFeedback::self(),
                                            maysd, sdtype );
    XClassHint classHint;
    classHint.res_name = const_cast<char*>("ksmserver");
    classHint.res_class = const_cast<char*>("ksmserver");

    XSetClassHint(QX11Info::display(), l->winId(), &classHint);
    bool result = l->exec();
    sdtype = l->m_shutdownType;
    bootOption = l->m_bootOption;

    delete l;

    return result;
}
