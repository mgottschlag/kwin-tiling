//-----------------------------------------------------------------------------
//
// KDE Display screen saver setup module
//
// Copyright (c)  Martin R. Jones 1996
//

#ifndef __SCRNSAVE_H__
#define __SCRNSAVE_H__

#include <qwidget.h>
#include <qxembed.h>
#include <kcmodule.h>

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
class KSSMonitor : public QXEmbed
{
    Q_OBJECT
public:
    KSSMonitor( QWidget *parent ) : QXEmbed( parent ) {}

    // we don't want no steenking palette change
    virtual void setPalette( const QPalette & ) {}
};

//===========================================================================
class SaverConfig
{
public:
    SaverConfig();

    bool read(const QString &file);

    QString exec() const { return mExec; }
    QString setup() const { return mSetup; }
    QString saver() const { return mSaver; }
    QString name() const { return mName; }
    QString file() const { return mFile; }
    QString category() const { return mCategory; }

protected:
    QString mExec;
    QString mSetup;
    QString mSaver;
    QString mName;
    QString mFile;
    QString mCategory;
};

//===========================================================================
class SaverList : public QPtrList<SaverConfig>
{
protected:
    virtual int compareItems(QPtrCollection::Item item1, QPtrCollection::Item item2);
};

//===========================================================================
class TestWin : public QXEmbed
{
    Q_OBJECT
public:
    TestWin();
};

//===========================================================================
class KScreenSaverAdvancedDialog : public KDialogBase
{
    Q_OBJECT
public:
    KScreenSaverAdvancedDialog(QWidget *parent, const char* name = 0);
      
public slots:
    void slotOk();
         
protected slots:
    void slotPriorityChanged( int val );
    void slotChangeBottomRightCorner( bool );
    void slotChangeBottomLeftCorner( bool );
    void slotChangeTopRightCorner( bool );
    void slotChangeTopLeftCorner( bool );
                        
private:
    void readSettings();
                     
    QCheckBox *m_topLeftCorner;
    QCheckBox *m_bottomLeftCorner;
    QCheckBox *m_topRightCorner;
    QCheckBox *m_bottomRightCorner;
    QSlider   *mPrioritySlider;
                                          
    bool mTopLeftCorner;
    bool mTopRightCorner;
    bool mBottomLeftCorner;
    bool mBottomRightCorner;
    bool mChanged;
    int  mPriority;
};

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
