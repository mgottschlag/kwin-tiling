//-----------------------------------------------------------------------------
//
// KDE Display screen saver setup module
//
// Copyright (c)  Martin R. Jones 1996,1999,2002
//
// Converted to a kcc module by Matthias Hoelzer 1997
//


#include <config-workspace.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/wait.h>

#include <kservicetypetrader.h>
#include <kstandarddirs.h>
#include <QCheckBox>
#include <Qt3Support/Q3Header>
#include <QLabel>
#include <Qt3Support/Q3CheckListItem>
#include <QPushButton>
#include <QTimer>
#include <kmacroexpander.h>
#include <kshell.h>

//Added by qt3to4:
#include <QTextStream>
#include <QKeyEvent>
#include <QHBoxLayout>
#include <QBoxLayout>
#include <QVBoxLayout>
#include <QResizeEvent>
#include <QMouseEvent>


#include <QtDBus/QtDBus>
#include <kaboutdata.h>
#include <kapplication.h>
#include <kdebug.h>
#include <kdialog.h>
#include <kiconloader.h>
#include <knuminput.h>
#include <k3process.h>
#include <kservicegroup.h>

#include <X11/Xlib.h>
#include <fixx11h.h>

#include "scrnsave.h"
#include <QX11Info>
#include <QDesktopWidget>
#include <kscreensaver_interface.h>
#include <KPluginFactory>
#include <KPluginLoader>

#include <kworkspace/screenpreviewwidget.h>

template class QList<SaverConfig*>;

const uint widgetEventMask =                 // X event mask
(uint)(
       ExposureMask |
       PropertyChangeMask |
       StructureNotifyMask
      );

//===========================================================================
// DLL Interface for kcontrol
K_PLUGIN_FACTORY(KSSFactory,
        registerPlugin<KScreenSaver>(); // K_EXPORT_COMPONENT_FACTORY (screensaver
)
K_EXPORT_PLUGIN(KSSFactory("kcmscreensaver"))


static QString findExe(const QString &exe) {
    QString result = KStandardDirs::locate("exe", exe);
    if (result.isEmpty())
        result = KStandardDirs::findExe(exe);
    return result;
}

