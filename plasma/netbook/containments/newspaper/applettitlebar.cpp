/*
 *   Copyright 2009 Marco Martin <notmart@gmail.com>
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

#include "applettitlebar.h"
#include "newspaper.h"

#include <QGraphicsGridLayout>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSceneResizeEvent>
#include <QPainter>
#include <QParallelAnimationGroup>
#include <QTimer>


#include <KIconLoader>

#include <plasma/animations/animation.h>
#include <plasma/applet.h>
#include <plasma/svg.h>
#include <plasma/theme.h>

AppletTitleBar::AppletTitleBar(Plasma::Applet *applet)
       : QGraphicsWidget(applet),
         m_applet(applet),
         m_pressedButton(NoButton),
         m_separator(0),
         m_background(0),
         m_savedAppletTopMargin(0),
         m_underMouse(false),
         m_buttonsVisible(false),
         m_appletHasBackground(false),
         m_forcedButtonsVisible(false)
{
    setObjectName("TitleBar");
    m_pulse =
    Plasma::Animator::create(Plasma::Animator::PulseAnimation);
    m_pulse->setTargetWidget(applet);

    m_maximizeButtonRect = m_configureButtonRect = m_closeButtonRect = QRect(0, 0, KIconLoader::SizeSmallMedium, KIconLoader::SizeSmallMedium);

    m_icons = new Plasma::Svg(this);
    m_icons->setImagePath("widgets/configuration-icons");
    m_icons->setContainsMultipleImages(true);

    if (applet->backgroundHints() != Plasma::Applet::NoBackground) {
        m_appletHasBackground = true;
    }

    if (applet->backgroundHints() & Plasma::Applet::StandardBackground ||
        applet->backgroundHints() & Plasma::Applet::TranslucentBackground) {
        m_separator = new Plasma::Svg(this);
        m_separator->setImagePath("widgets/line");
        m_separator->setContainsMultipleImages(true);
    } else {
        m_background = new Plasma::FrameSvg(this);
        m_background->setImagePath("widgets/background");
    }

    applet->installEventFilter(this);
    syncMargins();
    syncSize();

    if (applet->containment()) {
        connect(applet->containment(), SIGNAL(appletRemoved(Plasma::Applet *)), this, SLOT(appletRemoved(Plasma::Applet *)));
    }
    connect(Plasma::Theme::defaultTheme(), SIGNAL(themeChanged()), this, SLOT(themeChanged()));
}

AppletTitleBar::~AppletTitleBar()
{
    delete m_pulse;
    delete m_animations.data();
}

void AppletTitleBar::setButtonsVisible(bool visible)
{
    if (visible == m_buttonsVisible) {
        return;
    }

    m_buttonsVisible = visible;

    if (visible) {
        if (!m_animations) {
            initAnimations();

            m_animations.data()->start();
            m_animations.data()->setCurrentTime(0);
        } else {
            QParallelAnimationGroup *group = m_animations.data();

            group->stop();
            group->setCurrentTime(0);
            group->setDirection(QAbstractAnimation::Forward);

            group->start();
        }
    } else {
        initAnimations();
        QParallelAnimationGroup *group = m_animations.data();
        group->setDirection(QAbstractAnimation::Backward);
        group->start(QAbstractAnimation::DeleteWhenStopped);
    }
}

bool AppletTitleBar::buttonsVisible() const
{
    return m_buttonsVisible;
}

void AppletTitleBar::setForcedButtonsVisible(bool visible)
{
    if (visible == m_forcedButtonsVisible) {
        return;
    }

    setButtonsVisible(visible);
    m_forcedButtonsVisible = visible;
}

bool AppletTitleBar::forcedButtonsVisible() const
{
    return m_forcedButtonsVisible;
}

void AppletTitleBar::initAnimations()
{
    if (m_animations) {
        return;
    }

    m_animations = new QParallelAnimationGroup(this);
    QParallelAnimationGroup *group = m_animations.data();

    if (m_applet->hasValidAssociatedApplication()) {
        Plasma::Animation *maximizeAnim =
        Plasma::Animator::create(Plasma::Animator::PixmapTransitionAnimation);
        maximizeAnim->setProperty("targetPixmap", m_icons->pixmap("maximize"));
        maximizeAnim->setTargetWidget(this);
        group->addAnimation(maximizeAnim);
    }

    Plasma::Animation *confAnim =
        Plasma::Animator::create(Plasma::Animator::PixmapTransitionAnimation);
    Plasma::Animation *closeAnim =
        Plasma::Animator::create(Plasma::Animator::PixmapTransitionAnimation);
    confAnim->setProperty("targetPixmap", m_icons->pixmap("configure"));
    confAnim->setTargetWidget(this);

    closeAnim->setProperty("targetPixmap", m_icons->pixmap("close"));
    closeAnim->setTargetWidget(this);
    group->addAnimation(confAnim);
    group->addAnimation(closeAnim);
}

void AppletTitleBar::syncMargins()
{
    const int extraMargin = 2;
    syncIconRects();

    if (m_background) {
        qreal left, top, right, bottom;

        m_background->getMargins(left, top, right, bottom);
        setContentsMargins(left, top, right, bottom);
        setMaximumHeight(INT_MAX);
        setMinimumHeight(m_maximizeButtonRect.height() + extraMargin + top + bottom);
        setMaximumHeight(m_maximizeButtonRect.height() + extraMargin + top + bottom);
    } else {
        setContentsMargins(0, 0, 0, 0);
        setMaximumHeight(INT_MAX);
        setMinimumHeight(m_maximizeButtonRect.height() + extraMargin);
        setMaximumHeight(m_maximizeButtonRect.height() + extraMargin);
    }


    qreal left, right, bottom;
    m_applet->getContentsMargins(&left, &m_savedAppletTopMargin, &right, &bottom);
    m_applet->setContentsMargins(left, m_savedAppletTopMargin + size().height() + extraMargin, right, bottom);
}

void AppletTitleBar::syncSize()
{
    setGeometry(QRectF(QPointF(m_applet->contentsRect().left(), m_savedAppletTopMargin),
                QSizeF(m_applet->contentsRect().size().width(),
                size().height())));

    //sometimes the background of applets change on the go...
    if (m_separator) {
        if (m_applet->backgroundHints() == Plasma::Applet::NoBackground) {
            m_background = new Plasma::FrameSvg(this);
            m_background->setImagePath("widgets/background");
            m_separator->deleteLater();
            m_separator = 0;
            syncMargins();
        }
    } else {
        if (m_applet->backgroundHints() & Plasma::Applet::StandardBackground ||
            m_applet->backgroundHints() & Plasma::Applet::TranslucentBackground) {
            m_separator = new Plasma::Svg(this);
            m_separator->setImagePath("widgets/line");
            m_separator->setContainsMultipleImages(true);
            m_background->deleteLater();
            m_background = 0;
            syncMargins();
        }
    }
}

void AppletTitleBar::syncIconRects()
{
    m_maximizeButtonRect.moveTopLeft(contentsRect().topLeft());
    m_configureButtonRect.moveTopLeft(contentsRect().topLeft());

    if (m_applet->hasValidAssociatedApplication()) {
        m_configureButtonRect.moveLeft(contentsRect().left() + m_maximizeButtonRect.width() + 2);
    }

    m_closeButtonRect.moveTopRight(contentsRect().topRight());
}

bool AppletTitleBar::eventFilter(QObject *watched, QEvent *event)
{
    Q_UNUSED(watched)

    if (event->type() == QEvent::GraphicsSceneResize) {
        syncSize();
    } else if (event->type() == QEvent::GraphicsSceneHoverEnter) {
        m_underMouse = true;
        syncIconRects();

        if (!m_forcedButtonsVisible) {
            setButtonsVisible(true);
        }
    } else if (event->type() == QEvent::GraphicsSceneHoverLeave) {
        m_underMouse = false;

        if (!m_forcedButtonsVisible) {
            setButtonsVisible(false);
        }
    }

    return false;
}

void AppletTitleBar::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (m_applet->hasValidAssociatedApplication() &&
        m_maximizeButtonRect.contains(event->pos())) {
        m_pressedButton = MaximizeButton;
        m_maximizeButtonRect.translate(1, 1);
        update(m_maximizeButtonRect);
        event->accept();
    } else if (m_configureButtonRect.contains(event->pos())) {
        m_configureButtonRect.translate(1, 1);
        m_pressedButton = ConfigureButton;
        update(m_configureButtonRect);
        event->accept();
    } else if (m_closeButtonRect.contains(event->pos())) {
        m_closeButtonRect.translate(1, 1);
        m_pressedButton = CloseButton;
        update(m_closeButtonRect);
        event->accept();
    } else {
        event->ignore();
    }
}

void AppletTitleBar::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    event->ignore();
}

void AppletTitleBar::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (m_pressedButton == MaximizeButton && m_maximizeButtonRect.contains(event->pos())) {
        if (m_applet->hasValidAssociatedApplication()) {
            m_pulse->start();
            m_applet->runAssociatedApplication();
        }
    } else if (m_pressedButton == ConfigureButton && m_configureButtonRect.contains(event->pos())) {
        if (m_applet->hasConfigurationInterface()) {
            m_applet->showConfigurationInterface();
        }
    } else if (m_pressedButton == CloseButton && m_closeButtonRect.contains(event->pos())) {
        if (m_applet->immutability() == Plasma::Mutable) {
            m_applet->destroy();
        }
    } else {
        event->ignore();
    }

    switch (m_pressedButton) {
    case MaximizeButton:
        m_maximizeButtonRect.translate(-1, -1);
        update(m_maximizeButtonRect);
        break;
    case ConfigureButton:
        m_configureButtonRect.translate(-1, -1);
        update(m_configureButtonRect);
        break;
    case CloseButton:
        m_closeButtonRect.translate(-1, -1);
        update(m_closeButtonRect);
        break;
    default:
        break;
    }

    m_pressedButton = NoButton;
}

void AppletTitleBar::resizeEvent(QGraphicsSceneResizeEvent *event)
{
    if (m_background) {
        m_background->resizeFrame(event->newSize());
    }

    syncIconRects();
}

void AppletTitleBar::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

    if (m_background && (!m_appletHasBackground || m_buttonsVisible)) {
        m_background->paintFrame(painter);
    }

    if (m_buttonsVisible) {
        QParallelAnimationGroup *group = m_animations.data();

        int i = 0;

        if (m_applet->hasValidAssociatedApplication()) {
            if (group) {
                if (group->state() == QAbstractAnimation::Running) {
                    QAbstractAnimation *maximizeAnim = group->animationAt(i);
                    ++i;
                    QPixmap animPixmap = qvariant_cast<QPixmap>(maximizeAnim->property("currentPixmap"));
                    painter->drawPixmap(m_maximizeButtonRect, animPixmap, animPixmap.rect());
                 } else if (group->state() == QAbstractAnimation::Stopped && group->direction() != QAbstractAnimation::Backward) {
                     m_icons->paint(painter, m_maximizeButtonRect, "maximize");
                 }
            }
        }

        if (m_applet->hasConfigurationInterface()) {
            if (group) {
                if (group->state() == QAbstractAnimation::Running) {
                    QAbstractAnimation *confAnim = group->animationAt(i);
                    ++i;
                    QPixmap animPixmap = qvariant_cast<QPixmap>(confAnim->property("currentPixmap"));
                    painter->drawPixmap(m_configureButtonRect, animPixmap, animPixmap.rect());
                 } else if (group->state() == QAbstractAnimation::Stopped && group->direction() != QAbstractAnimation::Backward) {
                     m_icons->paint(painter, m_configureButtonRect, "configure");
                 }
            }
        }

        if (m_applet->immutability() == Plasma::Mutable) {
            if (group) {
                if (group->state() == QAbstractAnimation::Running) {
                    QAbstractAnimation *closeAnim = group->animationAt(i);
                    if (closeAnim) {
                        ++i;
                        QPixmap animPixmap = qvariant_cast<QPixmap>(closeAnim->property("currentPixmap"));
                        painter->drawPixmap(m_closeButtonRect, animPixmap, animPixmap.rect());
                    }
                } else if (group->state() == QAbstractAnimation::Stopped && group->direction() != QAbstractAnimation::Backward) {
                   m_icons->paint(painter, m_closeButtonRect, "close");
                }
            }
        }
    }

    painter->save();
    painter->setPen(Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor));
    painter->setFont(Plasma::Theme::defaultTheme()->font(Plasma::Theme::DefaultFont));
    painter->drawText(contentsRect(), Qt::AlignCenter, m_applet->name());
    painter->restore();

    if (m_separator) {
        QRectF lineRect = contentsRect();
        lineRect.setTop(lineRect.bottom() - m_separator->elementSize("horizontal-line").height());
        m_separator->paint(painter, lineRect, "horizontal-line");
    }
}

void AppletTitleBar::appletRemoved(Plasma::Applet *applet)
{
    if (applet == m_applet) {
        qreal left, top, right, bottom;
        m_applet->getContentsMargins(&left, &top, &right, &bottom);
        m_applet->setContentsMargins(left, m_savedAppletTopMargin + size().height(), right, bottom);
        deleteLater();
    }
}

void AppletTitleBar::themeChanged()
{
    //send the margin update in the back of eveny queue,
    //so it will be executed after the margins update by Plasma::Applet
    QTimer::singleShot(0, this, SLOT(syncMargins()));
}


#include <applettitlebar.moc>
