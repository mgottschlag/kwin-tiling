/***************************************************************************
 *   Copyright 2009 by Alessandro Diaferia <alediaferia@gmail.com>         *
 *   Copyright 2009 by Marco Martin <notmart@gmail.com>                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/
#include "itembackground.h"

#include <QPainter>
#include <QStyleOptionGraphicsItem>

#include <KDebug>

#include <Plasma/FrameSvg>
#include <Plasma/Animator>
#include <Plasma/Theme>

ItemBackground::ItemBackground(QGraphicsWidget *parent)
    : QGraphicsWidget(parent),
      m_frameSvg(new Plasma::FrameSvg(this)),
      m_animId(0),
      m_opacity(1),
      m_fading(false),
      m_fadeIn(false)
{
    setContentsMargins(0, 0, 0, 0);

    m_frameSvg->setImagePath("widgets/viewitem");
    m_frameSvg->setEnabledBorders(Plasma::FrameSvg::AllBorders);
    m_frameSvg->setCacheAllRenderedFrames(true);
    m_frameSvg->setElementPrefix("hover");

    setAcceptedMouseButtons(0);

}

ItemBackground::~ItemBackground()
{}

void ItemBackground::animatedShowAtRect(const QRectF &newGeometry)
{
    qreal left, top, right, bottom;
    m_frameSvg->getMargins(left, top, right, bottom);

    m_oldGeometry = geometry();
    m_newGeometry = newGeometry.adjusted(-left, -top, right, bottom);

    if (m_animId != 0) {
        Plasma::Animator::self()->stopCustomAnimation(m_animId);
    }

    if (isVisible()) {
        m_fading = false;
        m_opacity = 1;
        m_animId = Plasma::Animator::self()->customAnimation(
            15, 250, Plasma::Animator::EaseInOutCurve, this, "animationUpdate");
    } else {
        setGeometry(m_newGeometry);
        animatedSetVisible(true);
    }
}

void ItemBackground::animatedSetVisible(bool visible)
{
    m_fading = true;
    m_fadeIn = visible;
    if (visible) {
        setVisible(true);
    }

    if (m_animId != 0) {
        Plasma::Animator::self()->stopCustomAnimation(m_animId);
    }

    m_animId = Plasma::Animator::self()->customAnimation(
               10, 250, Plasma::Animator::EaseInCurve, this, "animationUpdate");
}

void ItemBackground::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(widget)

    if (m_frameSvg->frameSize() != option->rect.size()) {
        m_frameSvg->resizeFrame(option->rect.size());
    }

    if (qFuzzyCompare(m_opacity, (qreal)1.0)) {
        m_frameSvg->paintFrame(painter, option->rect.topLeft());
    } else if (qFuzzyCompare(m_opacity+1, (qreal)1.0)) {
        return;
    } else {
        QPixmap framePix = m_frameSvg->framePixmap();
        QPainter bufferPainter(&framePix);
        bufferPainter.setCompositionMode(QPainter::CompositionMode_DestinationIn);
        bufferPainter.fillRect(framePix.rect(), QColor(0, 0, 0, 255 * m_opacity));
        bufferPainter.end();
        painter->drawPixmap(framePix.rect(), framePix, framePix.rect());
    }
}

void ItemBackground::animationUpdate(qreal progress)
{
    if (progress == 1) {
        m_animId = 0;
    }

    if (m_fading) {
        m_opacity = m_fadeIn?progress:1-progress;
        if (!m_fadeIn && qFuzzyCompare(m_opacity+1, (qreal)1.0)) {
            hide();
        }
    } else {
        setGeometry(m_oldGeometry.x() + (m_newGeometry.x() - m_oldGeometry.x()) * progress,
                    m_oldGeometry.y() + (m_newGeometry.y() - m_oldGeometry.y()) * progress,

                    m_oldGeometry.width() + (m_newGeometry.width() - m_oldGeometry.width()) * progress,
                    m_oldGeometry.height() + (m_newGeometry.height() - m_oldGeometry.height()) * progress);
    }
    update();
}
