//-----------------------------------------------------------------------------
//
// KDE Display screen saver setup module
//
// Copyright (c)  Martin R. Jones 1996,1999
//
// Converted to a kcc module by Matthias Hoelzer 1997
//

#include <kprocess.h>

#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <qgroupbox.h>
#include <qbuttongroup.h>
#include <qlabel.h>
#include <qpixmap.h>
#include <qpushbutton.h>
#include <qfiledialog.h>
#include <qradiobutton.h>
#include <qcheckbox.h>
#include <qslider.h>
#include <qlayout.h>
#include <qtextstream.h>
#include <ksimpleconfig.h>
#include <kapp.h>
#include <kstddirs.h>
#include <kglobal.h>
#include <kwm.h>
#include <stdlib.h>
#include <X11/Xlib.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <kiconloader.h>
#include <kcontrol.h>

#include "kcolordlg.h"
#include "scrnsave.h"
#include <klocale.h>
#include <kconfig.h>
#include "scrnsave.moc"

#define CORNER_SIZE		15

//===========================================================================
//
CornerButton::CornerButton( QWidget *parent, int num, char _action )
	: QLabel( parent )
{
	popupMenu.insertItem(  i18n("Ignore"), 'i' );
	popupMenu.insertItem(  i18n("Save Screen"), 's' );
	popupMenu.insertItem(  i18n("Lock Screen"), 'l' );
    
	connect( &popupMenu, SIGNAL( activated( int ) ),
             SLOT( slotActionSelected( int ) ) );
    
	number = num;
	action = _action;
    
	setActionText();
    
	setAlignment( AlignCenter );
}

void CornerButton::setActionText()
{
	switch ( action ) {
    case 'i':
        setText( "" );
        break;
        
    case 's':
        setText( "s" );
        break;
        
    case 'l':
        setText( "l" );
        break;
	}
}

void CornerButton::mousePressEvent( QMouseEvent *me )
{
	QPoint p = mapToGlobal( me->pos() );

	popupMenu.popup( p );
}

void CornerButton::slotActionSelected( int a )
{
	action = a;
	setActionText();
	emit cornerAction( number, action );
}


int discardError(Display *, XErrorEvent *)
{
	return 0;
}

//===========================================================================
//
//
SaverConfig::SaverConfig()
{
}

bool SaverConfig::read(QString file)
{
    KSimpleConfig config(file, true);
    
    config.setDesktopGroup();
    mExec = config.readEntry("Exec");
    mSetup = config.readEntry("Exec-setup");
    mSaver = config.readEntry("Exec-kss");
    mName = config.readEntry("Name");
    
    int indx = file.findRev('/');
    if (indx >= 0) {
        mFile = file.mid(indx+1);
    }
    
    return !mSaver.isEmpty();
}

//===========================================================================
//
TestWin::TestWin()
    : QWidget(0, 0, WStyle_Customize | WStyle_NoBorder)
{
    setFocusPolicy(StrongFocus);
}

void TestWin::mousePressEvent(QMouseEvent *)
{
    emit stopTest();
}

void TestWin::keyPressEvent(QKeyEvent *)
{
    emit stopTest();
}


