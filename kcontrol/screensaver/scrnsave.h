/*
* scrnsave.h
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

#ifndef SCRNSAVE__H
#define SCRNSAVE__H

#include <QWidget>
#include <QMouseEvent>
#include <QResizeEvent>
#include <QKeyEvent>

#include <KCModule>

#include "kssmonitor.h"
#include "saverconfig.h"
#include "testwin.h"
#include "ui_screensaver.h"

class QTimer;

class KProcess;
class KIntSpinBox;

class ScreenPreviewWidget;

typedef QList<SaverConfig*> SaverList;

//===========================================================================
class KScreenSaver : public KCModule, private Ui::ScreenSaver
{
    Q_OBJECT
public:
    KScreenSaver(QWidget *parent, const QVariantList &);
    ~KScreenSaver();

    virtual void load();
    virtual void save();
    virtual void defaults();

    void updateValues();
    void readSettings();

protected Q_SLOTS:
    void slotEnable( bool );
    void slotSelectionChanged();
    void slotScreenSaver( QTreeWidgetItem* );
    void slotSetup();
    void slotTest();
    void slotStopTest();
    void slotTimeoutChanged( int );
    void slotLockTimeoutChanged( int );
    void slotLock( bool );
    void slotSetupDone();
    // when selecting a new screensaver, the old preview will
    // be killed. -- This callback is responsible for restarting the
    // new preview
    void slotPreviewExited();
    void findSavers();
    void slotEnablePlasma(bool enable);
    void slotPlasmaSetup();

protected:
    void writeSettings();
    void getSaverNames();
    void setMonitor();
    void setDefaults();
    bool event(QEvent *);

    QTreeWidgetItem * treeItemForSaverFile(const QString &);
    int indexForSaverFile(const QString &);

protected:
    TestWin     *mTestWin;
    KProcess    *mTestProc;
    KProcess    *mSetupProc;
    KProcess    *mPreviewProc;
    KSSMonitor  *mMonitor;
    ScreenPreviewWidget *mMonitorPreview;
    KService::List mSaverServices;
    SaverList   mSaverList;
    QTimer      *mLoadTimer;

    int         mSelected;
    int         mPrevSelected;
    int		mNumLoaded;
    bool        mChanged;
    bool	mTesting;

    // Settings
    int         mTimeout;
    int         mLockTimeout;
    bool        mLock;
    bool        mEnabled;
    QString     mSaver;
    bool        mImmutable;
    bool        mPlasmaEnabled;
};

#endif