KScreenSaver::KScreenSaver(QWidget *parent, const QVariantList&)
    : KCModule(KSSFactory::componentData(), parent)
{
    mSetupProc = 0;
    mPreviewProc = 0;
    mTestWin = 0;
    mTestProc = 0;
    mPrevSelected = -2;
    mMonitor = 0;
    mTesting = false;

    setQuickHelp( i18n("<h1>Screen Saver</h1> <p>This module allows you to enable and"
       " configure a screen saver. Note that you can enable a screen saver"
       " even if you have power saving features enabled for your display.</p>"
       " <p>Besides providing an endless variety of entertainment and"
       " preventing monitor burn-in, a screen saver also gives you a simple"
       " way to lock your display if you are going to leave it unattended"
       " for a while. If you want the screen saver to lock the session, make sure you enable"
       " the \"Require password\" feature of the screen saver; if you do not, you can still"
       " explicitly lock the session using the desktop's \"Lock Session\" action.</p>"));

    setButtons( KCModule::Help |  KCModule::Apply );


    setupUi(this);
    
    readSettings();

    mSetupProc = new K3Process;
    connect(mSetupProc, SIGNAL(processExited(K3Process *)),
            this, SLOT(slotSetupDone(K3Process *)));

    mPreviewProc = new K3Process;
    connect(mPreviewProc, SIGNAL(processExited(K3Process *)),
            this, SLOT(slotPreviewExited(K3Process *)));

    mSaverListView->addColumn("");
    mSaverListView->header()->hide();
    mSelected = -1;
    connect( mSaverListView, SIGNAL(doubleClicked ( Q3ListViewItem *)), this, SLOT( slotSetup()));
    
    connect( mSetupBt, SIGNAL( clicked() ), SLOT( slotSetup() ) );
    connect( mTestBt, SIGNAL( clicked() ), SLOT( slotTest() ) );

    mEnabledCheckBox->setChecked(mEnabled);
    connect(mEnabledCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(slotEnable(bool)));


    mActivateLbl->setEnabled(mEnabled);

    mWaitEdit->setRange(1, INT_MAX);
    mWaitEdit->setSuffix(ki18np(" minute", " minutes"));
    mWaitEdit->setValue(mTimeout/60);
    mWaitEdit->setEnabled(mEnabled);
    connect(mWaitEdit, SIGNAL(valueChanged(int)),
            this, SLOT(slotTimeoutChanged(int)));

    mLockCheckBox->setEnabled( mEnabled );
    mLockCheckBox->setChecked( mLock );
    connect( mLockCheckBox, SIGNAL( toggled( bool ) ),
             this, SLOT( slotLock( bool ) ) );


    mLockLbl->setEnabled(mEnabled && mLock);

    mWaitLockEdit->setRange(1, 300);
    mWaitLockEdit->setSuffix(ki18np(" second", " seconds"));
    mWaitLockEdit->setValue(mLockTimeout/1000);
    mWaitLockEdit->setEnabled(mEnabled && mLock);
    connect(mWaitLockEdit, SIGNAL(valueChanged(int)),
            this, SLOT(slotLockTimeoutChanged(int)));

    mPlasmaCheckBox->setChecked(mPlasmaEnabled);
    connect(mPlasmaCheckBox, SIGNAL(toggled(bool)), this, SLOT(slotEnablePlasma(bool)));

    mPlasmaSetup->setEnabled(mPlasmaEnabled);
    connect(mPlasmaSetup, SIGNAL(clicked()), this, SLOT(slotPlasmaSetup()));

    mMonitorPreview = new ScreenPreviewWidget(this);
    mMonitorPreview->setFixedSize(200,220);
    QDesktopWidget *desktop = QApplication::desktop();
    QRect avail = desktop->availableGeometry(desktop->screenNumber(this));
    mMonitorPreview->setRatio((qreal)avail.width()/(qreal)avail.height());
    mMonitorPreview->setWhatsThis( i18n("A preview of the selected screen saver.") );
    mPreviewAreaWidget->layout()->addWidget(mMonitorPreview);
    
    connect( advancedBt, SIGNAL( clicked() ),
             this, SLOT( slotAdvanced() ) );

    if (mImmutable)
    {
       setButtons(buttons() & ~Default);
       mSettingsGroup->setEnabled(false);
    }

    // finding the savers can take some time, so defer loading until
    // we've started up.
    mNumLoaded = 0;
    mLoadTimer = new QTimer( this );
    connect( mLoadTimer, SIGNAL(timeout()), SLOT(findSavers()) );
    mLoadTimer->start( 100 );
    mChanged = false;
    emit changed(false);

    KAboutData *about =
    new KAboutData(I18N_NOOP("kcmscreensaver"), 0, ki18n("KDE Screen Saver Control Module"),
                  0, KLocalizedString(), KAboutData::License_GPL,
                  ki18n("(c) 1997-2002 Martin R. Jones\n"
                  "(c) 2003-2004 Chris Howells"));
    about->addAuthor(ki18n("Chris Howells"), KLocalizedString(), "howells@kde.org");
    about->addAuthor(ki18n("Martin R. Jones"), KLocalizedString(), "jones@kde.org");

    setAboutData( about );

}

//---------------------------------------------------------------------------
//
void KScreenSaver::resizeEvent( QResizeEvent * )
{

  if (mMonitor)
    {
      mMonitor->setGeometry( mMonitorPreview->previewRect() );
    }
}

//---------------------------------------------------------------------------
//
void KScreenSaver::mousePressEvent( QMouseEvent *)
{
    if ( mTesting )
	slotStopTest();
}