//===========================================================================
//
KScreenSaver::KScreenSaver( QWidget *parent, int mode, int desktop )
	: KDisplayModule( parent, mode, desktop )
{
	if (mode == Init) 
        return;

    mTestWin = 0;
    mTestProc = 0;

    KGlobal::dirs()->addResourceType("scrsav",
                                     KGlobal::dirs()->kde_default("apps") +
                                     "ScreenSavers/");

	readSettings();
	
	setName( i18n("Screen Saver").ascii() );

	mSetupProc = new KProcess;
	connect(mSetupProc, SIGNAL(processExited(KProcess *)),
            this, SLOT(slotSetupDone(KProcess *)));
    
	mPreviewProc = new KProcess;
	connect(mPreviewProc, SIGNAL(processExited(KProcess *)),
            this, SLOT(slotPreviewExited(KProcess *)));
    
	findSavers();
    
	QGridLayout *topLayout = new QGridLayout( this, 4, 4, 10 );
	
	topLayout->setRowStretch(0,0);
	topLayout->setRowStretch(1,10);
	topLayout->setRowStretch(2,100);
	topLayout->setRowStretch(3,0);
	
	topLayout->setColStretch(0,0);
	topLayout->setColStretch(1,10);
	topLayout->setColStretch(2,10);
	topLayout->setColStretch(3,0);
    
	mMonitorLabel = new QLabel( this );
	mMonitorLabel->setAlignment( AlignCenter );
	mMonitorLabel->setPixmap(Icon(locate("data","kcontrol/pics/monitor.xpm"))); 
	mMonitorLabel->setMinimumSize( 220, 160 );
    
	topLayout->addMultiCellWidget( mMonitorLabel, 1, 1, 1, 2 );
    
	mMonitor = new KSSMonitor( mMonitorLabel );
	mMonitor->setBackgroundColor( black );
	mMonitor->setGeometry( (mMonitorLabel->width()-200)/2+20,
                           (mMonitorLabel->height()-160)/2+10, 157, 111 );
    
	CornerButton *corner = new CornerButton( mMonitor, 0, mCornerAction[0].latin1() );
	corner->setGeometry( 0, 0, CORNER_SIZE, CORNER_SIZE );
	connect( corner, SIGNAL( cornerAction( int, char ) ),
             SLOT( slotCornerAction( int, char ) ) );
    
	corner = new CornerButton( mMonitor, 1, mCornerAction[1].latin1() );
	corner->setGeometry( mMonitor->width()-CORNER_SIZE, 0, CORNER_SIZE, CORNER_SIZE );
	connect( corner, SIGNAL( cornerAction( int, char ) ),
             SLOT( slotCornerAction( int, char ) ) );
    
	corner = new CornerButton( mMonitor, 2, mCornerAction[2].latin1() );
	corner->setGeometry( 0, mMonitor->height()-CORNER_SIZE, CORNER_SIZE, CORNER_SIZE );
	connect( corner, SIGNAL( cornerAction( int, char ) ),
             SLOT( slotCornerAction( int, char ) ) );
    
	corner = new CornerButton( mMonitor, 3, mCornerAction[3].latin1() );
	corner->setGeometry( mMonitor->width()-CORNER_SIZE, mMonitor->height()-CORNER_SIZE, CORNER_SIZE, CORNER_SIZE );
	connect( corner, SIGNAL( cornerAction( int, char ) ),
             SLOT( slotCornerAction( int, char ) ) );
    
	QGroupBox *group = new QGroupBox(  i18n("Screen Saver"), this );
	
	topLayout->addWidget( group, 2, 1 );
	
	QBoxLayout *groupLayout = new QVBoxLayout( group, 10, 5 );
    
	mSaverListBox = new QListBox( group );
	mSaverListBox->insertItem( i18n("No screensaver"), 0 );
	mSaverListBox->setCurrentItem( 0 );
	mSaverListBox->adjustSize();
	mSaverListBox->setMinimumSize(mSaverListBox->size());

    SaverConfig *saver;
    mSelected = 0;
    for (saver = mSaverList.first(); saver != 0; saver = mSaverList.next())
    {
        mSaverListBox->insertItem(saver->name());
        if (saver->file() == mSaver && mEnabled) 
            mSelected = mSaverListBox->count()-1;
    }
    if (mSelected == 0) 
        mEnabled = false;
    
    mSaverListBox->setCurrentItem(mSelected);
	mSaverListBox->setTopItem(mSaverListBox->currentItem());
    mSelected = mSaverListBox->currentItem();
	connect( mSaverListBox, SIGNAL( highlighted( int ) ),
             SLOT( slotScreenSaver( int ) ) );
	
	groupLayout->addSpacing(  20 );		
	groupLayout->addWidget( mSaverListBox, 20 );
    
	mSetupBt = new QPushButton(  i18n("&Setup ..."), group );
	mSetupBt->adjustSize();
	mSetupBt->setFixedHeight( mSetupBt->height() );
	mSetupBt->setMinimumWidth(mSetupBt->width());
	connect( mSetupBt, SIGNAL( clicked() ), SLOT( slotSetup() ) );
	
	groupLayout->addWidget( mSetupBt );
    
	mTestBt = new QPushButton(  i18n("&Test"), group );
	mTestBt->adjustSize();
	mTestBt->setFixedHeight( mTestBt->height() );
	mTestBt->setMinimumWidth(mTestBt->width());
	connect( mTestBt, SIGNAL( clicked() ), SLOT( slotTest() ) );
	
	groupLayout->addWidget( mTestBt );
	groupLayout->activate();
    
	QBoxLayout *stackLayout = new QVBoxLayout( 5 );
	
	topLayout->addLayout( stackLayout, 2, 2 );

	group = new QGroupBox(  i18n("Settings"), this );
	
	stackLayout->addWidget( group, 15 );
	
	groupLayout = new QVBoxLayout( group, 10, 5 );
	
	QBoxLayout *pushLayout = new QHBoxLayout( 5 );
	
	groupLayout->addSpacing( 10 );
	groupLayout->addLayout( pushLayout );
	
	mWaitEdit = new QLineEdit( group );
	QString str;
	str.setNum( mTimeout/60 );
	mWaitEdit->setText( str );
	mWaitEdit->setMaxLength(4);
    connect( mWaitEdit, SIGNAL( textChanged( const QString & ) ),
             SLOT( slotTimeoutChanged( const QString & ) ) );
    
    QLabel *label = new QLabel( mWaitEdit, i18n("&Wait for"), group );
    
	pushLayout->addWidget( label );		
	pushLayout->addWidget( mWaitEdit, 10 );
	
	label = new QLabel(  i18n("min."), group );
	
	pushLayout->addWidget( label );
    
	mLockCheckBox = new QCheckBox( i18n("&Require password"), group );
	mLockCheckBox->setChecked( mLock );
	connect( mLockCheckBox, SIGNAL( toggled( bool ) ), SLOT( slotLock( bool ) ) );
	groupLayout->addWidget( mLockCheckBox );
    
	mStarsCheckBox = new QCheckBox( i18n("Show &password as stars"), group );
	mStarsCheckBox->setChecked(mPasswordStars);
	connect( mStarsCheckBox, SIGNAL( toggled( bool ) ), SLOT( slotStars( bool ) ) );
	groupLayout->addWidget( mStarsCheckBox );
    
	groupLayout->activate();
    
	group = new QGroupBox(  i18n("Priority"), this );
	
	stackLayout->addWidget( group, 10 );
	
	groupLayout = new QHBoxLayout( group, 10 );
    
	mPrioritySlider = new QSlider( QSlider::Horizontal, group );
	mPrioritySlider->setRange( 0, 20 );
	mPrioritySlider->setSteps( 5, 10 );
	mPrioritySlider->setValue( mPriority );
	connect( mPrioritySlider, SIGNAL( valueChanged(int) ),
             SLOT( slotPriorityChanged(int) ) );
    
	label = new QLabel( mPrioritySlider, i18n("&High"), group );
	
	mPrioritySlider->setFixedHeight( mPrioritySlider->sizeHint().height() );
	label->setFixedHeight( mPrioritySlider->sizeHint().height() );
	label->setMinimumWidth( label->sizeHint().width() );
	
	groupLayout->addWidget( label );
	groupLayout->addWidget( mPrioritySlider, 10 );
    
	label = new QLabel(  i18n("Low"), group );
	label->setFixedHeight( mPrioritySlider->sizeHint().height() );
	label->setMinimumWidth( label->sizeHint().width() );
	
	groupLayout->addWidget( label );

	// I have to call show() here, otherwise the screensaver
	// does not get the correct size information.
	show();
    
	setMonitor();
}

