//===========================================================================
//
// This file is part of the KDE project
//
// Copyright 1999 Martin R. Jones <mjones@kde.org>
// Copyright 2003 Chris Howells <howells@kde.org>
// Copyright 2003 Oswald Buddenhagen <ossi@kde.org>

#include <config-unix.h> // HAVE_PAM

#include "lockprocess.h"
#include "lockdlg.h"

#include <kcheckpass-enums.h>
#include <kworkspace/kdisplaymanager.h>

#include <KApplication>
#include <KLocale>
#include <KPushButton>
#include <KSeparator>
#include <KStandardDirs>
#include <KGlobalSettings>
#include <KConfig>
#include <KIconLoader>
#include <KNotification>
#include <kdesu/defaults.h>
#include <KPasswordDialog>
#include <KDebug>
#include <KUser>
#include <KMessageBox>
#include <KColorScheme>

#include <QtDBus/QtDBus>

#include <QLayout>
#include <QPushButton>
// #include <QMessageBox>
#include <QLabel>
#include <QFontMetrics>
#include <QStyle>
#include <QApplication>
#include <QTreeWidget>
#include <QHeaderView>
#include <QCheckBox>
#include <QGridLayout>
#include <QEvent>
#include <QFrame>
#include <QHBoxLayout>
#include <QBoxLayout>
#include <QSocketNotifier>
#include <QTimerEvent>
#include <QVBoxLayout>
#include <QFile>

#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <pwd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>

#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <fixx11h.h>
#include <QX11Info>
#include <kauthorized.h>

#include <KPluginLoader>
#include <KPluginFactory>

#ifndef AF_LOCAL
# define AF_LOCAL	AF_UNIX
#endif

#define PASSDLG_HIDE_TIMEOUT 10000

//===========================================================================
//
// Simple dialog for entering a password.
//
PasswordDlg::PasswordDlg(LockProcess *parent, GreeterPluginHandle *plugin, const QString &text)
    : KDialog(parent, Qt::X11BypassWindowManagerHint),
      mPlugin( plugin ),
      mCapsLocked(-1),
      mUnlockingFailed(false),
      sNot(0)
{
    QWidget* w = mainWidget();

    QLabel *pixLabel = new QLabel( w );
    pixLabel->setPixmap(DesktopIcon(QLatin1String( "system-lock-screen" )));

    KUser user; QString fullName=user.property(KUser::FullName).toString();
    QString greetString = text;
    if (text.isEmpty()) {
        greetString = fullName.isEmpty() ?
            i18n("<nobr><b>The session is locked</b></nobr><br />") :
            i18n("<nobr><b>The session was locked by %1</b></nobr><br />", fullName );
    }
    QLabel *greetLabel = new QLabel(greetString, w);

    mStatusLabel = new QLabel( QLatin1String( "<b> </b>" ), w );
    mStatusLabel->setAlignment( Qt::AlignCenter );

    greet = plugin->info->create(this, w, QString(),
                                 KGreeterPlugin::Authenticate,
                                 KGreeterPlugin::ExUnlock);

    KSeparator *sep = new KSeparator( Qt::Horizontal, w );

    ok = new KPushButton( KGuiItem(i18n("Unl&ock"), QLatin1String( "object-unlocked" )), w );
    cancel = new KPushButton( KStandardGuiItem::cancel(), w );
    mNewSessButton = new KPushButton( KGuiItem(i18n("Sw&itch User..."), QLatin1String( "fork" )), w );

    // Using keyboard layout component
    KPluginFactory *kxkbFactory = KPluginLoader(QLatin1String( "keyboard_layout_widget" )).factory();
    QWidget *kxkbComponent = NULL;
    if (kxkbFactory) {
        kxkbComponent = kxkbFactory->create<QWidget>(this);
    }
    else {
        kDebug() << "can't load keyboard layout widget library";
    }

    QHBoxLayout *layStatus = new QHBoxLayout();
    layStatus->addStretch();
    layStatus->addWidget( mStatusLabel );
    layStatus->addStretch();

    if( kxkbComponent ) {
	//TODO: without this the widget is off the parent area, but we need something better here
        kxkbComponent->setFixedSize(48, 24);
        layStatus->addWidget( kxkbComponent, 0, Qt::AlignRight );
    }

    QHBoxLayout *layButtons = new QHBoxLayout();
    layButtons->addWidget( mNewSessButton );
    layButtons->addStretch();
    layButtons->addWidget( ok );
    layButtons->addWidget( cancel );

    frameLayout = new QGridLayout( w );
    frameLayout->setSpacing( KDialog::spacingHint() );
    frameLayout->setMargin( KDialog::marginHint() );
    frameLayout->addWidget( pixLabel, 0, 0, 3, 1, Qt::AlignTop );
    frameLayout->addWidget( greetLabel, 0, 1 );
    frameLayout->addWidget( greet->getWidgets().first(), 1, 1 );
    frameLayout->addLayout( layStatus, 2, 1 );
    frameLayout->addWidget( sep, 3, 0, 1, 2 );
    frameLayout->addLayout( layButtons, 4, 0, 1, 2 );

    setButtons(None);
    connect(cancel, SIGNAL(clicked()), SLOT(reject()));
    connect(ok, SIGNAL(clicked()), SLOT(slotOK()));
    connect(mNewSessButton, SIGNAL(clicked()), SLOT(slotSwitchUser()));

    if (!text.isEmpty() || !KDisplayManager().isSwitchable() || !KAuthorized::authorizeKAction(QLatin1String( "switch_user" )))
        mNewSessButton->hide();

    installEventFilter(this);

    mFailedTimerId = 0;
    mTimeoutTimerId = startTimer(PASSDLG_HIDE_TIMEOUT);
    connect(qApp, SIGNAL(activity()), SLOT(slotActivity()) );

    greet->start();

    capsLocked();
}

