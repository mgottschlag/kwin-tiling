//-----------------------------------------------------------------------------
//
// KDE Display screen saver setup module
//
// Copyright (c)  Martin R. Jones 1996,1999
//
// Converted to a kcc module by Matthias Hoelzer 1997
//


#include <config.h>

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/wait.h>

#include <qbuttongroup.h>
#include <qlabel.h>
#include <qpixmap.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qcheckbox.h>
#include <qslider.h>
#include <qlayout.h>
#include <qtextstream.h>
#include <qwhatsthis.h>

#include <kapplication.h>
#include <kdebug.h>
#include <kprocess.h>
#include <ksimpleconfig.h>
#include <knuminput.h>
#include <kstandarddirs.h>
#include <klocale.h>
#include <kdesktopfile.h>
#include <kcolordialog.h>
#include <kiconloader.h>
#include <kcmodule.h>
#include <kglobal.h>
#include <dcopclient.h>
#include <kservicegroup.h>
#include <kgenericfactory.h>
#include <kwin.h>

#include <X11/Xlib.h>

#include "scrnsave.h"

// X11 headers
#undef Above
#undef Below
#undef None

template class QPtrList<SaverConfig>;

//===========================================================================
// DLL Interface for kcontrol
typedef KGenericFactory<KScreenSaver, QWidget > KSSFactory;
K_EXPORT_COMPONENT_FACTORY (kcm_screensaver, KSSFactory("kcmscreensaver") );


static QString findExe(const QString &exe) {
    QString result = locate("exe", exe);
    if (result.isEmpty())
        result = KStandardDirs::findExe(exe);
    return result;
}

//===========================================================================
//
//
SaverConfig::SaverConfig()
{
}

bool SaverConfig::read(QString file)
{
    KDesktopFile config(file, true);
    mExec = config.readEntry("Exec");
    mName = config.readEntry("Name");

    if (config.hasActionGroup("Setup"))
    {
      config.setActionGroup("Setup");
      mSetup = config.readEntry("Exec");
    }

    if (config.hasActionGroup("InWindow"))
    {
      config.setActionGroup("InWindow");
      mSaver = config.readEntry("Exec");
    }

    int indx = file.findRev('/');
    if (indx >= 0) {
        mFile = file.mid(indx+1);
    }

    return !mSaver.isEmpty();
}

//===========================================================================
//
int SaverList::compareItems(QPtrCollection::Item item1, QPtrCollection::Item item2)
{
    SaverConfig *s1 = (SaverConfig *)item1;
    SaverConfig *s2 = (SaverConfig *)item2;

    return s1->name().localeAwareCompare(s2->name());
}

//===========================================================================
//
TestWin::TestWin()
    : QXEmbed(0, 0, WStyle_Customize | WStyle_NoBorder)
{
    setFocusPolicy(StrongFocus);
    KWin::setState( winId(), NET::StaysOnTop );
    grabMouse();
}

void TestWin::mousePressEvent(QMouseEvent *)
{
    releaseMouse();
    emit stopTest();
}

void TestWin::keyPressEvent(QKeyEvent *)
{
    releaseMouse();
    emit stopTest();
}


//===========================================================================
//

