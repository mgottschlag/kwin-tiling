//===========================================================================
//
// This file is part of the KDE project
//
// Copyright (c) 1999 Martin R. Jones <mjones@kde.org>
//

#include <ctype.h>
#include <unistd.h>
#include <pwd.h>
#include <sys/types.h>
#include <qlayout.h>
#include <qframe.h>
#include <qpushbutton.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kglobalsettings.h>
#include <kconfig.h>
#include <kiconloader.h>
#include <kdesu/defaults.h>
#include <kpassdlg.h>
#include <kdebug.h>
#include "lockdlg.h"
#include "lockdlg.moc"

#include <X11/Xutil.h>
#include <X11/keysym.h>

#define PASSDLG_HIDE_TIMEOUT        10000

//===========================================================================
//
// Simple dialog for entering a password.
//
PasswordDlg::PasswordDlg(QWidget *parent, bool nsess)
    : QDialog(parent, "password dialog", true, WStyle_Customize | WStyle_NoBorder)
{
    QFrame *winFrame = new QFrame( this );
    winFrame->setFrameStyle( QFrame::Panel | QFrame::Raised );
    winFrame->setLineWidth( 2 );

//    setFocusPolicy(StrongFocus);

    QGridLayout *layout = new QGridLayout(winFrame, 2, 3, 20, 10);
    layout->setResizeMode(QLayout::Minimum);
    layout->addColSpacing(1, 20);

    QLabel *pixlabel= new QLabel(winFrame);
    pixlabel->setPixmap(QPixmap(locate("data", "kdesktop/pics/ksslogo.png")));
    layout->addMultiCellWidget(pixlabel, 0, 1, 0, 0, QLayout::AlignTop);

    QFont font = KGlobalSettings::generalFont();

    mLabel = new QLabel(passwordQueryMsg(), winFrame);
    mLabel->setAlignment(AlignCenter);
    font.setPointSize(18);
    mLabel->setFont(font);
    mLabel->setFixedSize(mLabel->sizeHint());

    layout->addWidget(mLabel, 0, 2);

    mEntry = new KPasswordEdit( winFrame, "password edit" );
    font.setPointSize(16);
    mEntry->setFont(font);
    mEntry->installEventFilter(this);

    layout->addWidget(mEntry, 1, 2);

    if (nsess) {
	mButton = new QPushButton(i18n("\nStart\n&New\nSession\n"), winFrame, "button");
	layout->addMultiCellWidget(mButton, 0,1, 3,3, AlignCenter);
	connect(mButton, SIGNAL(clicked()), SIGNAL(startNewSession()));
	mButton->installEventFilter(this);
    } else
	mButton = 0;

    layout->activate();

    resize(layout->sizeHint());

    installEventFilter(this);

    mFailedTimerId = 0;
    mTimeoutTimerId = startTimer(PASSDLG_HIDE_TIMEOUT);

    connect(&mPassProc, SIGNAL(processExited(KProcess *)),
                        SLOT(passwordChecked(KProcess *)));
}

//---------------------------------------------------------------------------
//
// Fetch current user id, and return "Firstname Lastname (username)"
//
QString PasswordDlg::currentUser(void)
{
    struct passwd *current = getpwuid(getuid());
    QString fullname = QString::fromLocal8Bit(current->pw_gecos);
    if (fullname.find(',') != -1)
    {
        // Remove everything from and including first comma
        fullname.truncate(fullname.find(','));
    }

    QString username = QString::fromLocal8Bit(current->pw_name);

    return fullname + " (" + username + ")";
}

//---------------------------------------------------------------------------
//
// This returns the string to use to ask the user for their password.
//
QString PasswordDlg::passwordQueryMsg()
{
    return i18n("Enter Password") + "\n" + currentUser();
} 

//---------------------------------------------------------------------------
//
// Handle timer events.
//
void PasswordDlg::timerEvent(QTimerEvent *ev)
{
    if (ev->timerId() == mTimeoutTimerId)
    {
        reject();
    }
    else if (ev->timerId() == mFailedTimerId)
    {
        killTimer(mFailedTimerId);
        mFailedTimerId = 0;
        // Show the normal password prompt.
        mLabel->setText(passwordQueryMsg());
        mEntry->erase();
        mEntry->setEnabled(true);
        if( mButton )
            mButton->setEnabled(true);
    }
}

#undef KeyPress	/* i hate X #defines */

