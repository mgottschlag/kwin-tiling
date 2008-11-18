/*
 *   Copyright (C) 2006 Aaron Seigo <aseigo@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2 as
 *   published by the Free Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "krunnerapp.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <QClipboard>
#include <QObject>
#include <QProcess>
#include <QTimer>
#include <QtDBus/QtDBus>
#include <QLineEdit>

#include <KAction>
#include <KActionCollection>
#include <KCrash>
#include <KDialog>
#include <KAuthorized>
#include <KGlobalAccel>
#include <KGlobalSettings>
#include <KLocale>
#include <KMessageBox>
#include <KWindowSystem>

#include <Plasma/RunnerManager>

#include "processui/ksysguardprocesslist.h"

#include "appadaptor.h"
#include "kworkspace.h"
#include "interfaces/default/interface.h"
#include "interfaces/quicksand/qs_dialog.h"
#ifdef Q_WS_X11
#include "startupid.h"
#endif
#include "klaunchsettings.h"
#include "krunnersettings.h"

#ifdef Q_WS_X11
#include <X11/extensions/Xrender.h>

Display* dpy = 0;
Colormap colormap = 0;
Visual *visual = 0;
#endif

void checkComposite()
{
#ifdef Q_WS_X11
    // thanks to zack rusin and frederik for pointing me in the right direction
    // for the following bits of X11 code
    dpy = XOpenDisplay(0); // open default display
    if (!dpy)
    {
        kError() << "Cannot connect to the X server";
        return;
    }
    if( qgetenv( "KDE_SKIP_ARGB_VISUALS" ) == "1" )
        return;

    int screen = DefaultScreen(dpy);
    int eventBase, errorBase;

    if (XRenderQueryExtension(dpy, &eventBase, &errorBase))
    {
        int nvi;
        XVisualInfo templ;
        templ.screen  = screen;
        templ.depth   = 32;
        templ.c_class = TrueColor;
        XVisualInfo *xvi = XGetVisualInfo(dpy, VisualScreenMask |
                                                VisualDepthMask |
                                                VisualClassMask,
                                            &templ, &nvi);
        for (int i = 0; i < nvi; ++i)
        {
            XRenderPictFormat *format = XRenderFindVisualFormat(dpy,
                                                                xvi[i].visual);
            if (format->type == PictTypeDirect && format->direct.alphaMask)
            {
                visual = xvi[i].visual;
                colormap = XCreateColormap(dpy, RootWindow(dpy, screen),
                                            visual, AllocNone);
                break;
            }
        }

    }
#endif
}

KRunnerApp* KRunnerApp::self()
{
    if (!kapp) {
        checkComposite();
#ifdef Q_WS_X11
        return new KRunnerApp(dpy, visual ? Qt::HANDLE(visual) : 0, colormap ? Qt::HANDLE(colormap) : 0);
#else
        return new KRunnerApp(0, 0, 0);
#endif
    }

    return qobject_cast<KRunnerApp*>(kapp);
}

KRunnerApp::KRunnerApp(Display *display, Qt::HANDLE visual, Qt::HANDLE colormap)
#ifdef Q_WS_X11
    : KUniqueApplication(display, visual, colormap),
#else
    : KUniqueApplication(),
#endif
      m_interface(0),
      m_tasks(0),
      m_startupId(NULL)
{
    initialize();
    connect(this, SIGNAL(aboutToQuit()), this, SLOT(cleanUp()));
}

KRunnerApp::~KRunnerApp()
{
}

void KRunnerApp::cleanUp()
{
    disconnect(KRunnerSettings::self(), SIGNAL(configChanged()), this, SLOT(reloadConfig()));
    delete m_interface;
    m_interface = 0;
    delete m_runnerManager;
    m_runnerManager = 0;
}

void KRunnerApp::initialize()
{
    setWindowIcon(KIcon("system-run"));

    setQuitOnLastWindowClosed(false);
    KCrash::setFlags(KCrash::AutoRestart);
    initializeStartupNotification();

    connect(KRunnerSettings::self(), SIGNAL(configChanged()), this, SLOT(reloadConfig()));

    m_runnerManager = new Plasma::RunnerManager;
    m_runnerManager->reloadConfiguration(); // pre-load the runners

    switch (KRunnerSettings::interface()) {
        default:
        case KRunnerSettings::EnumInterface::CommandOriented:
            m_interface = new Interface(m_runnerManager);
            break;
        case KRunnerSettings::EnumInterface::TaskOriented:
            m_interface = new QsDialog(m_runnerManager);
            break;
    }

#ifdef Q_WS_X11
    //FIXME: if argb visuals enabled Qt will always set WM_CLASS as "qt-subapplication" no matter what
    //the application name is we set the proper XClassHint here, hopefully won't be necessary anymore when
    //qapplication will manage apps with argvisuals in a better way
    XClassHint classHint;
    classHint.res_name = const_cast<char*>("krunner");
    classHint.res_class = const_cast<char*>("krunner");
    XSetClassHint(QX11Info::display(), m_interface->winId(), &classHint);
#endif

    // Global keys
    m_actionCollection = new KActionCollection(this);
    KAction* a = 0;

    if ( KAuthorized::authorizeKAction( "run_command" ) ) {
        a = m_actionCollection->addAction( I18N_NOOP("Run Command") );
        a->setText( i18n( I18N_NOOP( "Run Command" ) ) );
        a->setGlobalShortcut(KShortcut(Qt::ALT+Qt::Key_F2));
        connect( a, SIGNAL(triggered(bool)), SLOT(display()) );

        a = m_actionCollection->addAction( I18N_NOOP("Run Command on clipboard contents") );
        a->setText( i18n( I18N_NOOP( "Run Command on clipboard contents" ) ) );
        a->setGlobalShortcut(KShortcut(Qt::ALT+Qt::SHIFT+Qt::Key_F2));
        connect( a, SIGNAL(triggered(bool)), SLOT(displayWithClipboardContents()) );
    }

    a = m_actionCollection->addAction( I18N_NOOP( "Show System Activity" ) );
    a->setText( i18n( I18N_NOOP( "Show System Activity" ) ) );
    a->setGlobalShortcut( KShortcut( Qt::CTRL+Qt::Key_Escape ) );
    connect( a, SIGNAL(triggered(bool)), SLOT(showTaskManager()) );

/*
 * TODO: doesn't this belong in the window manager?
    a = m_actionCollection->addAction( I18N_NOOP( "Show Window List") );
    a->setText( i18n( I18N_NOOP( "Show Window List") ) );
    a->setGlobalShortcut( KShortcut( Qt::ALT+Qt::Key_F5 ) );
    connect( a, SIGNAL(triggered(bool)), SLOT(slotShowWindowList()) );
*/
    a = m_actionCollection->addAction( I18N_NOOP("Switch User") );
    a->setText( i18n( I18N_NOOP("Switch User") ) );
    a->setGlobalShortcut( KShortcut( Qt::ALT+Qt::CTRL+Qt::Key_Insert ) );
    connect(a, SIGNAL(triggered(bool)), SLOT(switchUser()));