PasswordDlg::~PasswordDlg()
{
    hide();
    delete greet;
}

void PasswordDlg::updateLabel()
{
    if (mUnlockingFailed)
    {
        QPalette palette;
        KColorScheme::adjustForeground(palette, KColorScheme::NormalText, QPalette::WindowText);
        mStatusLabel->setPalette(palette);
        mStatusLabel->setText(i18n("<b>Unlocking failed</b>"));
    }
    else
    if (mCapsLocked)
    {
        QPalette palette = mStatusLabel->palette();
        KColorScheme::adjustForeground(palette, KColorScheme::NegativeText, QPalette::WindowText);
        mStatusLabel->setPalette(palette);
        mStatusLabel->setText(i18n("<b>Warning: Caps Lock on</b>"));
    }
    else
    {
        mStatusLabel->setText(QLatin1String( "<b> </b>" ));
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
        done(TIMEOUT_CODE);
    }
    else if (ev->timerId() == mFailedTimerId)
    {
        killTimer(mFailedTimerId);
        mFailedTimerId = 0;
        // Show the normal password prompt.
        mUnlockingFailed = false;
        updateLabel();
        ok->setEnabled(true);
        cancel->setEnabled(true);
        mNewSessButton->setEnabled( true );
        greet->revive();
        greet->start();
    }
}

bool PasswordDlg::eventFilter(QObject *, QEvent *ev)
{
    if (ev->type() == QEvent::KeyPress || ev->type() == QEvent::KeyRelease)
        capsLocked();
    return false;
}

void PasswordDlg::slotActivity()
{
    if (mTimeoutTimerId) {
        killTimer(mTimeoutTimerId);
        mTimeoutTimerId = startTimer(PASSDLG_HIDE_TIMEOUT);
    }
}

////// kckeckpass interface code

int PasswordDlg::Reader (void *buf, int count)
{
    int ret, rlen;

    for (rlen = 0; rlen < count; ) {
      dord:
        ret = ::read (sFd, (void *)((char *)buf + rlen), count - rlen);
        if (ret < 0) {
            if (errno == EINTR)
                goto dord;
            if (errno == EAGAIN)
                break;
            return -1;
        }
        if (!ret)
            break;
        rlen += ret;
    }
    return rlen;
}

bool PasswordDlg::GRead (void *buf, int count)
{
    return Reader (buf, count) == count;
}

bool PasswordDlg::GWrite (const void *buf, int count)
{
    return ::write (sFd, buf, count) == count;
}

