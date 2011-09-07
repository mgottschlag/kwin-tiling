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
#include "plasma/framesvg.h"
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
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusMessage>
#include <QtDBus/QDBusPendingCall>

#include <KApplication>
#include <kdialog.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kuser.h>
#include <Solid/PowerManagement>
#include <kwindowsystem.h>
#include <netwm.h>

#include <stdio.h>
#include <kxerrorhandler.h>

#include <kworkspace/kdisplaymanager.h>

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
    QTimer::singleShot( 10, this, SLOT(slotPaintEffect()) );
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
    connect(effect, SIGNAL(initialized()),
            this,   SLOT  (slotPaintEffectInitialized()));

    effect->start();
}

void KSMShutdownFeedback::slotPaintEffectInitialized()
{
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
        if( wmsupport ) {
            // Announce that the user MAY be logging out (Intended for the compositor)
            Atom announce = XInternAtom(dpy, "_KDE_LOGGING_OUT", False);
            unsigned char dummy = 0;
            XChangeProperty(dpy, QX11Info::appRootWindow(), announce, announce, 8, PropModeReplace,
                &dummy, 1);

            // Don't show our own effect
            return;
        }
    }
    s_pSelf = new KSMShutdownFeedback();
    s_pSelf->show();
}

void KSMShutdownFeedback::stop()
{
    delete s_pSelf;
    s_pSelf = NULL;
}

void KSMShutdownFeedback::logoutCanceled()
{
    if( KWindowSystem::compositingActive()) {
        // We are no longer logging out, announce (Intended for the compositor)
        Display* dpy = QX11Info::display();
        Atom announce = XInternAtom(dpy, "_KDE_LOGGING_OUT", False);
        XDeleteProperty(QX11Info::display(), QX11Info::appRootWindow(), announce);
    }
}

////////////

KSMPushButton::KSMPushButton( const QString &text, QWidget *parent, bool smallButton )
 : QPushButton( text, parent ),  m_highlight( false ), m_text( text ), m_popupMenu(0), m_popupTimer(0),
   m_glowOpacity( 0.0 ), m_smallButton( smallButton )
{
    setAttribute(Qt::WA_Hover, true);
    m_text = text;
    init();
}

