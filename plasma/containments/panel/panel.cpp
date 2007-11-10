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

#include <QApplication>
#include <QPainter>
#include <QDesktopWidget>

#include <KDebug>

#include <plasma/corona.h>
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
    //kDebug() << "constraints updated with" << constraints << "!!!!!!!!!!!!!!!!!";
    if (constraints & Plasma::LocationConstraint ||
        constraints & Plasma::ScreenConstraint) {
        Plasma::Location loc = location();
        int s = screen();
        if (s < 0) {
            s = 0;
        }


        QRect r = QApplication::desktop()->screenGeometry(s);
        //kDebug() << "Setting location to" << loc << "on screen" << s << "with geom" << r;
        setMaximumSize(r.size());
        int x = 0;
        int y = 0;
        int width = 0;
        int height = 0;
        if (loc == BottomEdge || loc == TopEdge) {
            setFormFactor(Plasma::Horizontal);

            width = r.width();
            height = 48;
            if (loc == BottomEdge) {
                y = r.height() - height;
            }
            //kDebug() << "top/bottom: Width:" << width << ", height:" << height;
        } else if (loc == LeftEdge || loc == RightEdge) {
            setFormFactor(Plasma::Vertical);

            width = 48;
            height = r.height();
            if (loc == RightEdge) {
                x = r.width() - width;
            }
            //kDebug() << "left/right: Width:" << width << ", height:" << height;
        }
        //kDebug() << "Setting geometry to" << QRectF(x, y, width, height);
        QRectF geo = QRectF(x, y, width, height);
        setGeometry(geo);

        if (corona()) {
            foreach (Containment *c, corona()->containments()) {
                if (c->type() != PanelContainment || c == this) {
                    continue;
                }

                if (c->geometry().intersects(geo)) {
                    //TODO: here is where we need to schedule a negotiation for where to show the
                    //      panel on the scene
                    //
                    //      we also probably need to direct whether to allow this containment to
                    //      be resized before moved, or moved only
                    kDebug() << "conflict!";
                }
                kDebug() << "panel containment with geometry of" << c->geometry() << "but really" << c->transform().map(geometry());
            }
        }
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
    //FIXME: this background drawing is bad and ugly =)
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