#ifdef Q_WS_X11
    if ( KAuthorized::authorizeKAction( "lock_screen" ) ) {
        a = m_actionCollection->addAction( I18N_NOOP( "Lock Session" ) );
        a->setText( i18n( I18N_NOOP( "Lock Session" ) ) );
        a->setGlobalShortcut( KShortcut( Qt::ALT+Qt::CTRL+Qt::Key_L ) );
        connect( a, SIGNAL(triggered(bool)), &m_saver, SLOT(Lock()) );
    }
#endif

    if ( KAuthorized::authorizeKAction( "logout" ) ) {
        a = m_actionCollection->addAction( I18N_NOOP("Log Out") );
        a->setText( i18n(I18N_NOOP("Log Out")) );
        a->setGlobalShortcut(KShortcut(Qt::ALT+Qt::CTRL+Qt::Key_Delete));
        connect(a, SIGNAL(triggered(bool)), SLOT(logout()));

        a = m_actionCollection->addAction( I18N_NOOP("Log Out Without Confirmation") );
        a->setText( i18n(I18N_NOOP("Log Out Without Confirmation")) );
        a->setGlobalShortcut(KShortcut(Qt::ALT+Qt::CTRL+Qt::SHIFT+Qt::Key_Delete));
        connect(a, SIGNAL(triggered(bool)), SLOT(logoutWithoutConfirmation()));

        a = m_actionCollection->addAction( I18N_NOOP("Halt Without Confirmation") );
        a->setText( i18n(I18N_NOOP("Halt Without Confirmation")) );
        a->setGlobalShortcut(KShortcut(Qt::ALT+Qt::CTRL+Qt::SHIFT+Qt::Key_PageDown));
        connect(a, SIGNAL(triggered(bool)), SLOT(haltWithoutConfirmation()));

        a = m_actionCollection->addAction( I18N_NOOP("Reboot Without Confirmation") );
        a->setText( i18n(I18N_NOOP("Reboot Without Confirmation")) );
        a->setGlobalShortcut(KShortcut(Qt::ALT+Qt::CTRL+Qt::SHIFT+Qt::Key_PageUp));
        connect(a, SIGNAL(triggered(bool)), SLOT(rebootWithoutConfirmation()));
    }

    m_actionCollection->readSettings();

    new AppAdaptor(this);
    QDBusConnection::sessionBus().registerObject( "/App", this );
} // end void KRunnerApp::initializeBindings

