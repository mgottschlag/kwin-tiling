/*
*   Copyright 2007 by Aaron Seigo <aseigo@kde.org>
*   Copyright 2008 by Chani Armitage <chanika@gmail.com>
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

#include "desktop.h"

#include <QAction>
//#include <QApplication>
//#include <QDesktopWidget>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QPainter>

//#include <KAuthorized>
#include <KDebug>
//#include <KWindowSystem>
//#include <KActionCollection>

//#include "plasma/corona.h"
#include "plasma/theme.h"
//#include "kworkspace/kworkspace.h"
//#include "knewstuff2/engine.h"

//TODO offer a way to change the theme and other such not-really-our-responsibility things

using namespace Plasma;

SaverDesktop::SaverDesktop(QObject *parent, const QVariantList &args)
    : Containment(parent, args),
      m_lockDesktopAction(0),
      m_appletBrowserAction(0)
{
    //kDebug() << "!!! loading desktop";
}

SaverDesktop::~SaverDesktop()
{
}

void SaverDesktop::init()
{
    Containment::init();
    //setHasConfigurationInterface(true);

    //remove the desktop actions
    QAction *unwanted = action("zoom in");
    removeToolBoxTool(unwanted);
    delete unwanted;
    unwanted = action("zoom out");
    removeToolBoxTool(unwanted);
    delete unwanted;
    unwanted = action("addSiblingContainment");
    removeToolBoxTool(unwanted);
    delete unwanted;
}

QList<QAction*> SaverDesktop::contextualActions()
{
    if (!m_appletBrowserAction) {
        m_appletBrowserAction = action("add widgets");
        m_lockDesktopAction = action("lock widgets");
    }

    QList<QAction*> actions;
    actions.append(m_appletBrowserAction);
    actions.append(m_lockDesktopAction);

    return actions;
}

void SaverDesktop::paintInterface(QPainter *painter,
                                    const QStyleOptionGraphicsItem *option,
                                    const QRect& contentsRect)
{
    //kDebug() << "paintInterface of background";

    painter->save();

    if (painter->worldMatrix() == QMatrix()) {
        // draw the background untransformed when possible;(saves lots of per-pixel-math)
        painter->resetTransform();
    }

    // blit the background (saves all the per-pixel-products that blending does)
    painter->setCompositionMode(QPainter::CompositionMode_Source);

    QColor color = Plasma::Theme::defaultTheme()->color(Plasma::Theme::BackgroundColor);
    //FIXME should I use the contents rect or the exposed rect?
    painter->fillRect(contentsRect, color);


    // for pixmaps we draw only the exposed part (untransformed since the
    // bitmapBackground already has the size of the viewport)
    //painter->drawPixmap(option->exposedRect, m_bitmapBackground, option->exposedRect);
    //kDebug() << "draw pixmap of background to" << option->exposedRect;

    // restore transformation and composition mode
    painter->restore();
}

K_EXPORT_PLASMA_APPLET(saverdesktop, SaverDesktop)

#include "desktop.moc"
