/*****************************************************************

Copyright (c) 2004 Zack Rusin <zrusin@kde.org>
                   Sami Kyostil <skyostil@kempele.fi>
                   Aaron J. Seigo <aseigo@kde.org>

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

#ifndef KICKER_TIP_H
#define KICKER_TIP_H

#include <QWidget>

#include <kpanelapplet.h>

class QEvent;
class QMouseEvent;
class QPaintEvent;
class QTextEdit;
class QTimer;

class KDE_EXPORT KickerTip : public QWidget
{
    Q_OBJECT

public:
    /**
     * The two visual effects available.
     * Plain simply shows the tip, while Dissolve fades the tip in
     */
    enum MaskEffect { Plain, Dissolve };

    /**
     * Data contains the details for tip. A Data struct will be passed into
     * the Client subclass whenever the tip needs to update its content
     */
    struct Data
    {
            QString message;
            QString subtext;
            QPixmap icon;
            KickerTip::MaskEffect maskEffect;
            int duration;
            Plasma::Position direction;
    };

    /**
     * For a widget to have a mouse over tip, simply have it subclass Client.
     * Then call installEventFilter(KickerTip::self()); and the widget will be
     * tracked by the tip
     */
    class KDE_EXPORT Client
    {
        public:
            virtual ~Client() {}
            virtual void updateTipData(KickerTip::Data&) = 0;
            void updateTip() const;
    };

    /**
     * Singleton accessor. Call this whenever you need to interact with the tip.
     */
    static KickerTip* self();

    /**
     * Enables / disables tipping. For every call to turn it off/on, a
     * corresponding call to turn it on/off must be made at some point.
     * @param tip when true enables tipping, when false disables tipping
     */
    static void enableTipping(bool tip);

    /**
     * Returns whether or not tipping is currently enabled
     */
    static bool tippingEnabled();

    /**
     * The event filter installed by Clients on themselves to be tracked
     * by the mouse over tip effect.
     */
    bool eventFilter(QObject *o, QEvent *e);

protected:
    KickerTip(QWidget * parent);
    ~KickerTip();

    void paintEvent(QPaintEvent * e);
    void mousePressEvent(QMouseEvent * e);

    /**
     * Draws the tip background and outline.
     * @param p       The painter to use draw the background with
     * @param outline The outline color
     * @param fill    The brush to use for the center fill
     */
    void drawBackground(QPainter *p, const QColor &outline, const QBrush &fill) const;

    /**
     * Creates and installs a solid mask with rounded corners
     */
    void plainMask();

    /**
     * Creates and installs a progressively fading in mask with rounded corners
     */
    void dissolveMask();

    void displayInternal();
    void hide();

    /**
     * Sets the widgets currently being tipped for
     * @param w the widget that is the subject of the tipping
     */
    void tipFor(const QWidget* w);

    /**
     * If the widget passed in is currening being tipped for, then the tipping
     * is stopped
     * @param w the widget to cease tipping for
     */
    void untipFor(const QWidget* w);

    /**
     * Checks if the passed in widget is the current subject of the mouse
     * mouse over tip.
     * @param w the widget to check for
     */
    bool isTippingFor(const QWidget* w) const;

protected Q_SLOTS:
    void tipperDestroyed(QObject* o);
    void internalUpdate();
    void display();

private:
    static KickerTip* m_self;
    static int m_tippingEnabled;

    class Private;
    Private* d;
};

#endif