//---------------------------------------------------------------------------
//
void KScreenSaver::keyPressEvent( QKeyEvent *)
{
    if ( mTesting )
	slotStopTest();
}
//---------------------------------------------------------------------------
//
KScreenSaver::~KScreenSaver()
{
    if (mPreviewProc)
    {
        if (mPreviewProc->isRunning())
        {
            //Avoid triggering slotPreviewExited on close
            disconnect(mPreviewProc, SIGNAL(processExited(K3Process *)),
              this, SLOT(slotPreviewExited(K3Process *)));

            mPreviewProc->kill( );
            mPreviewProc->wait( );
        }
        delete mPreviewProc;
    }

    delete mTestProc;
    delete mSetupProc;
    delete mTestWin;

    qDeleteAll(mSaverList);
}

//---------------------------------------------------------------------------
//
void KScreenSaver::load()
{
    readSettings();

//with the following line, the Test and Setup buttons are not enabled correctly
//if no saver was selected, the "Reset" and the "Enable screensaver", it is only called when starting and when pressing reset, aleXXX
//    mSelected = -1;
    int i = 0;
    Q3ListViewItem *selectedItem = 0;
	Q_FOREACH( SaverConfig* saver, mSaverList ){
        if (saver->file() == mSaver)
        {
            selectedItem = mSaverListView->findItem ( saver->name(), 0 );
            if (selectedItem) {
                mSelected = i;
                break;
            }
        }
        i++;
    }
    if ( selectedItem )
    {
      mSaverListView->setSelected( selectedItem, true );
      mSaverListView->setCurrentItem( selectedItem );
      slotScreenSaver( selectedItem );
    }

    updateValues();
    mChanged = false;
    emit changed(false);
}

//------------------------------------------------------------After---------------
//
void KScreenSaver::readSettings()
{
    KConfigGroup config( KSharedConfig::openConfig( "kscreensaverrc"), "ScreenSaver" );

    mImmutable = config.isImmutable();

    mEnabled = config.readEntry("Enabled", false);
    mTimeout = config.readEntry("Timeout", 300);
    mLockTimeout = config.readEntry("LockGrace", 60000);
    mLock = config.readEntry("Lock", false);
    mSaver = config.readEntry("Saver");
    mPlasmaEnabled = config.readEntry("PlasmaEnabled", false);

    if (mTimeout < 60) mTimeout = 60;
    if (mLockTimeout < 0) mLockTimeout = 0;
    if (mLockTimeout > 300000) mLockTimeout = 300000;

    mChanged = false;
}

//---------------------------------------------------------------------------
//
void KScreenSaver::updateValues()
{
    if (mEnabled)
    {
        mWaitEdit->setValue(mTimeout/60);
    }
    else
    {
        mWaitEdit->setValue(0);
    }

    mWaitLockEdit->setValue(mLockTimeout/1000);
    mLockCheckBox->setChecked(mLock);
}

//---------------------------------------------------------------------------
//
void KScreenSaver::defaults()
{
    if (mImmutable) return;

    slotScreenSaver( 0 );

    Q3ListViewItem *item = mSaverListView->firstChild();
    if (item) {
        mSaverListView->setSelected( item, true );
        mSaverListView->setCurrentItem( item );
        mSaverListView->ensureItemVisible( item );
    }
    slotTimeoutChanged( 5 );
    slotLockTimeoutChanged( 60 );
    slotLock( false );
    mEnabledCheckBox->setChecked(false);
    mPlasmaCheckBox->setChecked(false);
    mPlasmaSetup->setEnabled(false);

    updateValues();

    emit changed(true);
}

