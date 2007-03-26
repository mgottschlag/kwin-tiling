//===========================================================================
//
// This file is part of the KDE project
//
// Copyright (c) 1999 Martin R. Jones <mjones@kde.org>
// Copyright (c) 2003 Oswald Buddenhagen <ossi@kde.org>
// Coypright (c) 2004 Chris Howells <howells@kde.org>

#ifndef __TIMEOUT_H__
#define __TIMEOUT_H__

#include <QStringList>

#include <QLayout>

class LockProcess;
class QFrame;
class QGridLayout;
class QLabel;
class QDialog;
class QProgressBar;

class AutoLogout : public QDialog
{
    Q_OBJECT

public:
    AutoLogout(LockProcess *parent);
    ~AutoLogout();
    virtual void setVisible(bool visible);
 
protected:
    virtual void timerEvent(QTimerEvent *);

private Q_SLOTS:
    void slotActivity();

private:
    void        updateInfo(int);
    QFrame      *frame;
    QGridLayout *frameLayout;
    QLabel      *mStatusLabel;
    int         mCountdownTimerId;
    int         mRemaining;
    QTimer      countDownTimer;
    QProgressBar *mProgressRemaining;
    void logout();
};

#endif

