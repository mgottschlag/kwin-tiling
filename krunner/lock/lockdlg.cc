//===========================================================================
//
// This file is part of the KDE project
//
// Copyright (c) 1999 Martin R. Jones <mjones@kde.org>
// Copyright (c) 2003 Chris Howells <howells@kde.org>

#include <config.h>

#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>
#include <pwd.h>
#include <sys/types.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qmessagebox.h>
#include <qsimplerichtext.h>
#include <qlabel.h>
#include <klocale.h>
#include <kpushbutton.h>
#include <kstandarddirs.h>
#include <kglobalsettings.h>
#include <kconfig.h>
#include <kiconloader.h>
#include <kdesu/defaults.h>
#include <kpassdlg.h>
#include <kdebug.h>
#include "lockdlg.h"
#include "lockprocess.h"
#include "lockdlg.moc"
#include "lockdlgimpl.h"

#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <fixx11h.h>

#define PASSDLG_HIDE_TIMEOUT 10000

//===========================================================================
//
// Simple dialog for entering a password.
//
PasswordDlg::PasswordDlg(LockProcess *parent, bool nsess)
    : LockDlgImpl(parent, "password dialog", true, WX11BypassWM),
      mCapsLocked(-1),
      mUnlockingFailed(false)
{
    frame->setFrameStyle(QFrame::Panel | QFrame::Raised);
    frame->setLineWidth(2);

    pixlabel->setPixmap(DesktopIcon("lock"));

    mLabel->setText(labelText());

    mEntry->installEventFilter(this);

    connect(cancel, SIGNAL(clicked()), SLOT(slotCancel()));
    connect(ok, SIGNAL(clicked()), SLOT(slotOK()));

    if (nsess) {
        connect(mButton, SIGNAL(clicked()), SLOT(slotStartNewSession()));
        mButton->installEventFilter(this);
     } else
     mButton->hide();

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
QString PasswordDlg::labelText()
{
    struct passwd *current = getpwuid(getuid());
    if ( !current )
        return QString::null;

    QString fullname = QString::fromLocal8Bit(current->pw_gecos);
    if (fullname.find(',') != -1)
    {
        // Remove everything from and including first comma
        fullname.truncate(fullname.find(','));
    }

    QString username = QString::fromLocal8Bit(current->pw_name);

    return i18n("Enter the password for user %1 (%2)").arg(fullname).arg(username);
}


void PasswordDlg::updateLabel()
{
    if (mUnlockingFailed)
    {
        mStatusLabel->setPaletteForegroundColor(Qt::black);
        mStatusLabel->setText( i18n("<b>Unlocking failed</b>"));
    }
    else
    if (mCapsLocked)
    {
        mStatusLabel->setPaletteForegroundColor(Qt::red);
        mStatusLabel->setText(i18n("<b>Warning: Caps Lock on</b>"));
    }
    else
    {
        mStatusLabel->clear();
    }
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
	mLabel->setEnabled(true);
        mLabel->setText(labelText());
        mUnlockingFailed = false;
	updateLabel();
        mEntry->erase();
        mEntry->setEnabled(true);
        ok->setEnabled(true);
        cancel->setEnabled(true);
        password->setEnabled(true);
        mEntry->setFocus();
        if(mButton)
            mButton->setEnabled(true);
    }
}

bool PasswordDlg::eventFilter(QObject *, QEvent *ev)
{
    if ( ev->type() == QEvent::KeyPress ) {
        killTimer(mTimeoutTimerId);
        mTimeoutTimerId = startTimer(PASSDLG_HIDE_TIMEOUT);
        capsLocked();
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
    }
    else if (ev->type() == QEvent::KeyRelease)
        capsLocked();
    else if (ev->type() == QEvent::ContextMenu)
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
    if (passwd)
    {
	mLabel->setEnabled(false);
        mButton->setEnabled(false);
        mEntry->setEnabled(false);
        ok->setEnabled(false);
        cancel->setEnabled(false);
        password->setEnabled(false);

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
	    accept();
        }
        else if ( mPassProc.exitStatus() == 2 )
	{
/*
XXX this needs to go into a separate routine at startup time
		KMessageBox::error(0,
		  i18n( "<h1>Screen Locking Failed!</h1>"
		  "Your screen was not locked because the <i>kcheckpass</i> "
		  "program was not able to check your password. This is "
		  "usually the result of kcheckpass not being installed "
		  "correctly. If you installed KDE yourself, reinstall "
		  "kcheckpass as root. If you are using a pre-compiled "
		  "package, contact the packager." ),
		  i18n( "Screen Locking Failed" ) );
*/
            kdDebug(1204) << "kcheckpass cannot verify password. not setuid root?" << endl;
            mLabel->setText(i18n("Verification failed\nKill kdesktop_lock"));
            mFailedTimerId = startTimer(10000);
	}
        else
        {
            mUnlockingFailed = true;
            updateLabel();
            mFailedTimerId = startTimer(1500);
        }
    }
}

