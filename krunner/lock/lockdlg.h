//===========================================================================
//
// This file is part of the KDE project
//
// Copyright (c) 1999 Martin R. Jones <mjones@kde.org>
//

#ifndef __LOCKDLG_H__
#define __LOCKDLG_H__

#include <qlabel.h>
#include <qdialog.h>
#include <kprocess.h>
#include <X11/Xlib.h>

class KPasswordEdit;
class QPushButton;

//===========================================================================
//
// Simple dialog for entering a password.
// It does not handle password validation.
//
class PasswordDlg : public QDialog
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

private:
    void startCheckPassword();
    QString currentUser();
    QString passwordQueryMsg();

    int         mFailedTimerId;
    int         mTimeoutTimerId;
    QLabel      *mLabel;
    KPasswordEdit      *mEntry;
    QPushButton	*mButton;
    KProcess    mPassProc;
};

#endif

