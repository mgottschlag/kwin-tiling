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
protected:
    KSMShutdownFeedback();
public:
    static KSMShutdownFeedback * self() {
        if ( !s_pSelf )
            s_pSelf = new KSMShutdownFeedback;
        return s_pSelf;
    }
    ~KSMShutdownFeedback() {}

    static void start();
    static void stop() { delete s_pSelf; s_pSelf = 0L; }

    // TODO more feedback (which apps have saved themselves, etc.)

signals:
    void aborted();

private:
    //virtual void paintEvent( QPaintEvent *event );
    virtual void keyPressEvent( QKeyEvent * event );
    static KSMShutdownFeedback * s_pSelf;
};

// The confirmation dialog
class KSMShutdownDlg : public QDialog
{
    Q_OBJECT
public:
    KSMShutdownDlg( QWidget* parent, bool saveSession, bool maysd, bool maynuke, KApplication::ShutdownType sdtype, KApplication::ShutdownMode sdmode );
    ~KSMShutdownDlg() {}

    static bool confirmShutdown( bool& saveSession, bool maysd, bool maynuke, KApplication::ShutdownType& sdtype, KApplication::ShutdownMode& sdmode );

    const QPixmap & pixmap() { return pm; }

public slots:
    void slotSdMode(int);

private slots:
    void requestFocus();

private:
    virtual void mousePressEvent( QMouseEvent * ){}
    virtual void showEvent( QShowEvent * e );
    QCheckBox* checkbox;
    QRadioButton *rLogout, *rHalt, *rReboot, *rSched, *rTry, *rForce;
    QVButtonGroup *mgrp;
    QPixmap pm;
};

#endif
