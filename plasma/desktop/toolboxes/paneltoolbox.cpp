/*
 *   Copyright 2007 by Aaron Seigo <aseigo@kde.org>
 *   Copyright 2008 by Marco Martin <notmart@gmail.com>
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

#include "paneltoolbox.h"

#include <QGraphicsSceneHoverEvent>
#include <QPainter>
#include <QRadialGradient>
#include <QApplication>
#include <QWeakPointer>

#include <KDebug>
#include <KIconLoader>

#include <plasma/applet.h>
#include <plasma/paintutils.h>
#include <plasma/svg.h>
#include <plasma/theme.h>
#include <plasma/tooltipcontent.h>
#include <plasma/tooltipmanager.h>

PanelToolBox::PanelToolBox(Plasma::Containment *parent)
    : InternalToolBox(parent)
{
    init();
}

PanelToolBox::PanelToolBox(QObject *parent, const QVariantList &args)
   : InternalToolBox(parent, args)
{
    init();
}

PanelToolBox::~PanelToolBox()
{
    m_anim.clear();
}

void PanelToolBox::init()
{
    m_icon = KIcon("plasma");
    m_animFrame = 0;
    m_highlighting = false;

    setIconSize(QSize(KIconLoader::SizeSmall, KIconLoader::SizeSmall));
    setSize(KIconLoader::SizeSmallMedium);
    connect(this, SIGNAL(toggled()), this, SLOT(toggle()));

    setZValue(10000000);
    setFlag(ItemClipsChildrenToShape, false);
    //panel toolbox is allowed to zoom, otherwise a part of it will be displayed behind the desktop
    //toolbox when the desktop is zoomed out
    setFlag(ItemIgnoresTransformations, false);
    assignColors();
    connect(Plasma::Theme::defaultTheme(), SIGNAL(themeChanged()),
            this, SLOT(assignColors()));

    m_background = new Plasma::Svg(this);
    m_background->setImagePath("widgets/toolbox");
    m_background->setContainsMultipleImages(true);

    Plasma::ToolTipManager::self()->registerWidget(this);

    if (containment()) {
        QObject::connect(containment(), SIGNAL(immutabilityChanged(Plasma::ImmutabilityType)),
                         this, SLOT(immutabilityChanged(Plasma::ImmutabilityType)));
    }
}

void PanelToolBox::assignColors()
{
    m_bgColor = Plasma::Theme::defaultTheme()->color(Plasma::Theme::BackgroundColor);
    m_fgColor = Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor);
    update();
}

QRectF PanelToolBox::boundingRect() const
{
    QRectF r;

    //Only Left,Right and Bottom supported, default to Right
    if (corner() == InternalToolBox::Bottom) {
        r = QRectF(0, 0, size() * 2, size());
    } else if (corner() == InternalToolBox::Left) {
        r = QRectF(0, 0, size(), size() * 2);
    } else {
        r = QRectF(0, 0, size(), size() * 2);
    }

    if (parentItem()) {
        QSizeF s = parentItem()->boundingRect().size();

        if (r.height() > s.height()) {
            r.setHeight(s.height());
        }

        if (r.width() > s.width()) {
            r.setWidth(s.width());
        }
    }

    return r;
}

void PanelToolBox::immutabilityChanged(Plasma::ImmutabilityType immutability)
{
    setVisible(immutability == Plasma::Mutable);
}

void PanelToolBox::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

    const qreal progress = m_animFrame / size();

    QRect backgroundRect;
    QPoint gradientCenter;
    QRectF rect = boundingRect();
    QString cornerElement;

    if (corner() == InternalToolBox::Bottom) {
        gradientCenter = QPoint(rect.center().x(), rect.bottom());
        cornerElement = "panel-south";

        backgroundRect = m_background->elementRect(cornerElement).toRect();
        backgroundRect.moveBottomLeft(shape().boundingRect().bottomLeft().toPoint());
    } else if (corner() == InternalToolBox::Right) {
        gradientCenter = QPoint(rect.right(), rect.center().y());
        cornerElement = "panel-east";

        backgroundRect = m_background->elementRect(cornerElement).toRect();
        backgroundRect.moveTopRight(shape().boundingRect().topRight().toPoint());
    } else {
        gradientCenter = QPoint(rect.right(), rect.center().y());
        cornerElement = "panel-west";

        backgroundRect = m_background->elementRect(cornerElement).toRect();
        backgroundRect.moveTopLeft(shape().boundingRect().topLeft().toPoint());
    }

    m_background->paint(painter, backgroundRect, cornerElement);

    //Only Left,Right and Bottom supported, default to Right
    QRect iconRect;
    if (corner() == InternalToolBox::Bottom) {
        iconRect = QRect(QPoint(gradientCenter.x() - iconSize().width() / 2,
                                (int)rect.bottom() - iconSize().height() - 2), iconSize());
    } else if (corner() == InternalToolBox::Left) {
        iconRect = QRect(QPoint(2, gradientCenter.y() - iconSize().height() / 2), iconSize());
    } else {
        iconRect = QRect(QPoint((int)rect.right() - iconSize().width() + 1,
                                gradientCenter.y() - iconSize().height() / 2), iconSize());
    }

    if (qFuzzyCompare(qreal(1.0), progress)) {
        m_icon.paint(painter, iconRect);
    } else if (qFuzzyCompare(qreal(1.0), 1 + progress)) {
        m_icon.paint(painter, iconRect, Qt::AlignCenter, QIcon::Disabled, QIcon::Off);
    } else {
        QPixmap disabled = m_icon.pixmap(iconSize(), QIcon::Disabled, QIcon::Off);
        QPixmap enabled = m_icon.pixmap(iconSize());
        QPixmap result = Plasma::PaintUtils::transition(
            m_icon.pixmap(iconSize(), QIcon::Disabled, QIcon::Off),
            m_icon.pixmap(iconSize()), progress);
        painter->drawPixmap(iconRect, result);
    }
}

QPainterPath PanelToolBox::shape() const
{
    QPainterPath path;
    int toolSize = size();// + (int)m_animFrame;
    QRectF rect = boundingRect();

    //Only Left,Right and Bottom supported, default to Right
    if (corner() == InternalToolBox::Bottom) {
        path.moveTo(rect.bottomLeft());
        path.arcTo(QRectF(rect.center().x() - toolSize,
                          rect.bottom() - toolSize,
                          toolSize * 2,
                          toolSize * 2), 0, 180);
    } else if (corner() == InternalToolBox::Left) {
        path.arcTo(QRectF(rect.left(),
                          rect.center().y() - toolSize,
                          toolSize * 2,
                          toolSize * 2), 90, -180);
    } else {
        path.moveTo(rect.topRight());
        path.arcTo(QRectF(rect.left(),
                          rect.center().y() - toolSize,
                          toolSize * 2,
                          toolSize * 2), 90, 180);
    }

    return path;
}

void PanelToolBox::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    highlight(true);
    QGraphicsItem::hoverEnterEvent(event);
}

void PanelToolBox::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    //kDebug() << event->pos() << event->scenePos()
    if (!isShowing()) {
        highlight(false);
    }

    QGraphicsItem::hoverLeaveEvent(event);
}

void PanelToolBox::showToolBox()
{
}

void PanelToolBox::hideToolBox()
{
}

void PanelToolBox::setShowing(bool show)
{
    InternalToolBox::setShowing(show);
    highlight(show);
}

void PanelToolBox::toolTipAboutToShow()
{
    if (isShowing()) {
        return;
    }

    Plasma::ToolTipContent c(i18n("Panel Tool Box"),
                     i18n("Click to access size, location and hiding controls as well as to add "
                          "new widgets to the panel."),
                     KIcon("plasma"));
    c.setAutohide(false);
    Plasma::ToolTipManager::self()->setContent(this, c);
}

void PanelToolBox::toolTipHidden()
{
    Plasma::ToolTipManager::self()->clearContent(this);
}

void PanelToolBox::highlight(bool highlighting)
{
    if (m_highlighting == highlighting) {
        return;
    }

    m_highlighting = highlighting;

    QPropertyAnimation *anim = m_anim.data();
    if (m_highlighting) {
        if (anim) {
            anim->stop();
            m_anim.clear();
        }
        anim = new QPropertyAnimation(this, "highlight", this);
        m_anim = anim;
    }

    if (anim->state() != QAbstractAnimation::Stopped) {
        anim->stop();
    }

    anim->setDuration(240);
    anim->setStartValue(0);
    anim->setEndValue(size());

    if (m_highlighting) {
        anim->start();
    } else {
        anim->setDirection(QAbstractAnimation::Backward);
        anim->start(QAbstractAnimation::DeleteWhenStopped);

    }
}

void PanelToolBox::setHighlightValue(qreal progress)
{
    m_animFrame = progress;
    update();
}

qreal PanelToolBox::highlightValue() const
{
    return m_animFrame;
}

void PanelToolBox::toggle()
{
    setShowing(!isShowing());
}


#include "paneltoolbox.moc"