void KScreenSaver::resizeEvent( QResizeEvent * )
{
    mMonitor->setGeometry( (mMonitorLabel->width()-200)/2+20,
                           (mMonitorLabel->height()-160)/2+10, 157, 111 );
}

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
void KScreenSaver::readSettings( int )
{
	KConfig *config = new KConfig( "kdesktoprc");
	config->setGroup( "ScreenSaver" );
    
	mEnabled = config->readBoolEntry("Enabled", false);
	mLock = config->readBoolEntry("Lock", false);
	mTimeout = config->readNumEntry("Timeout", 300);
	mPriority = config->readNumEntry("Priority", 0);
	mCornerAction = config->readEntry("CornerAction", "iiii");
	mPasswordStars = config->readBoolEntry("PasswordAsStars", true);
	mSaver = config->readEntry("Saver");
    
	if (mPriority < 0) mPriority = 0;
	if (mPriority > 19) mPriority = 19;
    if (mTimeout < 60) mTimeout = 60;

	delete config;
}

//---------------------------------------------------------------------------
//
void KScreenSaver::updateValues()
{
	QString str;
	str.setNum(mTimeout/60);
	mWaitEdit->setText(str);
	mLockCheckBox->setChecked(mLock);
	mStarsCheckBox->setChecked(mPasswordStars);
	mPrioritySlider->setValue(mPriority);
}

