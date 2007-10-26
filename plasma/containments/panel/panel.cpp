/*
*   Copyright 2007 by Alex Merry <huntedhacker@tiscali.co.uk>
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

#include "panel.h"

#include <QPainter>
#include <QDesktopWidget>

#include <KDebug>

#include <plasma/svg.h>

using namespace Plasma;

Panel::Panel(QObject *parent, const QVariantList &args)
    : Containment(parent, args)
{
    //FIXME: we need a proper background painting implementation here
    m_background = new Plasma::Svg("widgets/panel-background", this);
    setZValue(150);
}

Panel::~Panel()
{
    delete m_background;
}

Containment::Type Panel::type()
{
    return PanelContainment;
}

void Panel::constraintsUpdated(Plasma::Constraints constraints)
{
    kDebug() << "constraints updated with" << constraints << "!!!!!!!!!!!!!!!!!";
    if (constraints & Plasma::LocationConstraint ||
        constraints & Plasma::ScreenConstraint) {
        Plasma::Location loc = location();
        int s = screen();
        if (s < 0) {
            s = 0;
        }

        kDebug() << "Setting location to" << loc << "on screen" << s;

        QDesktopWidget desktop;
        QRect r = desktop.screenGeometry(s);
        setMaximumSize(r.size());
        int x = 0;
        int y = 0;
        int width = 0;
        int height = 0;
        if (loc == BottomEdge || loc == TopEdge) {
            setFormFactor(Plasma::Horizontal);

            width = r.width();
            height = 48;
            kDebug() << "Width:" << width << ", height:" << height;
            if (loc == BottomEdge) {
                y = r.height() - height;
            }
        } else if (loc == LeftEdge || loc == RightEdge) {
            setFormFactor(Plasma::Vertical);

            width = 48;
            height = r.height();
            if (loc == RightEdge) {
                x = r.width() - width;
            }
        }
        kDebug() << "Setting geometry to" << QRectF(x, y, width, height);
        setGeometry(QRectF(x, y, width, height));
    }
}

Qt::Orientations Panel::expandingDirections() const
{
    if (formFactor() == Plasma::Horizontal) {
        return Qt::Horizontal;
    } else {
        return Qt::Vertical;
    }
}

void Panel::paintInterface(QPainter *painter,
                           const QStyleOptionGraphicsItem *,
                           const QRect& contentsRect)
{
    // draw the background untransformed (saves lots of per-pixel-math)
    painter->save();
    painter->resetTransform();

    // blit the background (saves all the per-pixel-products that blending does)
    painter->setCompositionMode(QPainter::CompositionMode_Source);

    if (m_background) {
        m_background->resize(contentsRect.size());
        // Plasma::Svg doesn't support drawing only part of the image (it only
        // supports drawing the whole image to a rect), so we blit to 0,0-w,h
        m_background->paint(painter, 0, 0);
    }

    // restore transformation and composition mode
    painter->restore();
}

K_EXPORT_PLASMA_APPLET(panel, Panel)

#include "panel.moc"

