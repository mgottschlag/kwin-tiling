/*
 *   Copyright (C) 2010 Ivan Cukic <ivan.cukic(at)kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License version 2,
 *   or (at your option) any later version, as published by the Free
 *   Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "SplashWindow.h"

#include <QApplication>
#include <QDeclarativeContext>
#include <QGraphicsObject>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QTimer>

#include "SystemInfo.h"

SplashWindow::SplashWindow(bool testing)
    : QDeclarativeView(),
      m_stage(0),
      m_testing(testing)
{
    setWindowFlags(
            Qt::FramelessWindowHint |
            Qt::WindowStaysOnTopHint
        );

    if (m_testing) {
        setWindowState(Qt::WindowFullScreen);
    } else {
        setWindowFlags(Qt::X11BypassWindowManagerHint);
    }

    rootContext()->setContextProperty("screenSize", size());
    setSource(QUrl(themeDir(QApplication::arguments().at(1)) + "/main.qml"));
    setStyleSheet("background: #000000; border: none");
    //be sure it will be eventually closed
    //FIXME: should never be stuck
    QTimer::singleShot(30000, this, SLOT(close()));
}

void SplashWindow::setStage(int stage)
{
    m_stage = stage;

    rootObject()->setProperty("stage", stage);
}

void SplashWindow::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event)
    rootContext()->setContextProperty("screenSize", size());
    centerOn(rootObject());
}

void SplashWindow::keyPressEvent(QKeyEvent *event)
{
    QDeclarativeView::keyPressEvent(event);
    if (m_testing && !event->isAccepted() && event->key() == Qt::Key_Escape) {
        close();
    }
}

void SplashWindow::mousePressEvent(QMouseEvent *event)
{
    QDeclarativeView::mousePressEvent(event);
    if (m_testing && !event->isAccepted()) {
        close();
    }
}
