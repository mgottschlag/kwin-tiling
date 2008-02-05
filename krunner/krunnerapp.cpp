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

#include <QProcess>
#include <QObject>
#include <QTimer>
#include <QtDBus/QtDBus>
#include <QLineEdit>

#include <KAction>
#include <KActionCollection>
#include <KDialog>
#include <KAuthorized>
#include <KGlobalAccel>
#include <KGlobalSettings>
#include <KLocale>
#include <KMessageBox>
#include <KWindowSystem>

#include "processui/ksysguardprocesslist.h"

#include "appadaptor.h"
#include "kworkspace.h"
#include "interfaceadaptor.h"
#include "interface.h"
#include "startupid.h"
#include "klaunchsettings.h"

#include <X11/extensions/Xrender.h>

Display* dpy = 0;
Colormap colormap = 0;
Visual *visual = 0;

void checkComposite()
{
    // thanks to zack rusin and frederik for pointing me in the right direction
    // for the following bits of X11 code
    dpy = XOpenDisplay(0); // open default display
    if (!dpy)
    {
        kError() << "Cannot connect to the X server" << endl;
        return;
    }

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
}

KRunnerApp* KRunnerApp::self()
{
    if (!kapp) {
        checkComposite();
        return new KRunnerApp(dpy, visual ? Qt::HANDLE(visual) : 0, colormap ? Qt::HANDLE(colormap) : 0);
    }

    return qobject_cast<KRunnerApp*>(kapp);
}

KRunnerApp::KRunnerApp(Display *display, Qt::HANDLE visual, Qt::HANDLE colormap)
    : RestartingApplication(display, visual, colormap),
      m_interface(0),
      m_tasks(0),
      m_startupId(NULL)
{
    initialize();
}

KRunnerApp::~KRunnerApp()
{
    delete m_interface;
}

void KRunnerApp::initialize()
{
    setQuitOnLastWindowClosed(false);
    initializeStartupNotification();
    m_interface = new Interface;

    // Global keys
    m_actionCollection = new KActionCollection( m_interface );
    KAction* a = 0;

    if ( KAuthorized::authorizeKAction( "run_command" ) ) {
        a = m_actionCollection->addAction( I18N_NOOP("Run Command") );
        a->setText( i18n( I18N_NOOP( "Run Command" ) ) );
        a->setGlobalShortcut(KShortcut(Qt::ALT+Qt::Key_F2));
        connect( a, SIGNAL(triggered(bool)), m_interface, SLOT(display()) );
    }

    a = m_actionCollection->addAction( I18N_NOOP( "Show Taskmanager" ) );
    a->setText( i18n( I18N_NOOP( "Show Taskmanager" ) ) );
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
    connect(a, SIGNAL(triggered(bool)), m_interface, SLOT(switchUser()));

    if ( KAuthorized::authorizeKAction( "lock_screen" ) ) {
        a = m_actionCollection->addAction( I18N_NOOP( "Lock Session" ) );
        a->setText( i18n( I18N_NOOP( "Lock Session" ) ) );
        a->setGlobalShortcut( KShortcut( Qt::ALT+Qt::CTRL+Qt::Key_L ) );
        connect( a, SIGNAL(triggered(bool)), &m_saver, SLOT(Lock()) );
    }

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
    if( !KLaunchSettings::busyCursor() ) {
        delete m_startupId;
        m_startupId = NULL;
    } else {
        if( m_startupId == NULL ) {
            m_startupId = new StartupId;
        }

        m_startupId->configure();
    }
}

/*TODO: fixme - move to kwin
void KRunnerApp::showWindowList()
{
     //KRootWm::self()->slotWindowList();
}
*/

void KRunnerApp::showTaskManager()
{
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
        bool keepAbove = cg.readEntry("KeepAbove", true);
	if(keepAbove) {
            KWindowSystem::setState( m_tasks->winId(), NET::KeepAbove );
	}

    } else
        w = static_cast<KSysGuardProcessList *> (m_tasks->mainWidget());


    m_tasks->show();
    m_tasks->raise();
    KWindowSystem::setOnDesktop(m_tasks->winId(), KWindowSystem::currentDesktop());
    KWindowSystem::forceActiveWindow(m_tasks->winId());

    if(w) w->filterLineEdit()->setFocus();
}

void KRunnerApp::taskDialogFinished()
{
    KConfigGroup cg = KGlobal::config()->group("TaskDialog");
    m_tasks->saveDialogSize(cg);
    KSysGuardProcessList *w = static_cast<KSysGuardProcessList *> (m_tasks->mainWidget());
    if(w) w->saveSettings(cg);
    // Since we default to forcing the window to be KeepAbove, if the user turns this off, remember this
    bool keepAbove = KWindowSystem::windowInfo(m_tasks->winId(), NET::WMState).hasState(NET::KeepAbove);
    if(keepAbove)
        cg.revertToDefault("KeepAbove");
    else
        cg.writeEntry("KeepAbove", false);

    KGlobal::config()->sync();
    m_tasks->deleteLater();
    m_tasks = 0;
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
    if ( !KWorkSpace::requestShutDown( confirm, sdtype ) ) {
        // TODO: should we show these errors in Interface?
        KMessageBox::error( 0, i18n("Could not log out properly.\nThe session manager cannot "
                                    "be contacted. You can try to force a shutdown by pressing "
                                    "Ctrl+Alt+Backspace; note, however, that your current session "
                                    "will not be saved with a forced shutdown." ) );
    }
}

int KRunnerApp::newInstance()
{
    static bool firstTime = true;
    if ( firstTime ) {
        firstTime = false;
    } else {
        m_interface->display();
    }

    return RestartingApplication::newInstance();
    //return 0;
}

bool KRunnerApp::hasCompositeManager() const
{
    return colormap && KWindowSystem::compositingActive();
}

#include "krunnerapp.moc"
