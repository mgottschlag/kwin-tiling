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
    class FrameSvg;
}

class KRunnerConfigDialog;

class KRunnerDialog : public KDialog
{
    Q_OBJECT

    public:
        explicit KRunnerDialog(Plasma::RunnerManager *manager, QWidget *parent = 0,
                               Qt::WindowFlags f =  Qt::Dialog | Qt::FramelessWindowHint);
        virtual ~KRunnerDialog();

    public Q_SLOTS:
        virtual void display(const QString& term = QString()) = 0;
        virtual void switchUser() = 0;
        virtual void clearHistory() = 0;

    protected:
        void paintEvent(QPaintEvent *event);
        void resizeEvent(QResizeEvent *event);
        void mousePressEvent(QMouseEvent *event);
        bool event(QEvent *event);

    protected Q_SLOTS:
        void showConfigDialog();

    private Q_SLOTS:
        /**
         * React to theme changes
         */
        void themeUpdated();

        /**
         * React to configuration changes
         */
        void configCompleted();

    private:
        void paintBackground(QPainter* painter, const QRect &exposedRect);

        KRunnerConfigDialog *m_configDialog;
        Plasma::RunnerManager *m_runnerManager;
        Plasma::FrameSvg *m_background;
        QPixmap *m_cachedBackground;
};

#endif
