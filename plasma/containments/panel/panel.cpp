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
#include <plasma/layouts/layout.h>
#include <plasma/svg.h>

using namespace Plasma;

Panel::Panel(QObject *parent, const QVariantList &args)
    : Containment(parent, args),
      m_cachedBackground(0)
{
    m_background = new Plasma::Svg("widgets/panel-background", this);
    setZValue(150);
    setContainmentType(Containment::PanelContainment);
}

Panel::~Panel()
{
    delete m_background;
}

void Panel::constraintsUpdated(Plasma::Constraints constraints)
{
    if (constraints & Plasma::LocationConstraint || constraints & Plasma::ScreenConstraint) {
        Plasma::Location loc = location();
        bool drawTop = true;
        bool drawLeft = true;
        bool drawRight = true;
        bool drawBottom = true;

        //FIXME: panels with less than 100% width need to paint edges, but full width panels
        //       shouldn't
        switch (loc) {
            case TopEdge:
                drawTop = false;
                break;
            case LeftEdge:
                drawLeft = false;
                break;
            case RightEdge:
                drawRight = false;
                break;
            case BottomEdge:
                drawBottom = false;
                break;
            default:
                break;
        }

        const int topHeight = drawTop ? m_background->elementSize("top").height() : 0;
        const int leftWidth = drawLeft ? m_background->elementSize("left").width() : 0;
        const int rightWidth = drawRight ? m_background->elementSize("right").width() : 0;
        const int bottomHeight = drawBottom ? m_background->elementSize("bottom").height() : 0;


    //kDebug() << "constraints updated with" << constraints << "!!!!!!!!!!!!!!!!!";
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
                height += topHeight - 1;
                y = r.height() - height + 1;
            } else {
                height += bottomHeight - 1;
            }
            //kDebug() << "top/bottom: Width:" << width << ", height:" << height;
        } else if (loc == LeftEdge || loc == RightEdge) {
            setFormFactor(Plasma::Vertical);

            width = 48;
            height = r.height();
            if (loc == RightEdge) {
                width += leftWidth - 1;
                x = r.width() - width;
            } else {
                width += rightWidth - 1;
            }
            //kDebug() << "left/right: Width:" << width << ", height:" << height;
        }
        //kDebug() << "Setting geometry to" << QRectF(x, y, width, height) << "with margins" << leftWidth << topHeight << rightWidth << bottomHeight;
        QRectF geo = QRectF(x, y, width, height);
        setGeometry(geo);

        if (layout()) {
            layout()->setMargin(Plasma::Layout::TopMargin, topHeight);
            layout()->setMargin(Plasma::Layout::LeftMargin, leftWidth);
            layout()->setMargin(Plasma::Layout::RightMargin, rightWidth);
            layout()->setMargin(Plasma::Layout::BottomMargin, bottomHeight);
        }

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
    painter->setRenderHint(QPainter::Antialiasing);

    paintBackground(painter, contentsRect);

    // restore transformation and composition mode
    painter->restore();
}

