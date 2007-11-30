/***************************************************************************
 *   Copyright (C) 2007 by Alexis Ménard <darktears31@gmail.com>           *
 *   Copyright (C) 2006 Aaron Seigo <aseigo@kde.org>			   *
 *    									   *
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

#ifndef NOTIFIERWIDGET_H
#define NOTIFIERWIDGET_H

#include <QWidget>

namespace Plasma
{
    class Svg;
}

namespace Notifier
{

class NotifierWidget : public QWidget
{
    Q_OBJECT

    public:
        explicit NotifierWidget( QWidget * parent = 0,Qt::WindowFlags f =  Qt::Window );
        virtual ~NotifierWidget();

    protected:
        void paintEvent( QPaintEvent *e );

    private:
        void paintBackground(QPainter* painter, const QRect &exposedRect);
        Plasma::Svg *m_background;
        QPixmap *m_cachedBackground;
};

}
#endif