KScreenSaver::KScreenSaver(QWidget *parent, const char *name, const QStringList&)
    : KCModule(KSSFactory::instance(), parent, name)
{
    mSetupProc = 0;
    mPreviewProc = 0;
    mTestWin = 0;
    mTestProc = 0;
    mPrevSelected = -2;
    mMonitor = 0;

    // Add non-KDE path
    KGlobal::dirs()->addResourceType("scrsav",
                                     KGlobal::dirs()->kde_default("apps") +
                                     "apps/ScreenSavers/");

    // Add KDE specific screensaver path
    QString relPath="System/ScreenSavers/";
    KServiceGroup::Ptr servGroup = KServiceGroup::baseGroup( "screensavers" );
    if (servGroup)
    {
      relPath=servGroup->relPath();
      kdDebug() << "relPath=" << relPath << endl;
    }

    KGlobal::dirs()->addResourceType("scrsav",
                                     KGlobal::dirs()->kde_default("apps") +
                                     relPath);

    readSettings();

    mSetupProc = new KProcess;
    connect(mSetupProc, SIGNAL(processExited(KProcess *)),
            this, SLOT(slotSetupDone(KProcess *)));

    mPreviewProc = new KProcess;
    connect(mPreviewProc, SIGNAL(processExited(KProcess *)),
            this, SLOT(slotPreviewExited(KProcess *)));


    QBoxLayout *topLayout = new QVBoxLayout(this, 10, 10);
    
    QBoxLayout *helperLayout = new QHBoxLayout(topLayout, 10);

    // left column
    QBoxLayout *vLayout = new QVBoxLayout(helperLayout, 10);

    mSaverGroup = new QGroupBox(i18n("Screen Saver"), this );
    vLayout->addWidget(mSaverGroup);
    QBoxLayout *groupLayout = new QVBoxLayout( mSaverGroup, 10 );
    groupLayout->addSpacing(10);

    mSaverListBox = new QListBox( mSaverGroup );
    mSelected = -1;
    groupLayout->addWidget( mSaverListBox, 10 );
    QWhatsThis::add( mSaverListBox, i18n("This is a list of the available"
      " screen savers. Select the one you want to use.") );

    QBoxLayout* hlay = new QHBoxLayout(groupLayout, 10);
    mSetupBt = new QPushButton(  i18n("&Setup..."), mSaverGroup );
    connect( mSetupBt, SIGNAL( clicked() ), SLOT( slotSetup() ) );
    mSetupBt->setEnabled(false);
    hlay->addWidget( mSetupBt );
    QWhatsThis::add( mSetupBt, i18n("If the screen saver you selected has"
      " customizable features, you can set them up by clicking this button.") );

    mTestBt = new QPushButton(  i18n("&Test"), mSaverGroup );
    connect( mTestBt, SIGNAL( clicked() ), SLOT( slotTest() ) );
    mTestBt->setEnabled(false);
    hlay->addWidget( mTestBt );
    QWhatsThis::add( mTestBt, i18n("You can try out the screen saver by clicking"
      " this button. (Also, the preview image shows you what the screen saver"
      " will look like.)") );

    // right column
    vLayout = new QVBoxLayout(helperLayout, 10);

    mMonitorLabel = new QLabel( this );
    mMonitorLabel->setAlignment( AlignCenter );
    mMonitorLabel->setPixmap( QPixmap(locate("data",
                         "kcontrol/pics/monitor.png")));
    vLayout->addWidget(mMonitorLabel, 0);
    QWhatsThis::add( mMonitorLabel, i18n("Here you can see a preview of the selected screen saver.") );

    mSettingsGroup = new QGroupBox( i18n("Settings"), this );
    mSettingsGroup->setColumnLayout( 0, Qt::Vertical );
    vLayout->addWidget( mSettingsGroup );
    groupLayout = new QVBoxLayout( mSettingsGroup->layout(), 10 );


    mEnabledCheckBox = new QCheckBox(i18n("Start screensaver a&utomatically"), mSettingsGroup);
    mEnabledCheckBox->setChecked(mEnabled);
    QWhatsThis::add( mEnabledCheckBox, i18n("When you check this option, the selected screensaver will be started"
                                            " automatically after a certain number of minutes of inactivity."
                                            " This time out period can be defined in the spinbox below") );
    connect(mEnabledCheckBox, SIGNAL(toggled(bool)), this, SLOT(slotEnable(bool)));
    groupLayout->addWidget(mEnabledCheckBox);
    
    QBoxLayout *hbox = new QHBoxLayout();
    groupLayout->addLayout(hbox);
    hbox->addSpacing(30);
    mActivateLbl = new QLabel(i18n("After:"), mSettingsGroup);
    mActivateLbl->setEnabled(mEnabled);
    hbox->addWidget(mActivateLbl);
    mWaitEdit = new QSpinBox(mSettingsGroup);
    mWaitEdit->setSteps(1, 10);
    mWaitEdit->setRange(1, 120);
    mWaitEdit->setSuffix(i18n(" minutes"));
    mWaitEdit->setSpecialValueText(i18n("1 minute"));
    mWaitEdit->setValue(mTimeout/60);
    mWaitEdit->setEnabled(mEnabled);
    connect(mWaitEdit, SIGNAL(valueChanged(int)), SLOT(slotTimeoutChanged(int)));
    mActivateLbl->setBuddy(mWaitEdit);
    hbox->addWidget(mWaitEdit);
    hbox->addStretch(1);
    QString wtstr = i18n("Choose the period of inactivity (from 1"
      " to 120 minutes) after which the screen saver should start."
      "To prevent the screensaver from automatically starting, choose zero minutes.");
    QWhatsThis::add( mActivateLbl, wtstr );
    QWhatsThis::add( mWaitEdit, wtstr );
    
    mLockCheckBox = new QCheckBox( i18n("&Require password to stop screensaver"), mSettingsGroup );
    mLockCheckBox->setChecked( mLock );
    connect( mLockCheckBox, SIGNAL( toggled( bool ) ),
         this, SLOT( slotLock( bool ) ) );
    groupLayout->addWidget(mLockCheckBox);
    QWhatsThis::add( mLockCheckBox, i18n("If you check this option, the display"
      " will be locked when the screen saver starts. To restore the display,"
      " enter your account password at the prompt.") );

    QGridLayout *gl = new QGridLayout(groupLayout, 2, 4);
    gl->setColStretch( 2, 10 );

    QLabel* lbl = new QLabel(i18n("&Priority:"), mSettingsGroup);
    gl->addWidget(lbl, 0, 0);

    mPrioritySlider = new QSlider(QSlider::Horizontal, mSettingsGroup);
    mPrioritySlider->setRange(0, 19);
    mPrioritySlider->setSteps(1, 5);
    mPrioritySlider->setTickmarks(QSlider::Below);
    mPrioritySlider->setValue(19 - mPriority);
    connect(mPrioritySlider, SIGNAL( valueChanged(int)),
        SLOT(slotPriorityChanged(int)));
    lbl->setBuddy(mPrioritySlider);
    gl->addMultiCellWidget(mPrioritySlider, 0, 0, 1, 3);
    QWhatsThis::add( mPrioritySlider, i18n("Use this slider to change the"
      " processing priority for the screen saver over other jobs that are"
      " being executed in the background. For a processor-intensive screen"
      " saver, setting a higher priority may make the display smoother at"
      " the expense of other jobs.") );

#ifndef HAVE_SETPRIORITY
    lbl->setEnabled(false);
    mPrioritySlider->setEnabled(false);
#endif

    lbl = new QLabel(i18n("Low Priority", "Low"), mSettingsGroup);
    gl->addWidget(lbl, 1, 1);

#ifndef HAVE_SETPRIORITY
    lbl->setEnabled(false);
#endif

    lbl = new QLabel(i18n("High Priority", "High"), mSettingsGroup);
    gl->addWidget(lbl, 1, 3);

#ifndef HAVE_SETPRIORITY
    lbl->setEnabled(false);
#endif

    //groupLayout->addStretch(1);

    vLayout->addStretch();

    // finding the savers can take some time, so defer loading until
    // we've started up.
    mNumLoaded = 0;
    mLoadTimer = new QTimer( this );
    connect( mLoadTimer, SIGNAL(timeout()), SLOT(findSavers()) );
    mLoadTimer->start( 100 );
    mChanged = false;
    emit changed(false);
}

