/*****************************************************************
ksmserver - the KDE session management server

Copyright (C) 2000 Matthias Ettrich <ettrich@kde.org>
******************************************************************/

#ifndef SHUTDOWN_H
#define SHUTDOWN_H

#include <qpixmap.h>
#include <qdialog.h>
class QCheckBox;
class QRadioButton;
class QVButtonGroup;

#include <kapplication.h>

// The (singleton) widget that makes the desktop gray.
class KSMShutdownFeedback : public QWidget
{
    Q_OBJECT

public:
    static void start() { s_pSelf = new KSMShutdownFeedback(); s_pSelf->show(); }
    static void stop() { delete s_pSelf; s_pSelf = 0L; }
    static KSMShutdownFeedback * self() { return s_pSelf; }

protected:
    ~KSMShutdownFeedback() {}

private:
    void paintEvent( QPaintEvent* );
    static KSMShutdownFeedback * s_pSelf;
    KSMShutdownFeedback();
};

// The confirmation dialog
class KSMShutdownDlg : public QDialog
{
    Q_OBJECT

public:
    static bool confirmShutdown( bool& saveSession, bool maysd, bool maynuke, KApplication::ShutdownType& sdtype, KApplication::ShutdownMode& sdmode );

public slots:
    void slotSdMode(int);

protected:
    ~KSMShutdownDlg() {};
    virtual void showEvent( QShowEvent * );
    virtual void hideEvent( QHideEvent * );

private:
    KSMShutdownDlg( QWidget* parent, bool saveSession, bool maysd, bool maynuke, KApplication::ShutdownType sdtype, KApplication::ShutdownMode sdmode );
    QCheckBox* checkbox;
    QRadioButton *rLogout, *rHalt, *rReboot, *rSched, *rTry, *rForce;
    QVButtonGroup *mgrp;
    QPixmap pm;
};

#endif
