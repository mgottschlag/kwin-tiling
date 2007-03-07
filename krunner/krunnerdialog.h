/*
 *   Copyright (C) 2006 Aaron Seigo <aseigo@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2 as
 *   published by the Free Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef KRUNNERDIALOG_H
#define KRUNNERDIALOG_H

#include <QDialog>

class QSvgRenderer;

namespace Plasma
{
    class Svg;
}

class KRunnerDialog : public QDialog
{
    Q_OBJECT

    public:
        explicit KRunnerDialog( QWidget * parent = 0,
                                Qt::WindowFlags f =  Qt::Dialog | Qt::FramelessWindowHint );
        virtual ~KRunnerDialog();

    protected:
        void paintEvent( QPaintEvent *e );
        void resizeEvent( QResizeEvent *e );

    private:
        Plasma::Svg* m_background;
};

#endif