bool PasswordDlg::GSendInt (int val)
{
    return GWrite (&val, sizeof(val));
}

bool PasswordDlg::GSendStr (const char *buf)
{
    int len = buf ? ::strlen (buf) + 1 : 0;
    return GWrite (&len, sizeof(len)) && GWrite (buf, len);
}

bool PasswordDlg::GSendArr (int len, const char *buf)
{
    return GWrite (&len, sizeof(len)) && GWrite (buf, len);
}

bool PasswordDlg::GRecvInt (int *val)
{
    return GRead (val, sizeof(*val));
}

bool PasswordDlg::GRecvArr (char **ret)
{
    int len;
    char *buf;

    if (!GRecvInt(&len))
        return false;
    if (!len) {
        *ret = 0;
        return true;
    }
    if (!(buf = (char *)::malloc (len)))
        return false;
    *ret = buf;
    if (GRead (buf, len)) {
        return true;
    } else {
        ::free(buf);
        *ret = 0;
        return false;
    }
}

void PasswordDlg::reapVerify()
{
    sNot->setEnabled( false );
    sNot->deleteLater();
    sNot = 0;
    ::close( sFd );
    int status;
    while (::waitpid( sPid, &status, 0 ) < 0)
        if (errno != EINTR) { // This should not happen ...
            cantCheck();
            return;
        }
    if (WIFEXITED(status))
        switch (WEXITSTATUS(status)) {
        case AuthOk:
            greet->succeeded();
            accept();
            return;
        case AuthBad:
            greet->failed();
            mUnlockingFailed = true;
            updateLabel();
            mFailedTimerId = startTimer(1500);
            ok->setEnabled(false);
            cancel->setEnabled(false);
            mNewSessButton->setEnabled( false );
            KNotification::event( QLatin1String( "unlockfailed" ) );
            return;
        case AuthAbort:
            return;
        }
    cantCheck();
}

void PasswordDlg::handleVerify()
{
    int ret;
    char *arr;

    if (GRecvInt( &ret )) {
        switch (ret) {
        case ConvGetBinary:
            if (!GRecvArr( &arr ))
                break;
            greet->binaryPrompt( arr, false );
            if (arr)
                ::free( arr );
            return;
        case ConvGetNormal:
            if (!GRecvArr( &arr ))
                break;
            greet->textPrompt( arr, true, false );
            if (arr)
                ::free( arr );
            return;
        case ConvGetHidden:
            if (!GRecvArr( &arr ))
                break;
            greet->textPrompt( arr, false, false );
            if (arr)
                ::free( arr );
            return;
        case ConvPutInfo:
            if (!GRecvArr( &arr ))
                break;
            if (!greet->textMessage( arr, false ))
                static_cast< LockProcess* >(parent())->msgBox( this, QMessageBox::Information, QString::fromLocal8Bit( arr ) );
            ::free( arr );
            return;
        case ConvPutError:
            if (!GRecvArr( &arr ))
                break;
            if (!greet->textMessage( arr, true ))
                static_cast< LockProcess* >(parent())->msgBox( this, QMessageBox::Warning, QString::fromLocal8Bit( arr ) );
            ::free( arr );
            return;
        }
    }
    reapVerify();
}

////// greeter plugin callbacks

void PasswordDlg::gplugReturnText( const char *text, int tag )
{
    GSendStr( text );
    if (text)
        GSendInt( tag );
}

void PasswordDlg::gplugReturnBinary( const char *data )
{
    if (data) {
        unsigned const char *up = (unsigned const char *)data;
        int len = up[3] | (up[2] << 8) | (up[1] << 16) | (up[0] << 24);
        if (!len)
            GSendArr( 4, data );
        else
            GSendArr( len, data );
    } else
        GSendArr( 0, 0 );
}

void PasswordDlg::gplugSetUser( const QString & )
{
    // ignore ...
}

void PasswordDlg::cantCheck()
{
    greet->failed();
    static_cast< LockProcess* >(parent())->msgBox( this, QMessageBox::Critical,
        i18n("Cannot unlock the session because the authentication system failed to work;\n"
             "you must kill kscreenlocker (pid %1) manually.", getpid()) );
    greet->revive();
}

