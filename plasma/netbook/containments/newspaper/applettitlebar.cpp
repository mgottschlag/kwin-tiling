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
#include <QLabel>
#include <QPainter>
#include <QTimer>


#include <KIconLoader>

#include <Plasma/Animator>
#include <Plasma/IconWidget>
#include <Plasma/Label>
#include <Plasma/Separator>
#include <Plasma/Svg>
#include <Plasma/Theme>


AppletTitleBar::AppletTitleBar(Plasma::Applet *applet)
       : QGraphicsWidget(applet),
         m_applet(applet),
         m_pressedButton(NoButton),
         m_maximizeButtonAnimationId(0),
         m_configureButtonAnimationId(0),
         m_closeButtonAnimationId(0),
         m_separator(0),
         m_background(0),
         m_savedAppletTopMargin(0),
         m_underMouse(false),
         m_showButtons(false)
{
    m_maximizeButtonRect = m_configureButtonRect = m_closeButtonRect = QRect(0, 0, KIconLoader::SizeSmallMedium, KIconLoader::SizeSmallMedium);

    m_icons = new Plasma::Svg(this);
    m_icons->setImagePath("widgets/configuration-icons");
    m_icons->setContainsMultipleImages(true);


    if (applet->backgroundHints() & Plasma::Applet::StandardBackground) {
        m_separator = new Plasma::Svg(this);
        m_separator->setImagePath("widgets/line");
        m_separator->setContainsMultipleImages(true);
    } else {
        m_background = new Plasma::FrameSvg(this);
        m_background->setImagePath("widgets/frame");
        m_background->setElementPrefix("raised");
    }

    applet->installEventFilter(this);
    syncMargins();
    syncSize();

    if (applet->containment()) {
        connect(applet->containment(), SIGNAL(appletRemoved(Plasma::Applet *)), this, SLOT(appletRemoved(Plasma::Applet *)));
    }
    connect(Plasma::Theme::defaultTheme(), SIGNAL(themeChanged()), this, SLOT(themeChanged()));

    connect(Plasma::Animator::self(), SIGNAL(elementAnimationFinished(int)), this, SLOT(animationFinished(int)));
}

AppletTitleBar::~AppletTitleBar()
{
}


void AppletTitleBar::syncMargins()
{
    const int extraMargin = 2;
    syncIconRects();
    setMinimumHeight(m_maximizeButtonRect.height() + extraMargin);
    setMaximumHeight(m_maximizeButtonRect.height() + extraMargin);

    if (m_background) {
        qreal left, top, right, bottom;

        m_background->getMargins(left, top, right, bottom);
        setContentsMargins(left, top, right, bottom);
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
        m_showButtons = true;
        syncIconRects();

        if (m_maximizeButtonAnimationId) {
            Plasma::Animator::self()->stopElementAnimation(m_maximizeButtonAnimationId);
        }
        m_maximizeButtonAnimationId = Plasma::Animator::self()->animateElement(this, Plasma::Animator::AppearAnimation);
        Plasma::Animator::self()->setInitialPixmap(m_maximizeButtonAnimationId, m_icons->pixmap("maximize"));

        if (m_configureButtonAnimationId) {
            Plasma::Animator::self()->stopElementAnimation(m_maximizeButtonAnimationId);
        }
        m_configureButtonAnimationId = Plasma::Animator::self()->animateElement(this, Plasma::Animator::AppearAnimation);
        Plasma::Animator::self()->setInitialPixmap(m_configureButtonAnimationId, m_icons->pixmap("configure"));

        if (m_closeButtonAnimationId) {
            Plasma::Animator::self()->stopElementAnimation(m_maximizeButtonAnimationId);
        }
        m_closeButtonAnimationId = Plasma::Animator::self()->animateElement(this, Plasma::Animator::AppearAnimation);
        Plasma::Animator::self()->setInitialPixmap(m_closeButtonAnimationId, m_icons->pixmap("close"));

        update();
    } else if (event->type() == QEvent::GraphicsSceneHoverLeave) {
        m_underMouse = false;
        if (m_maximizeButtonAnimationId) {
            Plasma::Animator::self()->stopElementAnimation(m_maximizeButtonAnimationId);
        }
        m_maximizeButtonAnimationId = Plasma::Animator::self()->animateElement(this, Plasma::Animator::DisappearAnimation);
        Plasma::Animator::self()->setInitialPixmap(m_maximizeButtonAnimationId, m_icons->pixmap("maximize"));

        if (m_configureButtonAnimationId) {
            Plasma::Animator::self()->stopElementAnimation(m_maximizeButtonAnimationId);
        }
        m_configureButtonAnimationId = Plasma::Animator::self()->animateElement(this, Plasma::Animator::DisappearAnimation);
        Plasma::Animator::self()->setInitialPixmap(m_configureButtonAnimationId, m_icons->pixmap("configure"));

        if (m_closeButtonAnimationId) {
            Plasma::Animator::self()->stopElementAnimation(m_maximizeButtonAnimationId);
        }
        m_closeButtonAnimationId = Plasma::Animator::self()->animateElement(this, Plasma::Animator::DisappearAnimation);
        Plasma::Animator::self()->setInitialPixmap(m_closeButtonAnimationId, m_icons->pixmap("close"));

        update();
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
    } else if (m_configureButtonRect.contains(event->pos())) {
        m_configureButtonRect.translate(1, 1);
        m_pressedButton = ConfigureButton;
        update(m_configureButtonRect);
    } else if (m_closeButtonRect.contains(event->pos())) {
        m_closeButtonRect.translate(1, 1);
        m_pressedButton = CloseButton;
        update(m_closeButtonRect);
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

    if (m_background) {
        m_background->paintFrame(painter);
    }

    if (m_showButtons) {
        if (m_applet->hasValidAssociatedApplication()) {
            if (m_maximizeButtonAnimationId) {
                QPixmap animPixmap = Plasma::Animator::self()->currentPixmap(m_maximizeButtonAnimationId);
                painter->drawPixmap(m_maximizeButtonRect, animPixmap, animPixmap.rect());
            } else {
                m_icons->paint(painter, m_maximizeButtonRect, "maximize");
            }
        }
        if (m_applet->hasConfigurationInterface()) {
            if (m_maximizeButtonAnimationId) {
                QPixmap animPixmap = Plasma::Animator::self()->currentPixmap(m_configureButtonAnimationId);
                painter->drawPixmap(m_configureButtonRect, animPixmap, animPixmap.rect());
            } else {
                m_icons->paint(painter, m_configureButtonRect, "configure");
            }
        }
        if (m_applet->immutability() == Plasma::Mutable) {
            if (m_maximizeButtonAnimationId) {
                QPixmap animPixmap = Plasma::Animator::self()->currentPixmap(m_closeButtonAnimationId);
                painter->drawPixmap(m_closeButtonRect, animPixmap, animPixmap.rect());
            } else {
                m_icons->paint(painter, m_closeButtonRect, "close");
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
        lineRect.setTop(lineRect.height() - m_separator->elementSize("horizontal-line").height());
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

void AppletTitleBar::animationFinished(int id)
{
    if (!m_underMouse) {
        m_showButtons = false;
    }

    if (id == m_maximizeButtonAnimationId) {
        m_maximizeButtonAnimationId = 0;
    } else if (id == m_configureButtonAnimationId) {
        m_configureButtonAnimationId = 0;
    } else if (id == m_closeButtonAnimationId) {
        m_closeButtonAnimationId = 0;
    }
}

#include <applettitlebar.moc>
