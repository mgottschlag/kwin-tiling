//-----------------------------------------------------------------------------
//
// KDE Display screen saver setup module
//
// Copyright (c)  Martin R. Jones 1996
//

#ifndef __SCRNSAVE_H__
#define __SCRNSAVE_H__

#include <kprocess.h>
#include <qwidget.h>
#include <qpushbutton.h>
#include <qlistbox.h>
#include <qlineedit.h>
#include <qtimer.h>
#include <qlabel.h>
#include <qpopupmenu.h>
#include <qcheckbox.h>
#include <qslider.h>

#include "display.h"

//===========================================================================
class CornerButton : public QLabel
{
    Q_OBJECT
public:
    CornerButton( QWidget *parent, int num, char _action );

signals:
    void cornerAction( int corner, char action );

protected:
    virtual void mousePressEvent( QMouseEvent * );
    void setActionText();

protected slots:
    void slotActionSelected( int );

protected:
    QPopupMenu popupMenu;
    int number;
    char action;
};

//===========================================================================
class KSSMonitor : public QWidget
{
    Q_OBJECT
public:
    KSSMonitor( QWidget *parent ) : QWidget( parent ) {}

    // we don't want no steenking palette change
    virtual void setPalette( const QPalette & ) {}
};

//===========================================================================
class SaverConfig
{
public:
    SaverConfig();

    bool read(QString file);

    QString exec() const { return mExec; }
    QString setup() const { return mSetup; }
    QString saver() const { return mSaver; }
    QString name() const { return mName; }
    QString file() const { return mFile; }

protected:
    QString mExec;
    QString mSetup;
    QString mSaver;
    QString mName;
    QString mFile;
};

//===========================================================================
class TestWin : public QWidget
{
    Q_OBJECT
public:
    TestWin();

signals:
    void stopTest();

protected:
    void mousePressEvent(QMouseEvent *);
    void keyPressEvent(QKeyEvent *);
};

//===========================================================================
class KScreenSaver : public KDisplayModule
{
    Q_OBJECT
public:
    KScreenSaver( QWidget *parent, int mode, int desktop = 0 );
    ~KScreenSaver();

    virtual void readSettings( int deskNum = 0 );
    virtual void apply( bool force = FALSE );
    virtual void loadSettings();
    virtual void applySettings();
    virtual void defaultSettings();
    virtual void updateValues();

protected slots:
    void slotApply();
    void slotScreenSaver( int );
    void slotSetup();
    void slotTest();
    void slotStopTest();
    void slotTimeoutChanged( const QString &);
    void slotLock( bool );
    void slotStars( bool );
    void slotPriorityChanged( int val );
    void slotSetupDone(KProcess*);
    void slotHelp();
    void slotCornerAction( int, char );
    // when selecting a new screensaver, the old preview will
    // be killed. -- This callback is responsible for restarting the
    // new preview
    void slotPreviewExited(KProcess *);

protected:
    void writeSettings();
    void findSavers();
    void getSaverNames();
    void setMonitor();
    void setDefaults();
    void resizeEvent( QResizeEvent * );

protected:
    TestWin     *mTestWin;
    KProcess    *mTestProc;
    KProcess    *mSetupProc;
    KProcess    *mPreviewProc;
    KSSMonitor  *mMonitor;
    QPushButton *mSetupBt;
    QPushButton *mTestBt;
    QListBox    *mSaverListBox;
    QLineEdit   *mWaitEdit;
    QSlider     *mPrioritySlider;
    QCheckBox   *mLockCheckBox;
    QCheckBox   *mStarsCheckBox;
    QLabel      *mMonitorLabel;
    QList<SaverConfig> mSaverList;

    int         mSelected;
    bool        mChanged;

    // Settings
    int         mTimeout;
    int         mPriority;
    bool        mLock;
    bool        mEnabled;
    bool        mPasswordStars;
    QString     mSaver;
    QString     mCornerAction;
};

#endif