void KRunnerApp::initializeStartupNotification()
{
    // Startup notification
    KLaunchSettings::self()->readConfig();
#ifdef Q_WS_X11
    if( !KLaunchSettings::busyCursor() ) {
        delete m_startupId;
        m_startupId = NULL;
    } else {
        if( m_startupId == NULL ) {
            m_startupId = new StartupId;
        }

        m_startupId->configure();
    }
#endif
}

/*TODO: fixme - move to kwin
void KRunnerApp::showWindowList()
{
     //KRootWm::self()->slotWindowList();
}
*/

void KRunnerApp::showTaskManager()
{
#ifndef Q_WS_WIN
    //kDebug(1204) << "Launching KSysGuard...";
    KSysGuardProcessList* w = NULL;
    if (!m_tasks) {
        //TODO: move this dialog into its own KDialog subclass
        //      add an expander widget (as seen in the main
        //      krunner window when options get shown)
        //      and put some basic feedback plasmoids there
        //      BLOCKEDBY: said plasmoids and the dataengine
        //                 currently being worked on, so the
        //                 wait shouldn't be too long =)

        m_tasks = new KDialog(0);
        m_tasks->setWindowTitle(i18n("System Activity"));
        m_tasks->setWindowIcon(KIcon("utilities-system-monitor"));
        connect(m_tasks, SIGNAL(finished()),
                this, SLOT(taskDialogFinished()));
        m_tasks->setButtons(KDialog::Close);
        w = new KSysGuardProcessList(m_tasks);
        m_tasks->setMainWidget(w);

        m_tasks->setInitialSize(QSize(650, 420));
        KConfigGroup cg = KGlobal::config()->group("TaskDialog");
        m_tasks->restoreDialogSize(cg);
        w->loadSettings(cg);
        // Since we default to forcing the window to be KeepAbove, if the user turns this off, remember this
        bool keepAbove = KRunnerSettings::keepTaskDialogAbove();
        if (keepAbove) {
            KWindowSystem::setState( m_tasks->winId(), NET::KeepAbove );
        }

    } else
        w = static_cast<KSysGuardProcessList *> (m_tasks->mainWidget());


    m_tasks->show();
    m_tasks->raise();
    KWindowSystem::setOnDesktop(m_tasks->winId(), KWindowSystem::currentDesktop());
    KWindowSystem::forceActiveWindow(m_tasks->winId());

    if (w) {
        w->filterLineEdit()->setFocus();
    }
#endif
}