//---------------------------------------------------------------------------
//
void KScreenSaver::save()
{
    if ( !mChanged )
        return;

    KConfigGroup config(KSharedConfig::openConfig( "kscreensaverrc"), "ScreenSaver" );

    config.writeEntry("Enabled", mEnabled);
    config.writeEntry("Timeout", mTimeout);
    config.writeEntry("LockGrace", mLockTimeout);
    config.writeEntry("Lock", mLock);
    config.writeEntry("PlasmaEnabled", mPlasmaEnabled);

    if ( !mSaver.isEmpty() )
        config.writeEntry("Saver", mSaver);
    config.sync();

    org::kde::screensaver kscreensaver("org.kde.screensaver", "/ScreenSaver", QDBusConnection::sessionBus());
    kscreensaver.configure();

    mChanged = false;
    emit changed(false);
}

//---------------------------------------------------------------------------
//
void KScreenSaver::findSavers()
{
    if ( !mNumLoaded ) {
	mSaverServices = KServiceTypeTrader::self()->query( "ScreenSaver");
        new Q3ListViewItem ( mSaverListView, i18n("Loading...") );
        if ( mSaverServices.isEmpty() )
            mLoadTimer->stop();
        else
            mLoadTimer->start( 50 );
    }
    for( KService::List::const_iterator it = mSaverServices.constBegin();
        it != mSaverServices.constEnd(); it++,mNumLoaded++)
    {
      SaverConfig *saver = new SaverConfig;
      QString file = KStandardDirs::locate("services", (*it)->entryPath());
      if (saver->read(file)) {
	      mSaverList.append(saver);
        } else
            delete saver;
    }

    if ( mNumLoaded == mSaverServices.count() ) {
        Q3ListViewItem *selectedItem = 0;
        int categoryCount = 0;
        int indx = 0;

        mLoadTimer->stop();
        delete mLoadTimer;
		qSort(mSaverList.begin(), mSaverList.end());

        mSelected = -1;
        mSaverListView->clear();
        Q_FOREACH( SaverConfig *s, mSaverList )
        {
            Q3ListViewItem *item;
            if (s->category().isEmpty())
                item = new Q3ListViewItem ( mSaverListView, s->name(), '2' + s->name() );
            else
            {
                Q3ListViewItem *categoryItem = mSaverListView->findItem( s->category(), 0 );
                if ( !categoryItem ) {
                    categoryItem = new Q3ListViewItem ( mSaverListView, s->category(), '1' + s->category() );
                    categoryItem->setPixmap ( 0, SmallIcon ( "preferences-desktop-screensaver" ) );
                }
                item = new Q3ListViewItem ( categoryItem, s->name(), s->name() );
                categoryCount++;
            }
            if (s->file() == mSaver) {
                mSelected = indx;
                selectedItem = item;
            }
            indx++;
        }

        // Delete categories with only one item
        Q3ListViewItemIterator it ( mSaverListView );
        for ( ; it.current(); it++ )
            if ( it.current()->childCount() == 1 ) {
               Q3ListViewItem *item = it.current()->firstChild();
               it.current()->takeItem( item );
               mSaverListView->insertItem ( item );
               delete it.current();
               categoryCount--;
            }

        mSaverListView->setRootIsDecorated ( categoryCount > 0 );
        mSaverListView->setSorting ( 1 );

        if ( mSelected > -1 )
        {
            mSaverListView->setSelected(selectedItem, true);
            mSaverListView->setCurrentItem(selectedItem);
            mSaverListView->ensureItemVisible(selectedItem);
            mSetupBt->setEnabled(!mSaverList.at(mSelected)->setup().isEmpty());
            mTestBt->setEnabled(true);
        }

        connect( mSaverListView, SIGNAL( currentChanged( Q3ListViewItem * ) ),
                 this, SLOT( slotScreenSaver( Q3ListViewItem * ) ) );

        setMonitor();
    }
}

//---------------------------------------------------------------------------
//
void KScreenSaver::setMonitor()
{
    if (mPreviewProc->isRunning())
    // CC: this will automatically cause a "slotPreviewExited"
    // when the viewer exits
    mPreviewProc->kill();
    else
    slotPreviewExited(mPreviewProc);
}