//---------------------------------------------------------------------------
//
void KScreenSaver::resizeEvent( QResizeEvent * )
{

  if (mMonitor)
    {
      mMonitor->setGeometry( (mMonitorLabel->width()-200)/2+23,
                 (mMonitorLabel->height()-186)/2+14, 151, 115 );
    }
}

//---------------------------------------------------------------------------
//
int KScreenSaver::buttons()
{
    return KCModule::Help | KCModule::Default | KCModule::Apply;
}

//---------------------------------------------------------------------------
//
KScreenSaver::~KScreenSaver()
{
    if (mPreviewProc)
    {
        if (mPreviewProc->isRunning())
        {
            int pid = mPreviewProc->getPid();
            mPreviewProc->kill( );
            waitpid(pid, (int *) 0,0);
        }
        delete mPreviewProc;
    }

    delete mTestProc;
    delete mSetupProc;
    delete mTestWin;
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
    for (SaverConfig* saver = mSaverList.first(); saver != 0; saver = mSaverList.next()) {
        if (saver->file() == mSaver)
        {
            mSelected = i;
            break;
        };
        i++;
    }
    if ( mSelected > -1 )
    {
      mSaverListBox->setCurrentItem(mSelected);
      slotScreenSaver(mSelected);
    }

    updateValues();
    mChanged = false;
    emit changed(false);
}

