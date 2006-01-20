/*****************************************************************
ksmserver - the KDE session management server

Copyright (C) 2000 Matthias Ettrich <ettrich@kde.org>
******************************************************************/

#ifndef SHUTDOWNDLG_H
#define SHUTDOWNDLG_H

#include <qdialog.h>
#include <kpushbutton.h>
#include <kworkspace.h>

class QMenu;
class QTimer;

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

    virtual void paintEvent( QPaintEvent* );

private Q_SLOTS:
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
    static bool confirmShutdown( bool maysd, KWorkSpace::ShutdownType& sdtype, QString& bopt );

public Q_SLOTS:
    void slotLogout();
    void slotHalt();
    void slotReboot();
    void slotReboot(int);

protected:
    ~KSMShutdownDlg() {};

private:
    KSMShutdownDlg( QWidget* parent, bool maysd, KWorkSpace::ShutdownType sdtype );
    KWorkSpace::ShutdownType m_shutdownType;
    QString m_bootOption;
    QMenu *targets;
    QStringList rebootOptions;
};

class KSMDelayedPushButton : public KPushButton
{
  Q_OBJECT

public:

  KSMDelayedPushButton( const KGuiItem &item, QWidget *parent );
  void setPopup( QMenu *pop);

private Q_SLOTS:
  void slotTimeout();
  void slotPressed();
  void slotReleased();

private:
  QMenu *pop;
  QTimer *popt;
};

#endif
