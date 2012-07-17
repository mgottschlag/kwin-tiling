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

#include <QApplication>
#include <QDesktopWidget>
#include <QTimer>
#include <QFile>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusMessage>
#include <QtDBus/QDBusPendingCall>
#include <QDeclarativeView>
#include <QDeclarativeContext>
#include <QDeclarativeEngine>
#include <QDeclarativePropertyMap>

#include <kdialog.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kuser.h>
#include <Solid/PowerManagement>
#include <kwindowsystem.h>
#include <netwm.h>
#include <KStandardDirs>
#include <kdeclarative.h>

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

Q_DECLARE_METATYPE(Solid::PowerManagement::SleepState)

KSMShutdownDlg::KSMShutdownDlg( QWidget* parent,
                                bool maysd, bool choose, KWorkSpace::ShutdownType sdtype,
                                const QString& theme)
  : QDialog( parent, Qt::Popup ) //krazy:exclude=qclasses
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

    KDialog::centerOnScreen(this, -3);

    //kDebug() << "Creating QML view";
    m_view = new QDeclarativeView(this);
    QDeclarativeContext *context = m_view->rootContext();
    context->setContextProperty("maysd", maysd);
    context->setContextProperty("choose", choose);
    context->setContextProperty("sdtype", sdtype);

    QDeclarativePropertyMap *mapShutdownType = new QDeclarativePropertyMap(this);
    mapShutdownType->insert("ShutdownTypeDefault", QVariant::fromValue((int)KWorkSpace::ShutdownTypeDefault));
    mapShutdownType->insert("ShutdownTypeNone", QVariant::fromValue((int)KWorkSpace::ShutdownTypeNone));
    mapShutdownType->insert("ShutdownTypeReboot", QVariant::fromValue((int)KWorkSpace::ShutdownTypeReboot));
    mapShutdownType->insert("ShutdownTypeHalt", QVariant::fromValue((int)KWorkSpace::ShutdownTypeHalt));
    mapShutdownType->insert("ShutdownTypeLogout", QVariant::fromValue((int)KWorkSpace::ShutdownTypeLogout));
    context->setContextProperty("ShutdownType", mapShutdownType);

    QDeclarativePropertyMap *mapSpdMethods = new QDeclarativePropertyMap(this);
    QSet<Solid::PowerManagement::SleepState> spdMethods = Solid::PowerManagement::supportedSleepStates();
    mapSpdMethods->insert("StandbyState", QVariant::fromValue(spdMethods.contains(Solid::PowerManagement::StandbyState)));
    mapSpdMethods->insert("SuspendState", QVariant::fromValue(spdMethods.contains(Solid::PowerManagement::SuspendState)));
    mapSpdMethods->insert("HibernateState", QVariant::fromValue(spdMethods.contains(Solid::PowerManagement::HibernateState)));
    context->setContextProperty("spdMethods", mapSpdMethods);

    QString bootManager = KConfig(KDE_CONFDIR "/kdm/kdmrc", KConfig::SimpleConfig).group("Shutdown").readEntry("BootManager", "None");
    context->setContextProperty("bootManager", bootManager);

    QStringList options;
    int def, cur;
    if ( KDisplayManager().bootOptions( rebootOptions, def, cur ) ) {
        if ( cur > -1 ) {
            def = cur;
        }
    }
    QDeclarativePropertyMap *rebootOptionsMap = new QDeclarativePropertyMap(this);
    rebootOptionsMap->insert("options", QVariant::fromValue(rebootOptions));
    rebootOptionsMap->insert("default", QVariant::fromValue(def));
    context->setContextProperty("rebootOptions", rebootOptionsMap);

    setModal( true );

    // window stuff
    m_view->setFrameShape(QFrame::NoFrame);
    m_view->setWindowFlags(Qt::X11BypassWindowManagerHint);
    m_view->setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_TranslucentBackground);
    setStyleSheet("background:transparent;");
    QPalette pal = m_view->palette();
    pal.setColor(backgroundRole(), Qt::transparent);
    m_view->setPalette(pal);
    m_view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    // engine stuff
    foreach(const QString &importPath, KGlobal::dirs()->findDirs("module", "imports")) {
        m_view->engine()->addImportPath(importPath);
    }
    KDeclarative kdeclarative;
    kdeclarative.setDeclarativeEngine(m_view->engine());
    kdeclarative.initialize();
    kdeclarative.setupBindings();
    m_view->installEventFilter(this);

    QString fileName = KStandardDirs::locate("data", QString("ksmserver/themes/%1/main.qml").arg(theme));
    if (QFile::exists(fileName)) {
        //kDebug() << "Using QML theme" << fileName;
        m_view->setSource(QUrl::fromLocalFile(fileName));
    }
    QGraphicsObject *rootObject = m_view->rootObject();
    connect(rootObject, SIGNAL(logoutRequested()), SLOT(slotLogout()));
    connect(rootObject, SIGNAL(haltRequested()), SLOT(slotHalt()));
    connect(rootObject, SIGNAL(suspendRequested(int)), SLOT(slotSuspend(int)) );
    connect(rootObject, SIGNAL(rebootRequested()), SLOT(slotReboot()));
    connect(rootObject, SIGNAL(rebootRequested2(int)), SLOT(slotReboot(int)) );
    connect(rootObject, SIGNAL(cancelRequested()), SLOT(reject()));
    connect(rootObject, SIGNAL(lockScreenRequested()), SLOT(slotLockScreen()));
    m_view->show();
    m_view->setFocus();
    adjustSize();
}

void KSMShutdownDlg::resizeEvent(QResizeEvent *e)
{
    QDialog::resizeEvent( e );

    if( KWindowSystem::compositingActive()) {
        clearMask();
    } else {
        setMask(m_view->mask());
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

void KSMShutdownDlg::slotReboot(int opt)
{
    if (int(rebootOptions.size()) > opt)
        m_bootOption = rebootOptions[opt];
    m_shutdownType = KWorkSpace::ShutdownTypeReboot;
    accept();
}


void KSMShutdownDlg::slotLockScreen()
{
    m_bootOption.clear();
    QDBusMessage call = QDBusMessage::createMethodCall("org.kde.screensaver",
                                                       "/ScreenSaver",
                                                       "org.freedesktop.ScreenSaver",
                                                       "Lock");
    QDBusConnection::sessionBus().asyncCall(call);
    reject();
}

void KSMShutdownDlg::slotHalt()
{
    m_bootOption.clear();
    m_shutdownType = KWorkSpace::ShutdownTypeHalt;
    accept();
}


void KSMShutdownDlg::slotSuspend(int spdMethod)
{
    m_bootOption.clear();
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
        bool maysd, bool choose, KWorkSpace::ShutdownType& sdtype, QString& bootOption,
        const QString& theme)
{
    KSMShutdownDlg* l = new KSMShutdownDlg( 0,
                                            //KSMShutdownFeedback::self(),
                                            maysd, choose, sdtype, theme );
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