//---------------------------------------------------------------------------
//
void KScreenSaver::slotPreviewExited(K3Process *)
{
    // Ugly hack to prevent continual respawning of savers that crash
    if (mSelected == mPrevSelected)
        return;

    if ( mSaverList.isEmpty() ) // safety check
        return;

    // Some xscreensaver hacks do something nasty to the window that
    // requires a new one to be created (or proper investigation of the
    // problem).
    delete mMonitor;

    mMonitor = new KSSMonitor(mMonitorPreview);
    QPalette palette;
    palette.setColor(mMonitor->backgroundRole(), Qt::black);
    mMonitor->setPalette(palette);
    mMonitor->setGeometry(mMonitorPreview->previewRect());
    mMonitor->show();
    // So that hacks can XSelectInput ButtonPressMask
    XSelectInput(QX11Info::display(), mMonitor->winId(), widgetEventMask );

    if (mSelected >= 0) {
        mPreviewProc->clearArguments();

        QString saver = mSaverList.at(mSelected)->saver();
        QHash<QChar, QString> keyMap;
        keyMap.insert('w', QString::number(mMonitor->winId()));
        *mPreviewProc << KShell::splitArgs(KMacroExpander::expandMacrosShellQuote(saver, keyMap));

        mPreviewProc->start();
    }

    mPrevSelected = mSelected;
}

//---------------------------------------------------------------------------
//
void KScreenSaver::slotEnable(bool e)
{
    mEnabled = e;
    mActivateLbl->setEnabled( e );
    mWaitEdit->setEnabled( e );
    mLockCheckBox->setEnabled( e );
    mLockLbl->setEnabled( e && mLock );
    mWaitLockEdit->setEnabled( e && mLock );
    mChanged = true;
    emit changed(true);
}

void KScreenSaver::slotEnablePlasma(bool enable)
{
    mPlasmaEnabled = enable;
    //FIXME even though the button's enabled, plasma isn't until the user hits apply
    //so the button will just show the screensaver, no plasma.
    //what should I do about this?
    mPlasmaSetup->setEnabled(mPlasmaEnabled);
    mChanged = true;
    emit changed(true);
}

void KScreenSaver::slotPlasmaSetup()
{
    org::kde::screensaver kscreensaver("org.kde.screensaver", "/ScreenSaver", QDBusConnection::sessionBus());
    kscreensaver.setupPlasma();
}


//---------------------------------------------------------------------------
//
void KScreenSaver::slotScreenSaver(Q3ListViewItem *item)
{
    if (!item)
    {
        mSetupBt->setEnabled(false);
        mTestBt->setEnabled(false);
        return;
    }
    int i = 0, indx = -1;
	Q_FOREACH( SaverConfig* saver , mSaverList ){
        if ( item->parent() )
        {
            if (  item->parent()->text( 0 ) == saver->category() && saver->name() == item->text (0))
            {
                indx = i;
                break;
            }
        }
        else
        {
            if (  saver->name() == item->text (0) )
            {
                indx = i;
                break;
            }
        }
		i++;
    }

    mSetupBt->setEnabled(item->childCount()==0);
    mTestBt->setEnabled(item->childCount()==0);
    if (indx == -1) {
        mSelected = -1;
        return;
    }

    bool bChanged = (indx != mSelected);

    if (!mSetupProc->isRunning())
        mSetupBt->setEnabled(!mSaverList.at(indx)->setup().isEmpty());
    mTestBt->setEnabled(true);
    mSaver = mSaverList.at(indx)->file();

    mSelected = indx;
    setMonitor();
    if (bChanged)
    {
       mChanged = true;
       emit changed(true);
    }
}

