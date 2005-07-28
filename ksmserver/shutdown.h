/*****************************************************************
ksmserver - the KDE session management server

Copyright (C) 2000 Matthias Ettrich <ettrich@kde.org>
******************************************************************/

#ifndef SHUTDOWN_H
#define SHUTDOWN_H

#include <qpixmap.h>
#include <qdialog.h>
#include <kpushbutton.h>
#include <Q3PopupMenu>
class QPushButton;
class QVButtonGroup;
class Q3PopupMenu;
class QTimer;

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

private slots:
    void slotPaintEffect();

private:
    static KSMShutdownFeedback * s_pSelf;
    KSMShutdownFeedback();
    int m_currentY;
};


// The confirmation dialog
class KSMShutdownDlg : public QDialog
{
    Q_OBJECT

public:
    static bool confirmShutdown( bool maysd, KApplication::ShutdownType& sdtype, QString& bopt );

public slots:
    void slotLogout();
    void slotHalt();
    void slotReboot();
    void slotReboot(int);

protected:
    ~KSMShutdownDlg() {};

private:
    KSMShutdownDlg( QWidget* parent, bool maysd, KApplication::ShutdownType sdtype );
    KApplication::ShutdownType m_shutdownType;
    QString m_bootOption;
    Q3PopupMenu *targets;
    QStringList rebootOptions;
};

class KSMDelayedPushButton : public KPushButton
{
  Q_OBJECT

public:

  KSMDelayedPushButton( const KGuiItem &item, QWidget *parent, const char *name = 0 );
  void setPopup( Q3PopupMenu *pop);

private slots:
  void slotTimeout();
  void slotPressed();
  void slotReleased();

private:
  Q3PopupMenu *pop;
  QTimer *popt;
};

#endif
