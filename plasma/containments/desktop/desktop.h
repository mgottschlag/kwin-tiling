/*
*   Copyright 2007 by Aaron Seigo <aseigo@kde.org>
*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU Library General Public License version 2,
*   or (at your option) any later version.
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

#ifndef PLASMA_DESKTOP_H
#define PLASMA_DESKTOP_H

#include <QGraphicsItem>
#include <QList>
#include <QObject>
#include <QStyleOptionGraphicsItem>

#include <KIcon>

#include <plasma/containment.h>
#include <plasma/phase.h>
#include <plasma/widgets/widget.h>

class QAction;
class QTimeLine;

namespace Plasma
{
    class AppletBrowser;
    class Svg;
}

/*class Tool : public QObject, public QGraphicsItem
{
    Q_OBJECT

public:
    explicit Tool(QGraphicsItem *parent = 0);
    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);

};*/

class DefaultDesktop : public Plasma::Containment
{
    Q_OBJECT

public:
    DefaultDesktop(QObject *parent, const QVariantList &args);
    ~DefaultDesktop();
    void init();
    void constraintsUpdated(Plasma::Constraints constraints);

    QList<QAction*> contextActions();

    /**
     * Paints a default background image. Nothing fancy, but that's what plugins
     * are for. Reimplemented from Plasma::Containment;
     */
    void paintInterface(QPainter *painter,
                        const QStyleOptionGraphicsItem *option,
                        const QRect& contentsRect);

protected Q_SLOTS:
    void runCommand();
    void lockScreen();
    void logout();

private:
    QAction *m_appletBrowserAction;
    QAction *m_runCommandAction;
    QAction *m_lockAction;
    QAction *m_logoutAction;
    Plasma::Svg *m_background;
    QPixmap* m_bitmapBackground;
    QString m_wallpaperPath;
};

#endif // PLASMA_PANEL_H
