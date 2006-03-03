/*****************************************************************

Copyright (c) 2004-2005 Aaron J. Seigo <aseigo@kde.org>
Copyright (c) 2004 Zack Rusin <zrusin@kde.org>
                   Sami Kyostil <skyostil@kempele.fi>

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

#ifndef ADDAPPLETVISUALFEEDBACK_H
#define ADDAPPLETVISUALFEEDBACK_H

#include <QBitmap>
#include <QPixmap>
#include <QTimer>
#include <QWidget>

#include <utils.h>

class AppletItem;
class QPaintEvent;
class Q3SimpleRichText;
class QTimer;

class AddAppletVisualFeedback : QWidget
{
    Q_OBJECT

    public:
        AddAppletVisualFeedback(AppletWidget* parent,
                                const QWidget* destination,
                                Plasma::Position direction);

    protected Q_SLOTS:
        void internalUpdate();
        void swoopCloser();

    protected:
        void paintEvent(QPaintEvent * e);
        void mousePressEvent(QMouseEvent * e);

        void makeMask();
        void displayInternal();

    private:
        const QWidget* m_target;
        Plasma::Position m_direction;
        QBitmap m_mask;
        QPixmap m_pixmap;
        QPixmap m_icon;
        Q3SimpleRichText* m_richText;

        int m_dissolveSize;
        int m_dissolveDelta;
        int m_frames;

        QTimer m_moveTimer;
        bool m_dirty;

        QPoint m_destination;
};

#endif