bool PasswordDlg::eventFilter( QObject *, QEvent *ev )
{
    if ( ev->type() == QEvent::KeyPress ) {
        killTimer(mTimeoutTimerId);
        mTimeoutTimerId = startTimer(PASSDLG_HIDE_TIMEOUT);
        QKeyEvent *e = (QKeyEvent *)ev;
        if ( ( e->state() == 0 &&
	       ( e->key() == Key_Enter || e->key() == Key_Return ) ) || 
             ( e->state() & Keypad && e->key() == Key_Enter ) ) {
            if ( focusWidget() == mButton )
                mButton->animateClick();
            else if ( focusWidget() == mEntry )
                startCheckPassword();
            return true;
        }
    } else if (ev->type() == QEvent::ContextMenu)
	return true;
    return false;
}

//---------------------------------------------------------------------------
//
// Starts the kcheckpass process to check the user's password.
//
// Serge Droz <serge.droz@pso.ch> 10.2000
// Define ACCEPT_ENV if you want to pass an environment variable to
// kcheckpass. Define ACCEPT_ARGS if you want to pass command line
// arguments to kcheckpass
#define ACCEPT_ENV
//#define ACCEPT_ARGS
void PasswordDlg::startCheckPassword()
{
    const char *passwd = mEntry->password();
    if (passwd && *passwd)
    {
        if( mButton )
            mButton->setEnabled(false);
        mEntry->setEnabled(false);

        QString kcp_binName = KStandardDirs::findExe("kcheckpass");

        mPassProc.clearArguments();
        mPassProc << kcp_binName;

#ifdef HAVE_PAM
# ifdef ACCEPT_ENV
        setenv("KDE_PAM_ACTION", KSCREENSAVER_PAM_SERVICE, 1);
# elif defined(ACCEPT_ARGS)
        mPassProc << "-c" << KSCREENSAVER_PAM_SERVICE;
# endif
#endif
	bool ret = mPassProc.start(KProcess::NotifyOnExit, KProcess::Stdin);
#ifdef HAVE_PAM
# ifdef ACCEPT_ENV
        unsetenv("KDE_PAM_ACTION");
# endif
#endif
	if (ret == false)
        {
            kdDebug(1204) << "kcheckpass failed to start" << endl;
            mLabel->setText(i18n("Verification failed\nKill kdesktop_lock"));
            mFailedTimerId = startTimer(10000);
            return;
        }

        // write Password to stdin
        mPassProc.writeStdin(passwd, strlen(passwd));
        mPassProc.closeStdin();
    }
}

//---------------------------------------------------------------------------
//
// The kcheckpass process has exited.
//
void PasswordDlg::passwordChecked(KProcess *proc)
{
    if (proc == &mPassProc)
    {
	    /* the exit codes of kcheckpass:
	       0: everything fine
		   1: authentification failed
		   2: passwd access failed [permissions/misconfig]
	    */
        if (mPassProc.normalExit() && !mPassProc.exitStatus())
        {
/*
XXX this needs to go into a separate routine at startup time
            stopSaver();
	    if ( mPassProc.exitStatus() == 2 )
	    {
		KMessageBox::error(0,
		  i18n( "<h1>Screen Locking Failed!</h1>"
		  "Your screen was not locked because the <i>kcheckpass</i> "
		  "program was not able to check your password. This is "
		  "usually the result of kcheckpass not being installed "
		  "correctly. If you installed KDE yourself, reinstall "
		  "kcheckpass as root. If you are using a pre-compiled "
		  "package, contact the packager." ),
		  i18n( "Screen Locking Failed" ) );
	    }
	    kapp->quit();
*/
	    accept();
        }
        else
        {
            mLabel->setText(i18n("Failed"));
            mFailedTimerId = startTimer(1500);
        }
    }
}

// see the comment at the top of lockprocess.cpp
// with certain unreasonable focus policies (focus under mouse, ehm),
// KWin interferes and doesn't make the dialog focused, so we have
// to focus it manually (moving mouse doesn't help because of mouse grab)
// the right fix is of course override_redirect, so KWin won't get in the way
// and we can call setActiveWindow() and setFocus() without waiting
void PasswordDlg::show()
{
    QDialog::show();
    QApplication::flushX();
    for(;;)
    { // wait for the window to get mapped
        XWindowAttributes attrs;
        if( XGetWindowAttributes( qt_xdisplay(), winId(), &attrs )
            && attrs.map_state != IsUnmapped )
            break;
    }
    setActiveWindow();
    setFocus();
}
