//-----------------------------------------------------------------------------
//
// KDE Display screen saver setup module
//
// Copyright (c)  Martin R. Jones 1996
// Copyright (C) Chris Howells 2004
//

#ifndef __SCRNSAVE_H__
#define __SCRNSAVE_H__

#include <qwidget.h>
#include <kcmodule.h>

#include "kssmonitor.h"
#include "saverconfig.h"
#include "testwin.h"
#include "advanceddialog.h"
#include "kssmonitor.h"
#include "saverlist.h"

class QTimer;
class QSpinBox;
class QSlider;
class QCheckBox;
class QLabel;
class QListView;
class QListViewItem;
class QPushButton;
class KIntNumInput;
class KProcess;

//===========================================================================
class KScreenSaver : public KCModule
{
    Q_OBJECT
public:
    KScreenSaver(QWidget *parent, const char *name, const QStringList &);
    ~KScreenSaver();

    virtual void load();
    virtual void save();
    virtual void defaults();

    int buttons();
    void updateValues();
    void readSettings();

    QString quickHelp() const;

protected slots:
    void slotEnable( bool );
    void slotScreenSaver( QListViewItem* );
    void slotSetup();
    void slotAdvanced();
    void slotTest();
    void slotStopTest();
    void slotTimeoutChanged( int );
    void slotLockTimeoutChanged( int );
    void slotDPMS( bool );
    void slotLock( bool );
    void slotSetupDone(KProcess*);
    // when selecting a new screensaver, the old preview will
    // be killed. -- This callback is responsible for restarting the
    // new preview
    void slotPreviewExited(KProcess *);
    void findSavers();

protected:
    void writeSettings();
    void getSaverNames();
    void setMonitor();
    void setDefaults();
    void resizeEvent( QResizeEvent * );
    void mousePressEvent(QMouseEvent *);
    void keyPressEvent(QKeyEvent *);

protected:
    TestWin     *mTestWin;
    KProcess    *mTestProc;
    KProcess    *mSetupProc;
    KProcess    *mPreviewProc;
    KSSMonitor  *mMonitor;
    QPushButton *mSetupBt;
    QPushButton *mTestBt;
    QListView   *mSaverListView;
    QSpinBox	*mWaitEdit;
    QSpinBox    *mWaitLockEdit;
    QCheckBox   *mLockCheckBox;
    QCheckBox   *mStarsCheckBox;
    QCheckBox   *mEnabledCheckBox;
    QCheckBox	*mDPMSDependentCheckBox;
    QLabel      *mMonitorLabel;
    QLabel      *mActivateLbl;
    QLabel      *mLockLbl;
    QStringList mSaverFileList;
    SaverList   mSaverList;
    QTimer      *mLoadTimer;
    QGroupBox   *mSaverGroup;
    QGroupBox   *mSettingsGroup;

    int         mSelected;
    int         mPrevSelected;
    int		mNumLoaded;
    bool        mChanged;
    bool	mTesting;

    // Settings
    int         mTimeout;
    int         mLockTimeout;
    bool	mDPMS;
    bool        mLock;
    bool        mEnabled;
    QString     mSaver;
    bool        mImmutable;
};

#endif