void PasswordDlg::show()
{
    QDialog::show();
    QApplication::flushX();
}

void PasswordDlg::slotStartNewSession()
{
    killTimer(mTimeoutTimerId);

    QDialog *dialog = new QDialog( this, "warnbox", true, WX11BypassWM );
    QFrame *winFrame = new QFrame( dialog );
    winFrame->setFrameStyle( QFrame::WinPanel | QFrame::Raised );
    winFrame->setLineWidth( 2 );
    QVBoxLayout *vbox = new QVBoxLayout( dialog );
    vbox->addWidget( winFrame );

    QLabel *label1 = new QLabel( winFrame );
    label1->setPixmap( QMessageBox::standardIcon( QMessageBox::Warning ) );
    QString qt_text =
          i18n("You have chosen to open another desktop session "
               "instead of resuming the current one.<br>"
               "The current session will be hidden "
               "and a new login screen will be displayed.<br>"
               "An F-key is assigned to each session; "
               "F%1 is usually assigned to the first session, "
               "F%2 to the second session and so on. "
               "You can switch between sessions by pressing "
               "CTRL, ALT and the appropriate F-key at the same time.")
            .arg(7).arg(8);
    QLabel *label2 = new QLabel( qt_text, winFrame );
    QPushButton *okbutton = new QPushButton( KStdGuiItem::cont().text(), winFrame );
    okbutton->setDefault( true );
    connect( okbutton, SIGNAL( clicked() ), dialog, SLOT( accept() ) );
    QPushButton *cbutton = new QPushButton( KStdGuiItem::cancel().text(), winFrame );
    connect( cbutton, SIGNAL( clicked() ), dialog, SLOT( reject() ) );

    QBoxLayout *hbox = new QHBoxLayout( 0 );
    hbox->addStretch( 1 );
    hbox->addWidget( okbutton );
    hbox->addStretch( 1 );
    hbox->addWidget( cbutton );
    hbox->addStretch( 1 );

    QGridLayout *grid = new QGridLayout( winFrame, 2, 2, 10 );
    grid->addWidget( label1, 0, 0, Qt::AlignCenter );
    grid->addWidget( label2, 0, 1, Qt::AlignCenter );
    grid->addMultiCellLayout( hbox, 1,1, 0,1 );

    // stolen from kmessagebox
    int pref_width = 0;
    int pref_height = 0;
    // Calculate a proper size for the text.
    {
       QSimpleRichText rt(qt_text, dialog->font());
       QRect rect = KGlobalSettings::desktopGeometry(dialog);

       pref_width = rect.width() / 3;
       rt.setWidth(pref_width);
       int used_width = rt.widthUsed();
       pref_height = rt.height();
       if (used_width <= pref_width)
       {
          while(true)
          {
             int new_width = (used_width * 9) / 10;
             rt.setWidth(new_width);
             int new_height = rt.height();
             if (new_height > pref_height)
                break;
             used_width = rt.widthUsed();
             if (used_width > new_width)
                break;
          }
          pref_width = used_width;
       }
       else
       {
          if (used_width > (pref_width *2))
             pref_width = pref_width *2;
          else
             pref_width = used_width;
       }
    }
    label2->setFixedSize(QSize(pref_width+10, pref_height));

    dialog->show();
    dialog->setFocus();

    static_cast< LockProcess* >( parent())->registerDialog( dialog );
    int ret = dialog->exec();
    static_cast< LockProcess* >( parent())->unregisterDialog( dialog );

    delete dialog;

    mEntry->setFocus();

    if (ret == QDialog::Accepted)
	emit startNewSession();

    mTimeoutTimerId = startTimer(PASSDLG_HIDE_TIMEOUT);
}

void PasswordDlg::slotCancel()
{
    reject();
}

void PasswordDlg::slotOK()
{
   startCheckPassword();
}

void PasswordDlg::capsLocked()
{
    unsigned int lmask;
    Window dummy1, dummy2;
    int dummy3, dummy4, dummy5, dummy6;
    XQueryPointer(qt_xdisplay(), DefaultRootWindow( qt_xdisplay() ), &dummy1, &dummy2, &dummy3, &dummy4, &dummy5, &dummy6, &lmask);
    mCapsLocked = lmask & LockMask;
    updateLabel();
}