//---------------------------------------------------------------------------
//
void KScreenSaver::slotSetup()
{
    if ( mSelected < 0 )
    return;

    if (mSetupProc->isRunning())
    return;

    mSetupProc->clearArguments();

    QString saver = mSaverList.at(mSelected)->setup();
    if( saver.isEmpty())
        return;
    QTextStream ts(&saver, QIODevice::ReadOnly);

    QString word;
    ts >> word;
    bool kxsconfig = word == "kxsconfig";
    QString path = findExe(word);

    if (!path.isEmpty())
    {
        (*mSetupProc) << path;

        // Add caption and icon to about dialog
        if (!kxsconfig) {
            word = "-caption";
            (*mSetupProc) << word;
            word = mSaverList.at(mSelected)->name();
            (*mSetupProc) << word;
            word = "-icon";
            (*mSetupProc) << word;
            word = "kscreensaver";
            (*mSetupProc) << word;
        }

        while (!ts.atEnd())
        {
            ts >> word;
            (*mSetupProc) << word;
        }

        // Pass translated name to kxsconfig
        if (kxsconfig) {
          word = mSaverList.at(mSelected)->name();
          (*mSetupProc) << word;
        }

        mSetupBt->setEnabled( false );
        kapp->flush();

        mSetupProc->start();
    }
}

//---------------------------------------------------------------------------
//
void KScreenSaver::slotAdvanced()
{
   KScreenSaverAdvancedDialog dlg( window() );
   if ( dlg.exec() ) {
       mChanged = true;
       emit changed(true);
  }
}

//---------------------------------------------------------------------------
//
void KScreenSaver::slotTest()
{
    if ( mSelected == -1 )
        return;

    if (!mTestProc) {
        mTestProc = new K3Process;
    } else {
        mPreviewProc->kill();
        mPreviewProc->wait();
        mTestProc->clearArguments();
    }

    if (!mTestWin)
    {
        mTestWin = new TestWin();
        mTestWin->setAttribute(Qt::WA_NoSystemBackground, true);
        mTestWin->setAttribute(Qt::WA_PaintOnScreen, true);
        mTestWin->setGeometry(qApp->desktop()->geometry());
    }

    mTestWin->show();
    mTestWin->raise();
    mTestWin->setFocus();
	// So that hacks can XSelectInput ButtonPressMask
	XSelectInput(QX11Info::display(), mTestWin->winId(), widgetEventMask );

	grabMouse();
	grabKeyboard();

    mTestBt->setEnabled( false );

    QString saver = mSaverList.at(mSelected)->saver();
    QHash<QChar, QString> keyMap;
    keyMap.insert('w', QString::number(mTestWin->winId()));
    *mTestProc << KShell::splitArgs(KMacroExpander::expandMacrosShellQuote(saver, keyMap));

    mTestProc->start(K3Process::NotifyOnExit);

    mTesting = true;
}

//---------------------------------------------------------------------------
//
void KScreenSaver::slotStopTest()
{
    if (mTestProc->isRunning()) {
        mTestProc->kill();
        mTestProc->wait();
    }
    releaseMouse();
    releaseKeyboard();
    mTestWin->hide();
    mTestBt->setEnabled(true);
    mPrevSelected = -1;
    setMonitor();
    mTesting = false;
}

//---------------------------------------------------------------------------
//
void KScreenSaver::slotTimeoutChanged(int to )
{
    mTimeout = to * 60;
    mChanged = true;
    emit changed(true);
}

//-----------------------------------------------------------------------
//
void KScreenSaver::slotLockTimeoutChanged(int to )
{
    mLockTimeout = to * 1000;
    mChanged = true;
    emit changed(true);
}


//---------------------------------------------------------------------------
//
void KScreenSaver::slotLock( bool l )
{
    mLock = l;
    mLockLbl->setEnabled( l );
    mWaitLockEdit->setEnabled( l );
    mChanged = true;
    emit changed(true);
}

//---------------------------------------------------------------------------
//
void KScreenSaver::slotSetupDone(K3Process *)
{
    mPrevSelected = -1;  // see ugly hack in slotPreviewExited()
    setMonitor();
    mSetupBt->setEnabled( true );
    emit changed(true);
}

#include "scrnsave.moc"
