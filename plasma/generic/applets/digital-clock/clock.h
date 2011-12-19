/***************************************************************************
 *   Copyright (C) 2007-2008 by Riccardo Iaconelli <riccardo@kde.org>      *
 *   Copyright (C) 2007-2008 by Sebastian Kuegler <sebas@kde.org>          *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#ifndef CLOCK_H
#define CLOCK_H

#include <QtCore/QTime>
#include <QtCore/QDate>

#include <Plasma/Applet>
#include <Plasma/DataEngine>
#include <Plasma/Dialog>

#include "ui_clockConfig.h"
#include <plasmaclock/clockapplet.h>

namespace Plasma
{
    class Svg;
}

class Clock : public ClockApplet
{
    Q_OBJECT
    public:
        Clock(QObject *parent, const QVariantList &args);
        ~Clock();

        void init();
        void paintInterface(QPainter *painter, const QStyleOptionGraphicsItem *option, const QRect &contentsRect);

    public slots:
        void dataUpdated(const QString &name, const Plasma::DataEngine::Data &data);
        void updateColors();

    protected slots:
        void clockConfigAccepted();
        void clockConfigChanged();
        void constraintsEvent(Plasma::Constraints constraints);
        void resetSize();
        void updateClock(int category);

    protected:
        void createClockConfigurationInterface(KConfigDialog *parent);
        void changeEngineTimezone(const QString &oldTimezone, const QString &newTimezone);

    private slots:
        void configDrawShadowToggled(bool value);
        void launchDateKcm();

    private:
        void updateSize();
        bool showTimezone() const;
        void generatePixmap();
        QRect preparePainter(QPainter *p, const QRect &rect, const QFont &font, const QString &text, bool singleline = false);
        void prepareFont(QFont &font, QRect &rect, const QString &text, bool singleline);
        void expandFontToMax(QFont &font, const QString &text);
        QRectF normalLayout (int subtitleWidth, int subtitleHeight, const QRect &contentsRect);
        QRectF sideBySideLayout (int subtitleWidth, int subtitleHeight, const QRect &contentsRect);

        QFont m_plainClockFont;
        bool m_isDefaultFont;
        bool m_useCustomColor;
        QColor m_plainClockColor;
        bool m_useCustomShadowColor;
        QColor m_plainClockShadowColor;
        bool m_drawShadow;
        QRect m_timeRect;
        QRect m_dateRect;

        int m_dateStyle; //0 = don't show a date
        bool m_showSeconds;
        bool m_showTimezone;
        bool m_dateTimezoneBesides;

        int updateInterval() const;
        Plasma::IntervalAlignment intervalAlignment() const;

        QTime m_time;
        QDate m_date;
        QString m_dateString;
        QVBoxLayout *m_layout;
        QPixmap m_toolTipIcon;
        /// Designer Config files
        Ui::clockConfig ui;
        Plasma::Svg *m_svg;
        bool m_svgExistsInTheme;
        QPixmap m_pixmap;
};

K_EXPORT_PLASMA_APPLET(dig_clock, Clock)

#endif