//---------------------------------------------------------------------------
//
// Starts the kcheckpass process to check the user's password.
//
void PasswordDlg::gplugStart()
{
    int sfd[2];
    char fdbuf[16];

    if (sNot)
        return;
    if (::socketpair(AF_LOCAL, SOCK_STREAM, 0, sfd)) {
        cantCheck();
        return;
    }
    if ((sPid = ::fork()) < 0) {
        ::close(sfd[0]);
        ::close(sfd[1]);
        cantCheck();
        return;
    }
    if (!sPid) {
        ::close(sfd[0]);
        sprintf(fdbuf, "%d", sfd[1]);
        execlp(QFile::encodeName(KStandardDirs::findExe(QLatin1String( "kcheckpass" ))).data(),
               "kcheckpass",
               "-m", mPlugin->info->method,
               "-S", fdbuf,
               (char *)0);
        _exit(20);
    }
    ::close(sfd[1]);
    sFd = sfd[0];
    sNot = new QSocketNotifier(sFd, QSocketNotifier::Read, this);
    connect(sNot, SIGNAL(activated(int)), SLOT(handleVerify()));
}

void PasswordDlg::gplugChanged()
{
}

void PasswordDlg::gplugActivity()
{
    slotActivity();
}

void PasswordDlg::gplugMsgBox( QMessageBox::Icon type, const QString &text )
{
    static_cast< LockProcess* >(parent())->msgBox( this, type, text );
}

bool PasswordDlg::gplugHasNode( const QString & )
{
    return false;
}

void PasswordDlg::slotOK()
{
    greet->next();
}


void PasswordDlg::setVisible( bool visible )
{
    QDialog::setVisible( visible );

    if ( visible )
        QApplication::flush();
}

void PasswordDlg::slotStartNewSession()
{
    if (!KMessageBox::shouldBeShownContinue( QLatin1String( ":confirmNewSession" ) )) {
        KDisplayManager().startReserve();
        return;
    }

    killTimer(mTimeoutTimerId);
    mTimeoutTimerId = 0;

    KDialog *dialog = new KDialog( this, Qt::X11BypassWindowManagerHint );
    dialog->setModal( true );
    dialog->setButtons( KDialog::Yes | KDialog::No );
    dialog->setButtonGuiItem( KDialog::Yes, KGuiItem(i18n("&Start New Session"), QLatin1String( "fork" )) );
    dialog->setButtonGuiItem( KDialog::No, KStandardGuiItem::cancel() );
    dialog->setDefaultButton( KDialog::Yes );
    dialog->setEscapeButton( KDialog::No );

    bool dontAskAgain = false;

    KMessageBox::createKMessageBox( dialog, QMessageBox::Warning,
          i18n("You have chosen to open another desktop session "
               "instead of resuming the current one.\n"
               "The current session will be hidden "
               "and a new login screen will be displayed.\n"
               "An F-key is assigned to each session; "
               "F%1 is usually assigned to the first session, "
               "F%2 to the second session and so on. "
               "You can switch between sessions by pressing "
               "Ctrl, Alt and the appropriate F-key at the same time. "
               "Additionally, the KDE Panel and Desktop menus have "
               "actions for switching between sessions.",
             7, 8),
        QStringList(),
        i18n("&Do not ask again"), &dontAskAgain,
        KMessageBox::NoExec );

    int ret = static_cast< LockProcess* >( parent())->execDialog( dialog );

    delete dialog;

    if (ret == KDialog::Yes) {
        if (dontAskAgain)
            KMessageBox::saveDontShowAgainContinue( QLatin1String( ":confirmNewSession" ) );
        KDisplayManager().startReserve();
    }

    mTimeoutTimerId = startTimer(PASSDLG_HIDE_TIMEOUT);
}

class LockListViewItem : public QTreeWidgetItem {
public:
    LockListViewItem( QTreeWidget *parent,
                      const QString &sess, const QString &loc, int _vt )
        : QTreeWidgetItem( parent )
        , vt( _vt )
    {
        setText( 0, sess );
        setText( 1, loc );
    }

    int vt;
};