void Panel::paintBackground(QPainter* painter, const QRect& contentsRect)
{
    QSize s = geometry().toRect().size();
    m_background->resize();

    if (!m_cachedBackground || m_cachedBackground->size() != s) {
        bool drawTop = true;
        bool drawLeft = true;
        bool drawRight = true;
        bool drawBottom = true;

        //FIXME: panels with less than 100% width need to paint edges, but full width panels
        //       shouldn't
        switch (location()) {
            case TopEdge:
                drawTop = false;
                break;
            case LeftEdge:
                drawLeft = false;
                break;
            case RightEdge:
                drawRight = false;
                break;
            case BottomEdge:
                drawBottom = false;
                break;
            default:
                break;
        }

        const int topWidth = drawTop ? m_background->elementSize("top").width() : 0;
        const int topHeight = drawTop ? m_background->elementSize("top").height() : 0;
        const int leftWidth = drawLeft ? m_background->elementSize("left").width() : 0;
        const int leftHeight = drawLeft ? m_background->elementSize("left").height(): 0;
        const int rightWidth = drawRight ? m_background->elementSize("right").width() : 0;
        const int rightHeight = drawRight ? m_background->elementSize("right").height() : 0;
        const int bottomWidth = drawBottom ? m_background->elementSize("bottom").width() : 0;
        const int bottomHeight = drawBottom ? m_background->elementSize("bottom").height() : 0;

        const int topOffset = 0;
        const int leftOffset = 0;
        const int contentWidth = s.width() - leftWidth - rightWidth;
        const int contentHeight = s.height() - topHeight - bottomHeight;
        const int rightOffset = s.width() - rightWidth;
        const int bottomOffset = s.height() - bottomHeight;
        const int contentTop = topHeight;
        const int contentLeft = leftWidth;

        delete m_cachedBackground;
        m_cachedBackground = new QPixmap(s);

        m_cachedBackground->fill(Qt::transparent);
        QPainter p(m_cachedBackground);
        p.setCompositionMode(QPainter::CompositionMode_Source);
        p.setRenderHint(QPainter::SmoothPixmapTransform);

        //FIXME: This is a hack to fix a drawing problems with svg files where a thin transparent border is drawn around the svg image.
        //       the transparent border around the svg seems to vary in size depending on the size of the svg and as a result increasing the
        //       svn image by 2 all around didn't resolve the issue. For now it resizes based on the border size.

        if (contentWidth > 0 && contentHeight > 0) {
            m_background->resize(contentWidth, contentHeight);
            m_background->paint(&p, QRect(leftWidth, topHeight, contentWidth, contentHeight), "center");
            m_background->resize();
        }

        if (drawTop) {
            if (drawLeft) {
                m_background->paint(&p, QRect(leftOffset, topOffset, leftWidth, topHeight), "topleft");
            }

            if (drawRight) {
                m_background->paint(&p, QRect(rightOffset, topOffset, rightWidth, topHeight), "topright");
            }
        }

        if (drawBottom) {
            if (drawLeft) {
                m_background->paint(&p, QRect(leftOffset, bottomOffset, leftWidth, bottomHeight), "bottomleft");
            }

            if (drawRight) {
                m_background->paint(&p, QRect(rightOffset, bottomOffset, rightWidth, bottomHeight), "bottomright");
            }
        }

        if (m_background->elementExists("hint-stretch-borders")) {
            if (drawLeft) {
                m_background->paint(&p, QRect(leftOffset, contentTop, leftWidth, contentHeight), "left");
            }

            if (drawRight) {
                m_background->paint(&p, QRect(rightOffset, contentTop, rightWidth, contentHeight), "right");
            }

            if (drawTop) {
                m_background->paint(&p, QRect(contentLeft, topOffset, contentWidth, topHeight), "top");
            }

            if (drawBottom) {
                m_background->paint(&p, QRect(contentLeft, bottomOffset, contentWidth, bottomHeight), "bottom");
            }
        } else {
            if (drawLeft) {
                QPixmap left(leftWidth, leftHeight);
                left.fill(Qt::transparent);
                {
                    QPainter sidePainter(&left);
                    sidePainter.setCompositionMode(QPainter::CompositionMode_Source);
                    m_background->paint(&sidePainter, QPoint(0, 0), "left");
                }
                p.drawTiledPixmap(QRect(leftOffset, contentTop, leftWidth, contentHeight), left);
            }

            if (drawRight) {
                QPixmap right(rightWidth, rightHeight);
                right.fill(Qt::transparent);
                {
                    QPainter sidePainter(&right);
                    sidePainter.setCompositionMode(QPainter::CompositionMode_Source);
                    m_background->paint(&sidePainter, QPoint(0, 0), "right");
                }
                p.drawTiledPixmap(QRect(rightOffset, contentTop, rightWidth, contentHeight), right);
            }

            if (drawTop) {
                QPixmap top(topWidth, topHeight);
                top.fill(Qt::transparent);
                {
                    QPainter sidePainter(&top);
                    sidePainter.setCompositionMode(QPainter::CompositionMode_Source);
                    m_background->paint(&sidePainter, QPoint(0, 0), "top");
                }
                p.drawTiledPixmap(QRect(contentLeft, topOffset, contentWidth, topHeight), top);
            }

            if (drawBottom) {
                QPixmap bottom(bottomWidth, bottomHeight);
                bottom.fill(Qt::transparent);
                {
                    QPainter sidePainter(&bottom);
                    sidePainter.setCompositionMode(QPainter::CompositionMode_Source);
                    m_background->paint(&sidePainter, QPoint(0, 0), "bottom");
                }
                p.drawTiledPixmap(QRect(contentLeft, bottomOffset, contentWidth, bottomHeight), bottom);
            }
        }

        // re-enable this once Qt's svg rendering is un-buggered
        //background->resize(contentWidth, contentHeight);
        //background->paint(&p, QRect(contentLeft, contentTop, contentWidth, contentHeight), "center");
    }

    painter->drawPixmap(contentsRect, *m_cachedBackground, contentsRect);
}

K_EXPORT_PLASMA_APPLET(panel, Panel)

#include "panel.moc"