//---------------------------------------------------------------------------
//
void KScreenSaver::setDefaults()
{
	slotScreenSaver( 0 );
	mSaverListBox->setCurrentItem( 0 );
	mSaverListBox->centerCurrentItem();
	slotTimeoutChanged( "1" );
	slotPriorityChanged( 0 );
	slotLock( false );
	slotStars( true );
	slotCornerAction( 0, 'i' );
	slotCornerAction( 1, 'i' );
	slotCornerAction( 2, 'i' );
	slotCornerAction( 3, 'i' );
	updateValues();
}

//---------------------------------------------------------------------------
//
void KScreenSaver::defaultSettings()
{
	setDefaults();
}

//---------------------------------------------------------------------------
//
void KScreenSaver::writeSettings()
{
	if ( !mChanged )
		return;

	KConfig *config = new KConfig( "kdesktoprc");
	config->setGroup( "ScreenSaver" );

	config->writeEntry("Enabled", mEnabled);
	config->writeEntry("Timeout", mTimeout);
	config->writeEntry("Lock", mLock);
	config->writeEntry("Priority", mPriority);
	config->writeEntry("CornerAction", mCornerAction);
	config->writeEntry("PasswordAsStars", mPasswordStars);
    config->writeEntry("Saver", mSaver);
	config->sync();

	mChanged = false;
}

//---------------------------------------------------------------------------
//
void KScreenSaver::findSavers()
{
	QStringList saverFileList = KGlobal::dirs()->findAllResources("scrsav",
                                                                "*.desktop");
    
	QStringList::Iterator it = saverFileList.begin();
	for ( ; it != saverFileList.end(); ++it ) {
        SaverConfig *saver = new SaverConfig;
        if (saver->read(*it)) {
            mSaverList.append(saver);
        }
        else 
            delete saver;
	}
}

//---------------------------------------------------------------------------
//
void KScreenSaver::slotApply()
{
	apply();
	writeSettings();
}

//---------------------------------------------------------------------------
//
void KScreenSaver::apply( bool force )
{
	if ( !mChanged && !force )
		return;

    KWM::sendKWMCommand("kss_reconfigure");
}

//---------------------------------------------------------------------------
//
void KScreenSaver::setMonitor()
{
	if (mPreviewProc->isRunning())
	    // CC: this will automatically cause a "slotPreviewExited"
	    // when the viewer exits
	    mPreviewProc->kill( );
	else
	    slotPreviewExited(mPreviewProc);
}