void PasswordDlg::slotSwitchUser()
{
    int p = 0;
    KDisplayManager dm;

    QDialog dialog( this, Qt::X11BypassWindowManagerHint );
    dialog.setModal( true );

    QBoxLayout *hbox = new QHBoxLayout( &dialog );
    hbox->setSpacing( KDialog::spacingHint() );
    hbox->setMargin( KDialog::marginHint() );

    QBoxLayout *vbox1 = new QVBoxLayout( );
    hbox->addItem( vbox1 );
    QBoxLayout *vbox2 = new QVBoxLayout( );
    hbox->addItem( vbox2 );

    KPushButton *btn;

    SessList sess;
    if (dm.localSessions( sess )) {

        lv = new QTreeWidget( &dialog );
        connect( lv, SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)), SLOT(slotSessionActivated()) );
        connect( lv, SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)), &dialog, SLOT(reject()) );
        lv->setAllColumnsShowFocus( true );
        lv->setHeaderLabels( QStringList() << i18n("Session") << i18n("Location") );
        lv->header()->setResizeMode( 0, QHeaderView::Stretch );
        lv->header()->setResizeMode( 1, QHeaderView::Stretch );
        QTreeWidgetItem *itm = 0;
        QString user, loc;
        int ns = 0;
        for (SessList::ConstIterator it = sess.constBegin(); it != sess.constEnd(); ++it) {
            KDisplayManager::sess2Str2( *it, user, loc );
            itm = new LockListViewItem( lv, user, loc, (*it).vt );
            if (!(*it).vt)
                itm->setFlags( itm->flags() & ~Qt::ItemIsEnabled );
            if ((*it).self) {
                lv->setCurrentItem( itm );
                itm->setSelected( true );
            }
            ns++;
        }
        int fw = lv->frameWidth() * 2;
        QSize hds( lv->header()->sizeHint() );
        lv->setMinimumWidth( fw + hds.width() +
            (ns > 10 ? style()->pixelMetric(QStyle::PM_ScrollBarExtent) : 0 ) );
        int ih = lv->itemDelegate()->sizeHint(
            QStyleOptionViewItem(), lv->model()->index( 0, 0 ) ).height();
        lv->setFixedHeight( fw + hds.height() +
            ih * (ns < 6 ? 6 : ns > 10 ? 10 : ns) );
        lv->header()->adjustSize();
        vbox1->addWidget( lv );

        btn = new KPushButton( KGuiItem(i18nc("session", "&Activate"), QLatin1String( "fork" )), &dialog );
        connect( btn, SIGNAL(clicked()), SLOT(slotSessionActivated()) );
        connect( btn, SIGNAL(clicked()), &dialog, SLOT(reject()) );
        vbox2->addWidget( btn );
        vbox2->addStretch( 2 );
    }

    if (KAuthorized::authorizeKAction(QLatin1String( "start_new_session" )) && (p = dm.numReserve()) >= 0)
    {
        btn = new KPushButton( KGuiItem(i18n("Start &New Session"), QLatin1String( "fork" )), &dialog );
        connect( btn, SIGNAL(clicked()), SLOT(slotStartNewSession()) );
        connect( btn, SIGNAL(clicked()), &dialog, SLOT(reject()) );
        if (!p)
            btn->setEnabled( false );
        vbox2->addWidget( btn );
        vbox2->addStretch( 1 );
    }

    btn = new KPushButton( KStandardGuiItem::cancel(), &dialog );
    connect( btn, SIGNAL(clicked()), &dialog, SLOT(reject()) );
    vbox2->addWidget( btn );

    static_cast< LockProcess* >(parent())->execDialog( &dialog );
}

void PasswordDlg::slotSessionActivated()
{
    LockListViewItem *itm = (LockListViewItem *)lv->currentItem();
    if (itm && itm->vt > 0)
        KDisplayManager().switchVT( itm->vt );
}

void PasswordDlg::capsLocked()
{
    unsigned int lmask;
    Window dummy1, dummy2;
    int dummy3, dummy4, dummy5, dummy6;
    XQueryPointer(QX11Info::display(), DefaultRootWindow( QX11Info::display() ), &dummy1, &dummy2, &dummy3, &dummy4, &dummy5, &dummy6, &lmask);
    mCapsLocked = lmask & LockMask;
    updateLabel();
}

#include "lockdlg.moc"
