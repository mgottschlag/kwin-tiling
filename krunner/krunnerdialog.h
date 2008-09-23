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

#include <KDialog>


namespace Plasma
{
    class RunnerManager;
    class PanelSvg;
}

class KRunnerConfigDialog;

class KRunnerDialog : public KDialog
{
    Q_OBJECT

    public:
        explicit KRunnerDialog(Plasma::RunnerManager *manager, QWidget *parent = 0,
                               Qt::WindowFlags f =  Qt::Dialog | Qt::FramelessWindowHint);
        virtual ~KRunnerDialog();

    protected:
        void paintEvent( QPaintEvent *e );
        void resizeEvent( QResizeEvent *e );
        void mousePressEvent( QMouseEvent *e );

    protected Q_SLOTS:
        void showConfigDialog();

    private Q_SLOTS:
        /**
         * React to theme changes 
         */
        void themeUpdated();

    private:
        void configCompleted();
        void paintBackground(QPainter* painter, const QRect &exposedRect);

        KRunnerConfigDialog *m_configDialog;
        Plasma::RunnerManager *m_runnerManager;
        Plasma::PanelSvg *m_background;
        QPixmap *m_cachedBackground;
};

#endif