//---------------------------------------------------------------------------
//
void KScreenSaver::slotPreviewExited(KProcess *)
{
    mMonitor->setBackgroundColor(black);
    mMonitor->erase();

    if (mEnabled)
    {
        mPreviewProc->clearArguments();
        
        QString saver = mSaverList.at(mSelected-1)->saver();
        QTextStream ts(&saver, IO_ReadOnly);
        
        QString word;
        ts >> word;
        QString path = locate("exe", word);

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
}

//---------------------------------------------------------------------------
//
void KScreenSaver::slotScreenSaver(int indx)
{
	if ( indx == 0 ) {
		mSetupBt->setEnabled( false );
		mTestBt->setEnabled( false );
		mEnabled = false;
	}
	else {
		if (!mSetupProc->isRunning())
			mSetupBt->setEnabled( true );
		mTestBt->setEnabled( true );
        mSaver = mSaverList.at(indx - 1)->file();
		mEnabled = true;
	}
    
    mSelected = indx;
    
	setMonitor();

	mChanged = true;
}

//---------------------------------------------------------------------------
//
void KScreenSaver::slotSetup()
{
	if ( !mEnabled )
	    return;

	if (mSetupProc->isRunning())
	    return;
	
    mSetupProc->clearArguments();

    QString saver = mSaverList.at(mSelected-1)->setup();
    QTextStream ts(&saver, IO_ReadOnly);

    QString word;
    ts >> word;
    QString path = locate("exe", word);

    if (!path.isEmpty())
    {
        (*mSetupProc) << path;

        while (!ts.atEnd())
        {
            ts >> word;
            (*mSetupProc) << word;
        }

        mSetupBt->setEnabled( FALSE );
        kapp->flushX();

        mSetupProc->start();
    }
}

//---------------------------------------------------------------------------
//
void KScreenSaver::slotTest()
{
    if (!mTestProc) {
	    mTestProc = new KProcess;
    }

	mTestProc->clearArguments();
    QString saver = mSaverList.at(mSelected-1)->saver();
    QTextStream ts(&saver, IO_ReadOnly);

    QString word;
    ts >> word;
    QString path = locate("exe", word);

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

        mTestBt->setEnabled( FALSE );

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
    mTestWin->releaseKeyboard();
    mTestWin->hide();
	mTestBt->setEnabled(true);
}

//---------------------------------------------------------------------------
//
void KScreenSaver::slotTimeoutChanged( const QString &to )
{
	mTimeout = to.toInt() * 60;

	if ( mTimeout <= 0 )
		mTimeout = 60;
	mChanged = true;
}

//---------------------------------------------------------------------------
//
void KScreenSaver::slotLock( bool l )
{
	mLock = l;
	mChanged = true;
}

//---------------------------------------------------------------------------
//
void KScreenSaver::slotStars( bool s )
{
	mPasswordStars = s;
	mChanged = true;
}

//---------------------------------------------------------------------------
//
void KScreenSaver::slotPriorityChanged( int val )
{
	if ( val != mPriority )
		mChanged = true;
	
	mPriority = val;

	if ( mPriority > 19 )
		mPriority = 19;
}

//---------------------------------------------------------------------------
//
void KScreenSaver::slotSetupDone(KProcess *)
{
	setMonitor();
	mSetupBt->setEnabled( true );
}

//---------------------------------------------------------------------------
//
void KScreenSaver::slotHelp()
{
	kapp->invokeHTMLHelp( "kcmdisplay/index-4.html", "" );
}

//---------------------------------------------------------------------------
//
void KScreenSaver::slotCornerAction( int num, char action )
{
	mCornerAction[num] = action;
	mChanged = true;
}

//---------------------------------------------------------------------------
//
void KScreenSaver::loadSettings()
{
}

//---------------------------------------------------------------------------
//
void KScreenSaver::applySettings()
{
  writeSettings();
  apply(true);
}