//------------------------------------------------------------After---------------
//
void KScreenSaver::readSettings()
{
    KConfig *config = new KConfig( "kdesktoprc");
    config->setGroup( "ScreenSaver" );

    mEnabled = config->readBoolEntry("Enabled", false);
    mTimeout = config->readNumEntry("Timeout", 300);
    mLock = config->readBoolEntry("Lock", false);
    mPriority = config->readNumEntry("Priority", 19);
    mSaver = config->readEntry("Saver");

    if (mPriority < 0) mPriority = 0;
    if (mPriority > 19) mPriority = 19;
    if (mTimeout < 60) mTimeout = 60;

    mChanged = false;
    delete config;
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

    mLockCheckBox->setChecked(mLock);
    mPrioritySlider->setValue(19-mPriority);
}

//---------------------------------------------------------------------------
//
void KScreenSaver::defaults()
{
    slotScreenSaver( 0 );
    mSaverListBox->setCurrentItem( 0 );
    mSaverListBox->centerCurrentItem();
    slotTimeoutChanged( 5 );
    slotPriorityChanged( 0 );
    slotLock( false );
    updateValues();

    emit changed(true);
}

//---------------------------------------------------------------------------
//
void KScreenSaver::save()
{
    if ( !mChanged )
        return;

    KConfig *config = new KConfig( "kdesktoprc");
    config->setGroup( "ScreenSaver" );

    config->writeEntry("Enabled", mEnabled);
    config->writeEntry("Timeout", mTimeout);
    config->writeEntry("Lock", mLock);
    config->writeEntry("Priority", mPriority);
    if ( !mSaver.isEmpty() )
        config->writeEntry("Saver", mSaver);
    config->sync();
    delete config;

    // TODO (GJ): When you changed anything, these two lines will give a segfault
    // on exit. I don't know why yet.

    DCOPClient *client = kapp->dcopClient();
    client->send("kdesktop", "KScreensaverIface", "configure()", "");

    mChanged = false;
    emit changed(false);
}