void KSMPushButton::init()
{
    m_glowSvg = new Plasma::Svg(this);
    m_glowSvg->setImagePath("dialogs/shutdowndialog");

    if (m_smallButton) {
        setMinimumSize(88, 22);
        setFixedHeight(22); // workaround: force correct height
    } else {
        setMinimumSize(m_glowSvg->elementSize("button-normal"));
        setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    }

    connect( this, SIGNAL(pressed()), SLOT(slotPressed()) );
    connect( this, SIGNAL(released()), SLOT(slotReleased()) );

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

    m_glowSvg->resize();

    if (m_glowOpacity > 0) {
        p.setOpacity(m_glowOpacity); // fade in
        m_glowSvg->paint(&p, QRect(0, 0, width(), height()), m_smallButton ? "button-small-hover" : "button-hover");
        p.setOpacity(1.0 - m_glowOpacity); // fade normal background out
        m_glowSvg->paint(&p, QRect(0, 0, width(), height()), m_smallButton ? "button-small-normal" : "button-normal");
        p.setOpacity(1.0);
    } else {
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
        int baseY = height()/2 + m_pixmap.height()/2;
        QPoint points[3] = {
            QPoint(width() - 10 - 34, baseY - 3),
            QPoint(width() - 4 - 34, baseY - 3),
            QPoint(width() - 7 - 34, baseY) };
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

Q_DECLARE_METATYPE(Solid::PowerManagement::SleepState)

KSMShutdownDlg::KSMShutdownDlg( QWidget* parent,
                                bool maysd, bool choose, KWorkSpace::ShutdownType sdtype )
  : QDialog( parent, Qt::Popup ), //krazy:exclude=qclasses
    m_lastButton(0),
    m_btnLogout(0),
    m_btnHalt(0),
    m_btnReboot(0),
    m_automaticallyDoSeconds(30),
    m_pictureWidth(0)
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
    XSetWMProperties( QX11Info::display(), winId(), NULL, NULL, NULL, 0, NULL, NULL, &class_hint );
    XChangeProperty( QX11Info::display(), winId(),
        XInternAtom( QX11Info::display(), "WM_WINDOW_ROLE", False ), XA_STRING, 8, PropModeReplace,
        (unsigned char *)"logoutdialog", strlen( "logoutdialog" ));

//#endif
    m_svg = new Plasma::FrameSvg(this);
    m_svg->setImagePath("dialogs/shutdowndialog");
    connect( m_svg, SIGNAL(repaintNeeded()), this, SLOT(update()) );
    setModal( true );

    QVBoxLayout *mainLayout = new QVBoxLayout();

    qreal left, top, right, bottom;
    m_svg->getMargins(left, top, right, bottom);
    //not in framesvg mode
    if (left == 0) {
        mainLayout->setContentsMargins(12, 9, 12, 7);
    } else {
        mainLayout->setContentsMargins(left, top, right, bottom);
    }

    QVBoxLayout *buttonLayout = new QVBoxLayout();
    QHBoxLayout *buttonMainLayout = new QHBoxLayout();

    m_automaticallyDoLabel = new QLabel(this);
    mainLayout->addWidget(m_automaticallyDoLabel, 0, Qt::AlignRight);

    buttonMainLayout->addLayout(buttonLayout);

    QHBoxLayout *bottomLayout = new QHBoxLayout();

    QFont fnt;
    fnt.setPixelSize(16);
    QColor fntColor = Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor);
    QPalette palette;
    palette.setColor(QPalette::WindowText, fntColor);

    if ( choose || sdtype == KWorkSpace::ShutdownTypeNone ) {
        m_btnLogout = new KSMPushButton( i18n("&Logout"), this );
        m_btnLogout->setPixmap(KIconLoader::global()->loadIcon("system-log-out", KIconLoader::NoGroup, 32));
        if ( sdtype == KWorkSpace::ShutdownTypeNone )
            m_btnLogout->setFocus();
        connect(m_btnLogout, SIGNAL(clicked()), SLOT(slotLogout()));
        buttonLayout->addWidget(m_btnLogout, Qt::AlignRight | Qt::AlignTop);
    }

    if (maysd) {
        // Shutdown

        if ( choose || sdtype == KWorkSpace::ShutdownTypeHalt ) {
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
            QSet< Solid::PowerManagement::SleepState > spdMethods = Solid::PowerManagement::supportedSleepStates();
            if( spdMethods.contains(Solid::PowerManagement::StandbyState) ) {
                QAction* action = new QAction(i18n("&Standby"), spdActionGroup);
                action->setData(QVariant::fromValue(Solid::PowerManagement::StandbyState));
            }
            if( spdMethods.contains(Solid::PowerManagement::SuspendState) ) {
                QAction* action = new QAction(i18n("Suspend to &RAM"), spdActionGroup);
                action->setData(QVariant::fromValue(Solid::PowerManagement::SuspendState));
            }
            if( spdMethods.contains(Solid::PowerManagement::HibernateState) ) {
                QAction* action = new QAction(i18n("Suspend to &Disk"), spdActionGroup);
                action->setData(QVariant::fromValue(Solid::PowerManagement::HibernateState));
            }
            shutdownMenu->addActions(spdActionGroup->actions());
        }

        if ( choose || sdtype == KWorkSpace::ShutdownTypeReboot ) {
            // Reboot
            m_btnReboot = new KSMPushButton( i18n("&Restart Computer"), this );
            m_btnReboot->setPixmap(KIconLoader::global()->loadIcon("system-reboot", KIconLoader::NoGroup, 32));
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
                for (QStringList::ConstIterator it = rebootOptions.constBegin(); it != rebootOptions.constEnd(); ++it, ++index) {
                    QString label = (*it);
                    label=label.replace('&',"&&");
                    QAction* action = new QAction(label, rebootActionGroup);
                    action->setData(index);
                    if (index == cur) {
                        action->setText( label + i18nc("default option in boot loader", " (default)") );
                    }
                }
                rebootMenu->addActions(rebootActionGroup->actions());
            }
        }
    }

    btnBack = new KSMPushButton(i18n("&Cancel"), this, true);
    btnBack->setPixmap(KIconLoader::global()->loadIcon( "dialog-cancel", KIconLoader::NoGroup, 16));

    m_automaticallyDoLabel->setPalette(palette);
    fnt.setPixelSize(11);
    m_automaticallyDoLabel->setFont(fnt);
    automaticallyDoTimeout();

    QTimer *automaticallyDoTimer = new QTimer(this);
    connect(automaticallyDoTimer, SIGNAL(timeout()), this, SLOT(automaticallyDoTimeout()));
    automaticallyDoTimer->start(1000);

    bottomLayout->addStretch();
    bottomLayout->addWidget(btnBack);
    connect(btnBack, SIGNAL(clicked()), SLOT(reject()));

    mainLayout->addLayout(buttonMainLayout);
    mainLayout->addSpacing(9);
    mainLayout->addLayout(bottomLayout);

    setLayout(mainLayout);
    adjustSize();
    if (m_svg->hasElement("picture")) {
        QRect pictRect = m_svg->elementRect("picture").toRect();

        if (pictRect.height() < 1 || pictRect.width() < 1) {
            m_pictureWidth = 0;
        } else if (height() > width()) {
            m_pictureWidth = width();
        } else {
            m_svg->isValid();
            m_pictureWidth = mainLayout->sizeHint().height() * (pictRect.width() / pictRect.height());
            //kDebug() << "blurk!" << buttonMainLayout->sizeHint().height() << pictRect;
        }

        //kDebug() << width() << m_pictureWidth;
        //FIXME: this spaces will be taken from framesvg borders
        if (m_pictureWidth > 0) {
            const int extraSpace = 18;
            buttonMainLayout->insertSpacing(0, m_pictureWidth + extraSpace);
        }
        //resize(width() + m_pictureWidth, height());
        //kDebug() << width();
    } else {
        m_pictureWidth = 0;
    }

    KDialog::centerOnScreen(this, -3);
}

