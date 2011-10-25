/*
* scrnsave.cpp
* Copyright 1997       Matthias Hoelzer
* Copyright 1996,1999,2002    Martin R. Jones
* Copyright 2004       Chris Howells
* Copyright 2007-2008  Benjamin Meyer <ben@meyerhome.net>
* Copyright 2007-2008  Hamish Rodda <rodda@kde.org>
* Copyright 2009       Dario Andres Rodriguez <andresbajotierra@gmail.com>
* Copyright 2009       Davide Bettio
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License as
* published by the Free Software Foundation; either version 2 of
* the License or (at your option) version 3 or any later version
* accepted by the membership of KDE e.V. (or its successor approved
* by the membership of KDE e.V.), which shall act as a proxy
* defined in Section 14 of version 3 of the license.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <config-workspace.h>

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/wait.h>

#include <QCheckBox>
#include <QLabel>
#include <QPushButton>
#include <QTimer>
#include <QTreeWidget>
#include <QHeaderView>
#include <QTreeWidgetItem>
#include <QTextStream>
#include <QKeyEvent>
#include <QHBoxLayout>
#include <QBoxLayout>
#include <QVBoxLayout>
#include <QResizeEvent>
#include <QMouseEvent>
#include <QtDBus/QtDBus>

#include <QDesktopWidget>

#include <kmacroexpander.h>
#include <KShell>
#include <KServiceTypeTrader>
#include <KStandardDirs>
#include <KApplication>
#include <KDebug>
#include <KDialog>
#include <KAboutData>
#include <KIcon>
#include <KNumInput>
#include <KProcess>
//#include <KServiceGroup>
#include <KPluginFactory>
#include <KPluginLoader>

#include <kscreensaver_interface.h>
#include <kworkspace/screenpreviewwidget.h>

#include <X11/Xlib.h>
#include <fixx11h.h>

#include "scrnsave.h"
#include <QX11Info>

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
        registerPlugin<KScreenSaver>();
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

    mSetupProc = new KProcess;
    connect(mSetupProc, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(slotSetupDone()));

    mPreviewProc = new KProcess;
    connect(mPreviewProc, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(slotPreviewExited()));

    mSaverListView->setColumnCount(1);
    mSaverListView->header()->hide();
    mSelected = -1;
    connect( mSaverListView, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)), this, SLOT(slotSetup()));

    connect( mSetupBt, SIGNAL(clicked()), SLOT(slotSetup()) );
    connect( mTestBt, SIGNAL(clicked()), SLOT(slotTest()) );

    mEnabledCheckBox->setChecked(mEnabled);
    connect(mEnabledCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(slotEnable(bool)));

    mWaitEdit->setRange(1, INT_MAX);
    mWaitEdit->setSuffix(ki18ncp("unit of time. minutes until the screensaver is triggered",
                                 " minute", " minutes"));
    mWaitEdit->setValue(mTimeout/60);
    mWaitEdit->setEnabled(mEnabled);
    connect(mWaitEdit, SIGNAL(valueChanged(int)),
            this, SLOT(slotTimeoutChanged(int)));

    mLockCheckBox->setEnabled( mEnabled );
    mLockCheckBox->setChecked( mLock );
    connect( mLockCheckBox, SIGNAL(toggled(bool)),
             this, SLOT(slotLock(bool)) );

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
bool KScreenSaver::event(QEvent *e)
{
    if (e->type() == QEvent::Resize) {
        if (mMonitor)
            mMonitor->setGeometry(mMonitorPreview->previewRect());
    } else if (e->type() == QEvent::KeyPress ||
               e->type() == QEvent::MouseButtonPress) {
        if (mTesting) {
            slotStopTest();
            return true;
        }
    }
    return KCModule::event(e);
}

//---------------------------------------------------------------------------
//
KScreenSaver::~KScreenSaver()
{
    if (mPreviewProc)
    {
        if (mPreviewProc->state() == QProcess::Running)
        {
            //Avoid triggering slotPreviewExited on close
            mPreviewProc->disconnect(this);

            mPreviewProc->kill( );
            mPreviewProc->waitForFinished( );
        }
        delete mPreviewProc;
    }

    if (mSetupProc)
    {
        if (mSetupProc->state() == QProcess::Running)
        {
            //Avoid triggering slotSetupDone on close
            mSetupProc->disconnect(this);

            mSetupProc->kill( );
            mSetupProc->waitForFinished( );
        }
        delete mSetupProc;
    }

    delete mTestProc;
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

    QTreeWidgetItem * selectedItem = treeItemForSaverFile(mSaver);
    if (selectedItem) {
        mSelected = indexForSaverFile(mSaver);
        mSaverListView->setCurrentItem(selectedItem, QItemSelectionModel::ClearAndSelect);
        slotScreenSaver(selectedItem);
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
    QTreeWidgetItem *item = mSaverListView->topLevelItem(0);
    if (item) {
        mSaverListView->setCurrentItem(item, QItemSelectionModel::ClearAndSelect);
        mSaverListView->scrollToItem(item);
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
        new QTreeWidgetItem ( mSaverListView, QStringList() << i18n("Loading...") );
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

    if ( mNumLoaded != mSaverServices.count() ) {
        return;
    }

    int categoryCount = 0;

    mLoadTimer->stop();
    delete mLoadTimer;

    qSort(mSaverList.begin(), mSaverList.end());

    mSaverListView->clear();
    //Create the treewidget items
    Q_FOREACH( SaverConfig *s, mSaverList )
    {
        QTreeWidgetItem *item = 0;
        if (s->category().isEmpty()) {
            //Create top level item without category
            item = new QTreeWidgetItem ( mSaverListView, QStringList() << s->name() << '2' + s->name() );
            item->setData(0, Qt::UserRole, s->file());
        } else {
            //Create item within a category
            QList<QTreeWidgetItem*> categoryItemList = mSaverListView->findItems( s->category(), Qt::MatchExactly );
            QTreeWidgetItem * categoryItem = 0;
            if ( categoryItemList.isEmpty() ) { //Create the category toplevel item
                categoryItem = new QTreeWidgetItem ( mSaverListView, QStringList() << s->category() << '1' + s->category() );
                categoryItem->setIcon ( 0, KIcon ( "preferences-desktop-screensaver" ) );
            } else {
                categoryItem = categoryItemList.first();
            }
            //Add the child item to the category
            item = new QTreeWidgetItem ( categoryItem, QStringList() << s->name() << '2' + s->name() );
            item->setData(0, Qt::UserRole, s->file());
            categoryCount++;
        }
    }

    //Get the current Item and index
    mSelected = indexForSaverFile(mSaver);
    QTreeWidgetItem *selectedItem = treeItemForSaverFile(mSaver);

    // Delete categories with only one item
    QList<QTreeWidgetItem*> itemsToBeDeleted;
    QList<QTreeWidgetItem*> itemsToBeAddedAsTopLevel;
    QTreeWidgetItemIterator it(mSaverListView, QTreeWidgetItemIterator::HasChildren);
    while ((*it)) {
        if ((*it)->childCount() == 1) {
            QTreeWidgetItem * item = (*it)->child(0);
            (*it)->removeChild(item);
            itemsToBeAddedAsTopLevel.append(item);
            itemsToBeDeleted.append((*it));
        }
        ++it;
    }
    mSaverListView->addTopLevelItems(itemsToBeAddedAsTopLevel);
    qDeleteAll(itemsToBeDeleted);

    mSaverListView->setRootIsDecorated ( categoryCount > 0 );
    mSaverListView->sortByColumn ( 0, Qt::AscendingOrder );

    //Set the current screensaver
    if ( mSelected > -1 )
    {
        mSaverListView->setCurrentItem(selectedItem, QItemSelectionModel::ClearAndSelect);
        mSaverListView->scrollToItem(selectedItem);

        mSetupBt->setEnabled(!mSaverList.at(mSelected)->setup().isEmpty());
        mTestBt->setEnabled(true);
    }

    connect( mSaverListView, SIGNAL(itemSelectionChanged()),
             this, SLOT(slotSelectionChanged()) );

    setMonitor();

}

void KScreenSaver::slotSelectionChanged()
{
    QList<QTreeWidgetItem *> selection = mSaverListView->selectedItems();
    if (selection.isEmpty()) {
        slotScreenSaver(0);
    } else {
        slotScreenSaver(selection.at(0));
    }
}
//---------------------------------------------------------------------------
//
void KScreenSaver::setMonitor()
{
    if (mPreviewProc->state() == QProcess::Running)
    // CC: this will automatically cause a "slotPreviewExited"
    // when the viewer exits
    mPreviewProc->kill();
    else
    slotPreviewExited();
}

//---------------------------------------------------------------------------
//
void KScreenSaver::slotPreviewExited()
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
        mPreviewProc->clearProgram();

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
    mWaitEdit->setEnabled( e );
    mLockCheckBox->setEnabled( e );
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
void KScreenSaver::slotScreenSaver(QTreeWidgetItem *item)
{
    if (!item)
    {
        mSetupBt->setEnabled(false);
        mTestBt->setEnabled(false);
        return;
    }

    //Get the index of the saver file of the current selected treewidget item
    int indx = indexForSaverFile(item->data(0, Qt::UserRole).toString());

    mSetupBt->setEnabled(item->childCount()==0);
    mTestBt->setEnabled(item->childCount()==0);
    if (indx == -1) {
        mSelected = -1;
        return;
    }

    bool bChanged = (indx != mSelected);

    if (mSetupProc->state() != QProcess::Running)
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

    if (mSetupProc->state() == QProcess::Running)
    return;

    mSetupProc->clearProgram();

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
void KScreenSaver::slotTest()
{
    if ( mSelected == -1 )
        return;

    if (!mTestProc) {
        mTestProc = new KProcess;
    } else {
        mPreviewProc->kill();
        mPreviewProc->waitForFinished();
        mTestProc->clearProgram();
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

    mTestProc->start();

    mTesting = true;
}

//---------------------------------------------------------------------------
//
void KScreenSaver::slotStopTest()
{
    if (mTestProc->state() == QProcess::Running) {
        mTestProc->kill();
        mTestProc->waitForFinished(500);
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
    mWaitLockEdit->setEnabled( l );
    mChanged = true;
    emit changed(true);
}

//---------------------------------------------------------------------------
//
void KScreenSaver::slotSetupDone()
{
    mPrevSelected = -1;  // see ugly hack in slotPreviewExited()
    setMonitor();
    mSetupBt->setEnabled( true );
    emit changed(true);
}

QTreeWidgetItem * KScreenSaver::treeItemForSaverFile(const QString & saver)
{
    QTreeWidgetItem * item = 0;
    QTreeWidgetItemIterator it(mSaverListView);
    while ((*it) && item == 0) {
        if ((*it)->data(0, Qt::UserRole) == saver) {
            item = (*it);
        }
        ++it;
    }
    return item;
}

int KScreenSaver::indexForSaverFile(const QString & saver)
{
    int index = -1;
    int i = 0;
    Q_FOREACH( SaverConfig* saverConfig, mSaverList ) {
        if (saverConfig->file() == saver) {
            index = i;
            break;
        }
        i++;
    }
    return index;
}

#include "scrnsave.moc"
