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

#include <QTimer>

#include <kaction.h>
#include <kglobalaccel.h>
#include <kauthorized.h>
#include <kglobalsettings.h>
#include <kapplication.h>
#include <klocale.h>
#include <KActionCollection>
#include <KMessageBox>
#include <k3process.h>
#include <QObject>
#include <QtDBus/QtDBus>

#include "kworkspace.h"
#include "interfaceadaptor.h"
#include "interface.h"
#include "startupid.h"
#include "klaunchsettings.h"

#include <X11/extensions/Xrender.h>

Display* dpy = 0;
Colormap colormap = 0;
Visual *visual = 0;
bool argbVisual = false;

bool checkComposite()
{
    dpy = XOpenDisplay(0); // open default display
    if (!dpy)
    {
        kError() << "Cannot connect to the X server" << endl;
        return true;
    }

    KRunnerApp::s_haveCompositeManager = XGetSelectionOwner(dpy,
                                                            XInternAtom(dpy,
                                                                        "_NET_WM_CM_S0",
                                                                        false));

    if (KRunnerApp::s_haveCompositeManager)
    {
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
                    argbVisual = true;
                    break;
                }
            }
        }

        KRunnerApp::s_haveCompositeManager = argbVisual;
    }

    kDebug() << "KRunnerApp::s_haveCompositeManager: " << KRunnerApp::s_haveCompositeManager << endl;
    return true;
}
/*
KRunnerApp::KRunnerApp(Display *display,
                       Qt::HANDLE visual,
                       Qt::HANDLE colormap)
    : RestartingApplication(display, visual, colormap ),
      m_interface( 0 )
{
    kDebug() << "new krunner app" << endl;
    initialize();
}*/

KRunnerApp::KRunnerApp()
    : RestartingApplication(checkComposite() ? dpy : dpy, dpy ? Qt::HANDLE(visual) : 0, dpy ? Qt::HANDLE(colormap) : 0),
      m_interface( 0 )
{
    kDebug() << "new simple krunner app " << dpy << " " << argbVisual << endl;
    initialize();
}

KRunnerApp::~KRunnerApp()
{
    delete m_interface;
}

