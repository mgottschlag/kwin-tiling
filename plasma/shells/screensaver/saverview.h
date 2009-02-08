/*
 *   Copyright 2007 Aaron Seigo <aseigo@kde.org>
 *   Copyright 2007 André Duffeck <duffeck@kde.org>
 *   Copyright 2008 Chani Armitage <chanika@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
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

#ifndef SAVERVIEW_H
#define SAVERVIEW_H

#include <Plasma/Plasma>
#include <Plasma/View>

namespace Plasma
{
    class AppletBrowser;
    class Containment;
}

class SaverView : public Plasma::View
{
    Q_OBJECT

public:
    SaverView(Plasma::Containment* containment, QWidget *parent);
    ~SaverView();

signals:
    void hidden();

protected:
    void drawBackground(QPainter * painter, const QRectF & rect);
    void keyPressEvent(QKeyEvent *event);
    void showEvent(QShowEvent *event);
    void paintEvent(QPaintEvent *event);
    bool eventFilter(QObject *watched, QEvent *event);

public slots:
    void showView();
    void hideView();

    /**
     * Sets the containment for this view, which will also cause the view
     * to track the geometry of the containment.
     *
     * @arg containment the containment to center the view on
     */
    void setContainment(Plasma::Containment *newContainment);
    void hideAppletBrowser();

    void enableSetupMode();
    void disableSetupMode();

protected slots:
    void showAppletBrowser(); //FIXME actually this is toggle
    void appletBrowserDestroyed();
    void suppressShowTimeout();

private:
    Plasma::AppletBrowser *m_appletBrowser;
    QPoint m_appletBrowserDragStart;
    bool m_suppressShow : 1;
    bool m_setupMode : 1;
    bool m_init : 1;
};

#endif // multiple inclusion guard