void KRunnerApp::display()
{
    m_interface->display();
}

void KRunnerApp::query(const QString &term)
{
    m_interface->display(term);
}

void KRunnerApp::displayWithClipboardContents()
{
   QString clipboardData = QApplication::clipboard()->text(QClipboard::Selection);
   m_interface->display(clipboardData);
}

void KRunnerApp::switchUser()
{
    m_interface->switchUser();
}

void KRunnerApp::clearHistory()
{
    m_interface->clearHistory();
}

void KRunnerApp::taskDialogFinished()
{
#ifndef Q_WS_WIN  
    KConfigGroup cg = KGlobal::config()->group("TaskDialog");
    m_tasks->saveDialogSize(cg);
    KSysGuardProcessList *w = static_cast<KSysGuardProcessList *> (m_tasks->mainWidget());
    if (w) {
        w->saveSettings(cg);
    }

    // Since we default to forcing the window to be KeepAbove, if the user turns this off, remember this
    bool keepAbove = KWindowSystem::windowInfo(m_tasks->winId(), NET::WMState).hasState(NET::KeepAbove);
    KRunnerSettings::setKeepTaskDialogAbove(keepAbove);
    KGlobal::config()->sync();

    m_tasks->deleteLater();
    m_tasks = 0;
#endif
}

void KRunnerApp::logout()
{
    logout( KWorkSpace::ShutdownConfirmDefault,
            KWorkSpace::ShutdownTypeDefault );
}

void KRunnerApp::logoutWithoutConfirmation()
{
    logout( KWorkSpace::ShutdownConfirmNo,
            KWorkSpace::ShutdownTypeNone );
}

void KRunnerApp::haltWithoutConfirmation()
{
    logout( KWorkSpace::ShutdownConfirmNo,
            KWorkSpace::ShutdownTypeHalt );
}

void KRunnerApp::rebootWithoutConfirmation()
{
    logout( KWorkSpace::ShutdownConfirmNo,
            KWorkSpace::ShutdownTypeReboot );
}

void KRunnerApp::logout( KWorkSpace::ShutdownConfirm confirm,
                       KWorkSpace::ShutdownType sdtype )
{
#ifndef Q_WS_WIN
    if ( !KWorkSpace::requestShutDown( confirm, sdtype ) ) {
        // TODO: should we show these errors in Interface?
        KMessageBox::error( 0, i18n("Could not log out properly.\nThe session manager cannot "
                                    "be contacted. You can try to force a shutdown by pressing "
                                    "Ctrl+Alt+Backspace; note, however, that your current session "
                                    "will not be saved with a forced shutdown." ) );
    }
#endif
}

int KRunnerApp::newInstance()
{
    static bool firstTime = true;
    if ( firstTime ) {
        firstTime = false;
    } else {
        m_interface->display();
    }

    return KUniqueApplication::newInstance();
    //return 0;
}

bool KRunnerApp::hasCompositeManager() const
{
#ifdef Q_WS_X11
    return colormap && KWindowSystem::compositingActive();
#else
    return false;
#endif
}

void KRunnerApp::reloadConfig()
{
    //Prevent Interface destructor from triggering this method
    disconnect(KRunnerSettings::self(), SIGNAL(configChanged()), this, SLOT(reloadConfig()));

    int interface = KRunnerSettings::interface();
    if (qobject_cast<Interface*>(m_interface) &&
        interface == KRunnerSettings::EnumInterface::TaskOriented) {
        delete m_interface;
        m_interface = new QsDialog(m_runnerManager);
        m_interface->display();
    } else if (interface == KRunnerSettings::EnumInterface::CommandOriented) {
        delete m_interface;
        m_interface = new Interface(m_runnerManager);
        m_interface->display();
    }

    connect(KRunnerSettings::self(), SIGNAL(configChanged()), this, SLOT(reloadConfig()));
}

#include "krunnerapp.moc"
