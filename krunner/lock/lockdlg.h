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
#include "lockdlgimpl.h"

//===========================================================================
//
// Simple dialog for entering a password.
// It does not handle password validation.
//
class PasswordDlg : public LockDlgImpl
{
    Q_OBJECT

public:
    PasswordDlg(QWidget *parent, bool msess);
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

private:
    void startCheckPassword();
    QString labelText();

    int         mFailedTimerId;
    int         mTimeoutTimerId;
    KProcess    mPassProc;
};

#endif

