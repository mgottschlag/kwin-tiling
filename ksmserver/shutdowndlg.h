/*****************************************************************
ksmserver - the KDE session management server

Copyright 2000 Matthias Ettrich <ettrich@kde.org>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

******************************************************************/

#ifndef SHUTDOWNDLG_H
#define SHUTDOWNDLG_H

#include <QDialog>
#include <QPushButton>
#include <kworkspace/kworkspace.h>

class QMenu;
class QTimer;
class QTimeLine;
class QLabel;
class LogoutEffect;

namespace Plasma
{
    class Svg;
    class FrameSvg;
}

// The (singleton) widget that makes the desktop gray.
class KSMShutdownFeedback : public QWidget
{
    Q_OBJECT

public:
    static void start();
    static void stop();
    static void logoutCanceled();

protected:
    ~KSMShutdownFeedback() {}

    virtual void paintEvent( QPaintEvent* );

private Q_SLOTS:
    void slotPaintEffect();
    void slotPaintEffectInitialized();

private:
    static KSMShutdownFeedback * s_pSelf;
    KSMShutdownFeedback();
    int m_currentY;
    QPixmap m_pixmap;
    LogoutEffect *effect;
    bool initialized;
};

class KSMPushButton : public QPushButton
{
    Q_OBJECT

public:
    explicit KSMPushButton( const QString &text, QWidget *parent = 0, bool smallButton = false );

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
    bool m_smallButton;
};

// The confirmation dialog
class KSMShutdownDlg : public QDialog
{
    Q_OBJECT

public:
    static bool confirmShutdown(
            bool maysd, bool choose, KWorkSpace::ShutdownType& sdtype, QString& bopt );

public Q_SLOTS:
    void slotLogout();
    void slotHalt();
    void slotReboot();
    void slotReboot(QAction*);
    void slotSuspend(QAction*);

protected:
    ~KSMShutdownDlg() {}
    void paintEvent(QPaintEvent *e);
    void resizeEvent(QResizeEvent *e);

private:
    KSMShutdownDlg( QWidget* parent, bool maysd, bool choose, KWorkSpace::ShutdownType sdtype );
    KWorkSpace::ShutdownType m_shutdownType;
    QString m_bootOption;
    QStringList rebootOptions;
    QPixmap m_renderedSvg;
    Plasma::FrameSvg* m_svg;
    QLabel *m_automaticallyDoLabel;
    QPushButton *m_lastButton;
    KSMPushButton *m_btnLogout;
    KSMPushButton *m_btnHalt;
    KSMPushButton *m_btnReboot;
    KSMPushButton *btnBack;
    int m_automaticallyDoSeconds;
    int m_pictureWidth;

private Q_SLOTS:
    void automaticallyDoTimeout();
};

#endif
