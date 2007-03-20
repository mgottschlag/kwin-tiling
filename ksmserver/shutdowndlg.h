/*****************************************************************
ksmserver - the KDE session management server

Copyright (C) 2000 Matthias Ettrich <ettrich@kde.org>
******************************************************************/

#ifndef SHUTDOWNDLG_H
#define SHUTDOWNDLG_H

#include <QDialog>
#include <QPushButton>
#include <kworkspace.h>

class QMenu;
class QTimer;
class QTimeLine;

namespace Plasma
{
    class Svg;
}

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

class KSMPushButton : public QPushButton
{
    Q_OBJECT

public:
    KSMPushButton( const QString &text, QWidget *parent = 0 );

    void setPixmap( const QPixmap & );
    void setPopupMenu( QMenu * );
protected:
    void paintEvent(QPaintEvent *e);
    void resizeEvent(QResizeEvent *e);
    bool event(QEvent *e);

    void init();
protected:
    QPixmap m_pixmap;
    bool m_highlight;
    QString m_text;
private Q_SLOTS:
    void slotPressed();
    void slotReleased();
    void slotTimeout();
    void animateGlow( qreal );
private:
    QMenu* m_popupMenu;
    QTimer* m_popupTimer;
    Plasma::Svg* m_glowSvg;
    qreal m_glowOpacity;
    QTimeLine *m_glowTimeLine;
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
    void slotSuspend(int);

protected:
    ~KSMShutdownDlg() {}
    void paintEvent(QPaintEvent *e);
    void resizeEvent(QResizeEvent *e);

private:
    KSMShutdownDlg( QWidget* parent, bool maysd, KWorkSpace::ShutdownType sdtype );
    KWorkSpace::ShutdownType m_shutdownType;
    QString m_bootOption;
    QStringList rebootOptions;
    QPixmap m_renderedSvg;
    Plasma::Svg* m_svg;
};

#endif