void KSMShutdownDlg::automaticallyDoTimeout()
{
    QPushButton *focusedButton = qobject_cast<QPushButton *>(focusWidget());
    if (focusedButton != m_lastButton) {
        m_lastButton = focusedButton;
        m_automaticallyDoSeconds = 30;
    }
    if (focusedButton) {
        if (m_automaticallyDoSeconds <= 0) { // timeout is at 0, do selected action
                focusedButton->click();
        // following code is required to provide a clean way to translate strings
        } else if (focusedButton == m_btnLogout) {
            m_automaticallyDoLabel->setText(i18np("Logging out in 1 second.",
                                            "Logging out in %1 seconds.", m_automaticallyDoSeconds));
        } else if (focusedButton == m_btnHalt) {
                m_automaticallyDoLabel->setText(i18np("Turning off computer in 1 second.",
                                                      "Turning off computer in %1 seconds.", m_automaticallyDoSeconds));
        } else if (focusedButton == m_btnReboot) {
                m_automaticallyDoLabel->setText(i18np("Restarting computer in 1 second.",
                                                      "Restarting computer in %1 seconds.", m_automaticallyDoSeconds));
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

    if (m_svg->hasElement("center")) {
        m_svg->resizeFrame(size());
        m_svg->paintFrame(&p);
    } else {
        m_svg->paint(&p, QRect(0, 0, width(), height()), "background");
    }

    if (m_pictureWidth > 0) { // implies hasElement("picture")
        QRect r = layout()->geometry();
        r.setWidth(m_pictureWidth);

	m_svg->resize();
        m_svg->resize(m_svg->elementRect("picture").size());
        QPixmap picture = m_svg->pixmap("picture");
        m_svg->resize();

        //kDebug() << 1 << r << picture.size();
        if (r.width() < picture.width()) {
            picture = picture.scaledToWidth(r.width(), Qt::SmoothTransformation);
        }

        if (r.height() < picture.height()) {
            picture = picture.scaledToHeight(r.height(), Qt::SmoothTransformation);
        }


        int left = (r.height() - picture.height())/2;
        if (QApplication::isLeftToRight()) {
            r.moveLeft(left);
        } else {
            r.moveRight(layout()->geometry().width() - left);
        }

        //kDebug() << 2 << r << picture.size();
        QRect dest = picture.rect();
        dest.moveCenter(r.center());
        p.setCompositionMode( QPainter::CompositionMode_SourceOver );
        p.drawPixmap(dest, picture, picture.rect());
    }
}

void KSMShutdownDlg::resizeEvent(QResizeEvent *e)
{
    QDialog::resizeEvent( e );

    if( KWindowSystem::compositingActive()) {
        clearMask();
    } else {
        setMask(m_svg->mask());
    }

    KDialog::centerOnScreen(this, -3);
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
    Solid::PowerManagement::SleepState spdMethod = action->data().value<Solid::PowerManagement::SleepState>();
    QDBusMessage call;
    switch (spdMethod) {
        case Solid::PowerManagement::StandbyState:
        case Solid::PowerManagement::SuspendState:
            call = QDBusMessage::createMethodCall("org.kde.Solid.PowerManagement",
                                                  "/org/kde/Solid/PowerManagement",
                                                  "org.kde.Solid.PowerManagement",
                                                  "suspendToRam");
            break;
        case Solid::PowerManagement::HibernateState:
            call = QDBusMessage::createMethodCall("org.kde.Solid.PowerManagement",
                                                  "/org/kde/Solid/PowerManagement",
                                                  "org.kde.Solid.PowerManagement",
                                                  "suspendToDisk");
            break;
    }
    QDBusConnection::sessionBus().asyncCall(call);
    reject();
}

bool KSMShutdownDlg::confirmShutdown(
        bool maysd, bool choose, KWorkSpace::ShutdownType& sdtype, QString& bootOption )
{
    KSMShutdownDlg* l = new KSMShutdownDlg( 0,
                                            //KSMShutdownFeedback::self(),
                                            maysd, choose, sdtype );
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
