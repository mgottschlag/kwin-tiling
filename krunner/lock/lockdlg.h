//===========================================================================
//
// This file is part of the KDE project
//
// Copyright (c) 1999 Martin R. Jones <mjones@kde.org>
//

#ifndef __LOCKDLG_H__
#define __LOCKDLG_H__

#include <qdialog.h>
#include <kprocess.h>
#include <X11/Xlib.h>
#include <qstringlist.h>
#include "lockdlgimpl.h"

class LockProcess;
//===========================================================================
//
// Simple dialog for entering a password.
// It does not handle password validation.
//
class PasswordDlg : public LockDlgImpl
{
    Q_OBJECT

public:
    PasswordDlg(LockProcess *parent, bool msess);
    QString checkForUtf8(QString txt);
    virtual void show();

signals:
    void startNewSession();

protected:
    virtual void timerEvent(QTimerEvent *);
    virtual bool eventFilter(QObject *, QEvent *);

protected slots:
    void passwordChecked(KProcess *);
    void slotStartNewSession();
    void slotCancel();
    void slotOK();
    void layoutClicked();

private:
    void setLayoutText( const QString &txt );
    void startCheckPassword();
    void capsLocked();
    void updateLabel();
    QString labelText();
    int         mFailedTimerId;
    int         mTimeoutTimerId;
    int         mCapsLocked;
    bool        mUnlockingFailed;
    KProcess    mPassProc;
    QStringList layoutsList;
};

#endif

