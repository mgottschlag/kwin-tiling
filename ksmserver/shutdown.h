/*****************************************************************
ksmserver - the KDE session management server

Copyright (C) 2000 Matthias Ettrich <ettrich@kde.org>
******************************************************************/

#ifndef SHUTDOWN_H
#define SHUTDOWN_H

#include <qpixmap.h>
#include <qdialog.h>
#include <qradiobutton.h>
class QCheckBox;
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

// A radiobutton with a dblclk signal
class KSMRadioButton : public QRadioButton
{
	Q_OBJECT

public:
	KSMRadioButton (const QString &text, QWidget *parent, const char *name = 0L);

private:
	virtual void mouseDoubleClickEvent (QMouseEvent *pe);
	
signals:
	void doubleClicked();
};

// The confirmation dialog
class KSMShutdownDlg : public QDialog
{
    Q_OBJECT

public:
    static bool confirmShutdown( bool maysd, bool maynuke, KApplication::ShutdownType& sdtype, KApplication::ShutdownMode& sdmode );

public slots:
    void slotSdMode(int);

protected:
    ~KSMShutdownDlg() {};

private:
    KSMShutdownDlg( QWidget* parent, bool maysd, bool maynuke, KApplication::ShutdownType sdtype, KApplication::ShutdownMode sdmode );
    QRadioButton *rLogout, *rHalt, *rReboot;
#if 0
    , *rSched, *rTry, *rForce;
    QVButtonGroup *mgrp;
#endif
};

#endif
