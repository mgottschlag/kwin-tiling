/*

clock module for kdm

Copyright (C) 2000 Espen Sand, espen@kde.org
  Based on work by NN(yet to be determined)
flicker free code by Remi Guyomarch <rguyom@mail.dotcom.fr>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

*/

#include "kdmclock.h"

#include <QPainter>
#include <QTime>
#include <QTimer>

KdmClock::KdmClock(QWidget *parent)
    : inherited(parent)
{
    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), SLOT(timeout()));
    timer->start(1000);

#ifdef MORE
    mDate = false;//config->readEntry("date", false);
    mSecond = true;//config->readEntry("second", true);
    mDigital = false;//config->readEntry("digital", false);
    mBorder = true;//config->readEntry("border", false);

    //config->setGroup("Font");
    mFont.setFamily(QString::fromLatin1("Utopia")/*config->readEntry("Family", "Utopia")*/);
    mFont.setPointSize(51/*config->readEntry("Point Size", 51)*/);
    mFont.setWeight(75/*config->readEntry("Weight", 75)*/);
    mFont.setItalic(true/*config->readEntry("Italic", true)*/);
    mFont.setBold(true/*config->readEntry("Bold", false)*/);

    if (mBorder)
        setFrameStyle(WinPanel | Sunken);
#endif

    setFixedSize(100, 100);

    repaint();
}


void KdmClock::showEvent(QShowEvent *)
{
    repaint();
}


void KdmClock::timeout()
{
    repaint();
}

void KdmClock::paintEvent(QPaintEvent *)
{
    QPainter p(this);
#ifdef MORE
    drawFrame(&p);
#endif

    p.setPen(palette().foreground().color());
    p.setBrush(palette().foreground());
    p.setRenderHint(QPainter::Antialiasing);

    QTime time = QTime::currentTime();

#ifdef MORE
    if (mDigital) {
        QString buf;
        if (mSecond)
            buf.sprintf("%02d:%02d:%02d", time.hour(), time.minute(),
                        time.second());
        else
            buf.sprintf("%02d:%02d", time.hour(), time.minute());
        mFont.setPointSize(qMin((int)(width() / buf.length() * 1.5), height()));
        p.setFont(mFont);
        p.drawText(contentsRect(), Qt::AlignCenter, buf);
    } else
#endif
    {
        QTransform matrix;
        QPoint cp = contentsRect().center();
        matrix.translate(cp.x(), cp.y());
        int d = qMin(contentsRect().width() - 15, contentsRect().height() - 15);
        matrix.scale(d / 1000.0F, d / 1000.0F);

        QPolygon pts;

        // Hour
        float h_angle = 30 * (time.hour() % 12 - 3) + time.minute() / 2;
        matrix.rotate(h_angle);
        p.setWorldTransform(matrix);
        pts.setPoints(4, -20, 0, 0, -20, 300, 0, 0, 20);
        p.drawPolygon(pts);
        matrix.rotate(-h_angle);

        // Minute
        float m_angle = (time.minute() - 15) * 6;
        matrix.rotate(m_angle);
        p.setWorldTransform(matrix);
        pts.setPoints(4, -10, 0, 0, -10, 400, 0, 0, 10);
        p.drawPolygon(pts);
        matrix.rotate(-m_angle);

        // Second
#ifdef MORE
        if (mSecond)
#endif
        {
            float s_angle = (time.second() - 15) * 6;
            matrix.rotate(s_angle);
            p.setWorldTransform(matrix);
            pts.setPoints(4, 0, 0, 0, 0, 400, 0, 0, 0);
            p.drawPolygon(pts);
            matrix.rotate(-s_angle);
        }

        // quadrante
        for (int i = 0; i < 60; i++) {
            p.setWorldTransform(matrix);
            if ((i % 5) == 0)
                p.drawLine(450, 0, 500, 0); // draw hour lines
            else
                p.drawPoint(480, 0); // draw second lines
            matrix.rotate(6);
        }

    } // if (mDigital)
}

#include "kdmclock.moc"