void KRunnerApp::initialize()
{
    // Startup notification
    KLaunchSettings::self()->readConfig();
    StartupId *startup_id( NULL );
    if( !KLaunchSettings::busyCursor() ) {
        delete startup_id;
        startup_id = NULL;
    } else {
        if( startup_id == NULL ) {
            startup_id = new StartupId;
        }

        startup_id->configure();
    }

    kDebug() << "initliaze!()" << endl;
    m_interface = new Interface;

    // Global keys
    m_actionCollection = new KActionCollection( m_interface );
    QAction* a = 0;

    if ( KAuthorized::authorizeKAction( "run_command" ) ) {
        a = m_actionCollection->addAction( I18N_NOOP("Run Command") );
        a->setText( i18n( I18N_NOOP( "Run Command" ) ) );
        qobject_cast<KAction*>( a )->setGlobalShortcut(KShortcut(Qt::ALT+Qt::Key_F2));
        connect( a, SIGNAL(triggered(bool)), m_interface, SLOT(display()) );
/*
        QDialog* test = new QDialog;
        connect( a, SIGNAL(triggered(bool)), test, SLOT(show()) );
*/
    }

    a = m_actionCollection->addAction( I18N_NOOP( "Show Taskmanager" ) );
    a->setText( i18n( I18N_NOOP( "Show Taskmanager" ) ) );
    qobject_cast<KAction*>( a )->setGlobalShortcut( KShortcut( Qt::CTRL+Qt::Key_Escape ) );
    connect( a, SIGNAL(triggered(bool)), SLOT(showTaskManager()) );

/*
 * TODO: doesn't this belong in the window manager?
    a = m_actionCollection->addAction( I18N_NOOP( "Show Window List") );
    a->setText( i18n( I18N_NOOP( "Show Window List") ) );
    qobject_cast<KAction*>( a )->setGlobalShortcut( KShortcut( Qt::ALT+Qt::Key_F5 ) );
    connect( a, SIGNAL(triggered(bool)), SLOT(slotShowWindowList()) );
*/
    a = m_actionCollection->addAction( I18N_NOOP("Switch User") );
    a->setText( i18n( I18N_NOOP("Switch User") ) );
    qobject_cast<KAction*>( a )->setGlobalShortcut( KShortcut( Qt::ALT+Qt::CTRL+Qt::Key_Insert ) );
    connect(a, SIGNAL(triggered(bool)), m_interface, SLOT(switchUser()));

    if ( KAuthorized::authorizeKAction( "lock_screen" ) ) {
        a = m_actionCollection->addAction( I18N_NOOP( "Lock Session" ) );
        a->setText( i18n( I18N_NOOP( "Lock Session" ) ) );
        qobject_cast<KAction*>( a )->setGlobalShortcut( KShortcut( Qt::ALT+Qt::CTRL+Qt::Key_L ) );
        connect( a, SIGNAL(triggered(bool)), &m_saver, SLOT(Lock()) );
    }

    if ( KAuthorized::authorizeKAction( "logout" ) ) {
        a = m_actionCollection->addAction( I18N_NOOP("Log Out") );
        a->setText( i18n(I18N_NOOP("Log Out")) );
        qobject_cast<KAction*>( a )->setGlobalShortcut(KShortcut(Qt::ALT+Qt::CTRL+Qt::Key_Delete));
        connect(a, SIGNAL(triggered(bool)), SLOT(logout()));

        a = m_actionCollection->addAction( I18N_NOOP("Log Out Without Confirmation") );
        a->setText( i18n(I18N_NOOP("Log Out Without Confirmation")) );
        qobject_cast<KAction*>( a )->setGlobalShortcut(KShortcut(Qt::ALT+Qt::CTRL+Qt::SHIFT+Qt::Key_Delete));
        connect(a, SIGNAL(triggered(bool)), SLOT(logoutWithoutConfirmation()));

        a = m_actionCollection->addAction( I18N_NOOP("Halt without Confirmation") );
        a->setText( i18n(I18N_NOOP("Halt without Confirmation")) );
        qobject_cast<KAction*>( a )->setGlobalShortcut(KShortcut(Qt::ALT+Qt::CTRL+Qt::SHIFT+Qt::Key_PageDown));
        connect(a, SIGNAL(triggered(bool)), SLOT(haltWithoutConfirmation()));

        a = m_actionCollection->addAction( I18N_NOOP("Reboot without Confirmation") );
        a->setText( i18n(I18N_NOOP("Reboot without Confirmation")) );
        qobject_cast<KAction*>( a )->setGlobalShortcut(KShortcut(Qt::ALT+Qt::CTRL+Qt::SHIFT+Qt::Key_PageUp));
        connect(a, SIGNAL(triggered(bool)), SLOT(rebootWithoutConfirmation()));
    }

    m_actionCollection->readSettings();

} // end void KRunnerApp::initializeBindings


/*TODO: fixme - move to kwin
void KRunnerApp::showWindowList()
{
     //KRootWm::self()->slotWindowList();
}
*/

void KRunnerApp::showTaskManager()
{
    //kDebug(1204) << "Launching KSysGuard..." << endl;
    K3Process* p = new K3Process;
    Q_CHECK_PTR(p);

    *p << "ksysguard";
    *p << "--showprocesses";

    p->start(K3Process::DontCare);

    delete p;
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

#include <KStartupInfo>
int KRunnerApp::newInstance()
{
    static bool firstTime = true;
    if ( firstTime ) {
        // App startup: do nothing
        firstTime = false;
    } else {
        //KStartupInfo::setNewStartupId( m_interface, KStartupInfo::createNewStartupId() );
        m_interface->display();
        //kDebug() << "startup id is " << startupId() << endl;
    }

    return RestartingApplication::newInstance();
    //return 0;
}

#include "krunnerapp.moc"