//---------------------------------------------------------------------------
//
void KScreenSaver::findSavers()
{
    if ( !mNumLoaded ) {
        mSaverFileList = KGlobal::dirs()->findAllResources("scrsav",
                            "*.desktop", false, true);
        if ( mSaverFileList.isEmpty() )
            mLoadTimer->stop();
        else
            mLoadTimer->start( 50 );
    }

    for ( int i = 0; i < 5 &&
            (unsigned)mNumLoaded < mSaverFileList.count();
            i++, mNumLoaded++ ) {
        QString file = mSaverFileList[mNumLoaded];
        SaverConfig *saver = new SaverConfig;
        if (saver->read(file)) {
            mSaverList.append(saver);
        } else
            delete saver;
    }

    if ( (unsigned)mNumLoaded == mSaverFileList.count() ) {
        mLoadTimer->stop();
        delete mLoadTimer;
        mSaverList.sort();

        mSelected = -1;
        mSaverListBox->clear();
        for ( SaverConfig *s = mSaverList.first(); s != 0; s = mSaverList.next())
        {
            mSaverListBox->insertItem(s->name());
            if (s->file() == mSaver)
                mSelected = mSaverListBox->count()-1;
        }

        if ( mSelected > -1 )
        {
            mSaverListBox->setCurrentItem(mSelected);
            mSaverListBox->ensureCurrentVisible();
            mSetupBt->setEnabled(!mSaverList.at(mSelected)->setup().isEmpty());
            mTestBt->setEnabled(!mSaverList.at(mSelected)->setup().isEmpty());
        }

        connect( mSaverListBox, SIGNAL( highlighted( int ) ),
                 this, SLOT( slotScreenSaver( int ) ) );

        setMonitor();
    } else {
        mSaverList.sort();
        mSaverListBox->clear();
        for (SaverConfig *s= mSaverList.first(); s!= 0; s= mSaverList.next())
        {
            mSaverListBox->insertItem(s->name());
        }
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
void KScreenSaver::slotPreviewExited(KProcess *)
{
    // Ugly hack to prevent continual respawning of savers that crash
    if (mSelected == mPrevSelected)
        return;

    if ( mSaverList.isEmpty() ) // safety check
        return;

    // Some xscreensaver hacks do something nasty to the window that
    // requires a new one to be created (or proper investigation of the
    // problem).
    if (mMonitor)
        delete mMonitor;

    mMonitor = new KSSMonitor(mMonitorLabel);
    mMonitor->setBackgroundColor(black);
    mMonitor->setGeometry((mMonitorLabel->width()-200)/2+23,
                          (mMonitorLabel->height()-186)/2+14, 151, 115);
    mMonitor->show();

    if (mSelected >= 0) {
        mPreviewProc->clearArguments();

        QString saver = mSaverList.at(mSelected)->saver();
        QTextStream ts(&saver, IO_ReadOnly);

        QString word;
        ts >> word;
        QString path = findExe(word);

        if (!path.isEmpty())
        {
            (*mPreviewProc) << path;

            while (!ts.atEnd())
            {
                ts >> word;
                if (word == "%w")
                {
                    word = word.setNum(mMonitor->winId());
                }
                (*mPreviewProc) << word;
            }

            mPreviewProc->start();
        }
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
    mChanged = true;
    emit changed(true);
}


//---------------------------------------------------------------------------
//
void KScreenSaver::slotScreenSaver(int indx)
{
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
    QTextStream ts(&saver, IO_ReadOnly);

    QString word;
    ts >> word;
    QString path = findExe(word);

    if (!path.isEmpty())
    {
        (*mSetupProc) << path;

        while (!ts.atEnd())
        {
            ts >> word;
            (*mSetupProc) << word;
        }

        mSetupBt->setEnabled( false );
        kapp->flushX();

        mSetupProc->start();
    }
}

//---------------------------------------------------------------------------
//
void KScreenSaver::slotTest()
{
    if ( mSelected == -1 )
        return;

    if (!mTestProc) {
        mTestProc = new KProcess;
    }

    mTestProc->clearArguments();
    QString saver = mSaverList.at(mSelected)->saver();
    QTextStream ts(&saver, IO_ReadOnly);

    QString word;
    ts >> word;
    QString path = findExe(word);

    if (!path.isEmpty())
    {
        (*mTestProc) << path;

        if (!mTestWin)
        {
            mTestWin = new TestWin();
            mTestWin->setBackgroundMode(QWidget::NoBackground);
            mTestWin->setGeometry(0, 0, kapp->desktop()->width(),
                                    kapp->desktop()->height());
            connect(mTestWin, SIGNAL(stopTest()), SLOT(slotStopTest()));
        }

        mTestWin->show();
        mTestWin->raise();
        mTestWin->setFocus();
        mTestWin->grabKeyboard();
        mTestWin->grabMouse();

        mTestBt->setEnabled( FALSE );
	mPreviewProc->kill();

        while (!ts.atEnd())
        {
            ts >> word;
            if (word == "%w")
            {
                word = word.setNum(mTestWin->winId());
            }
            (*mTestProc) << word;
        }

        mTestProc->start(KProcess::DontCare);
    }
}

//---------------------------------------------------------------------------
//
void KScreenSaver::slotStopTest()
{
    if (mTestProc->isRunning()) {
        mTestProc->kill();
    }
    mTestWin->releaseMouse();
    mTestWin->releaseKeyboard();
    mTestWin->hide();
    mTestBt->setEnabled(true);
    mPrevSelected = -1;
    setMonitor();
}

//---------------------------------------------------------------------------
//
void KScreenSaver::slotTimeoutChanged(int to )
{
    mTimeout = to * 60;
    mChanged = true;
    emit changed(true);
}

//---------------------------------------------------------------------------
//
void KScreenSaver::slotLock( bool l )
{
    mLock = l;
    mChanged = true;
    emit changed(true);
}

//---------------------------------------------------------------------------
//
void KScreenSaver::slotPriorityChanged( int val )
{
    if (val == mPriority)
    return;

    mPriority = 19 - val;
    if (mPriority > 19)
    mPriority = 19;
    else if (mPriority < 0)
    mPriority = 0;

    mChanged = true;
    emit changed(true);
}

//---------------------------------------------------------------------------
//
void KScreenSaver::slotSetupDone(KProcess *)
{
    mPrevSelected = -1;  // see ugly hack in slotPreviewExited()
    setMonitor();
    mSetupBt->setEnabled( true );
}

//---------------------------------------------------------------------------
//
QString KScreenSaver::quickHelp() const
{
    return i18n("<h1>Screen saver</h1> This module allows you to enable and"
       " configure a screen saver. Note that you can enable a screen saver"
       " even if you have power saving features enabled for your display.<p>"
       " Besides providing an endless variety of entertainment and"
       " preventing monitor burn-in, a screen saver also gives you a simple"
       " way to lock your display if you are going to leave it unattended"
       " for a while. If you want the screen saver to lock the screen, make sure you enable"
       " the \"Require password\" feature of the screen saver. If you don't, you can still"
       " explicitly lock the screen using the desktop's \"Lock Screen\" action.");
}

#include "scrnsave.moc"
