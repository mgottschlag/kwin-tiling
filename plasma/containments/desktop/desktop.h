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

#include <KIcon>

#include <plasma/containment.h>
#include <plasma/widgets/widget.h>

class QAction;
class QTimeLine;

namespace Plasma
{
    class AppletBrowser;
}

/*class Tool : public QObject, public QGraphicsItem
{
    Q_OBJECT

public:
    explicit Tool(QGraphicsItem *parent = 0);
    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);

};*/

class ToolBox : public Plasma::Widget
{
    Q_OBJECT

public:
    explicit ToolBox(QGraphicsItem *parent = 0);
    QRectF boundingRect() const;
    QPainterPath shape() const;

protected:
    void paintWidget(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);

protected slots:
    void animate(int frame);
    void toolMoved(QGraphicsItem*);

private:
    QList<Plasma::Widget*> m_tools;
    QTimeLine *m_showTimeLine;
    KIcon m_icon;
    int m_size;
    bool m_hidden;
};

class DefaultDesktop : public Plasma::Containment
{
    Q_OBJECT

public:
    DefaultDesktop(QObject *parent, const QVariantList &args);
    ~DefaultDesktop();
    void init();
    void constraintsUpdated(Plasma::Constraints constraints);

    QList<QAction*> contextActions();

protected Q_SLOTS:
    void launchExplorer();
    void launchAppletBrowser();
    void runCommand();
    void lockScreen();
    void logout();

private:
    QAction *m_engineExplorerAction;
    QAction *m_appletBrowserAction;
    QAction *m_runCommandAction;
    QAction *m_lockAction;
    QAction *m_logoutAction;
    ToolBox *m_toolbox;
    Plasma::AppletBrowser *m_appletBrowser;
};

#endif // PLASMA_PANEL_H
