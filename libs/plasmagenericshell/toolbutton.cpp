/*
 *   Copyright 2008 Marco Martin <notmart@gmail.com>
 *   Copyright 2008 Aaron Seigo <aseigo@kde.org>
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

#include "toolbutton.h"

//Qt
#include <QAction>
#include <QPainter>
#include <QPaintEvent>
#include <QStyle>
#include <QStyleOptionToolButton>
#include <QGraphicsSceneHoverEvent>
#include <QEasingCurve>
#include <QPropertyAnimation>

//KDE
#include <KColorUtils>

//Plasma
#include <Plasma/PaintUtils>
#include <Plasma/Theme>
#include <Plasma/FrameSvg>

ToolButton::ToolButton(QWidget *parent)
    : QToolButton(parent),
      m_action(0),
      m_isAnimating(false),
      m_alpha(0),
      m_fadeIn(true)
{
    m_background = new Plasma::FrameSvg(this);
    m_background->setImagePath("widgets/button");
    m_background->setCacheAllRenderedFrames(true);
    m_background->setElementPrefix("plain");
}

void ToolButton::setAction(QAction *action)
{
    if (!action) {
        return;
    }

    if (m_action) {
        disconnect(m_action, SIGNAL(changed()), this, SLOT(syncToAction()));
        disconnect(this, SIGNAL(clicked()), m_action, SLOT(trigger()));
    }

    m_action = action;
    connect(m_action, SIGNAL(changed()), this, SLOT(syncToAction()));
    connect(this, SIGNAL(clicked()), m_action, SLOT(trigger()));
    connect(m_action, SIGNAL(destroyed(QObject*)), this, SLOT(actionDestroyed(QObject*)));
    syncToAction();
}

void ToolButton::syncToAction()
{
    if (!m_action) {
        return;
    }

    setIcon(m_action->icon());
    setText(m_action->text());

    if (toolButtonStyle() == Qt::ToolButtonIconOnly) {
        setToolTip(m_action->text());
    }

    setCheckable(m_action->isCheckable());
    if (m_action->actionGroup()) {
        setAutoExclusive(m_action->actionGroup()->isExclusive());
    }

    setEnabled(m_action->isEnabled());
}

void ToolButton::actionDestroyed(QObject *)
{
    m_action = 0;
}

void ToolButton::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPainter painter(this);

    QStyleOptionToolButton buttonOpt;
    initStyleOption(&buttonOpt);

    if (m_isAnimating || (buttonOpt.state & QStyle::State_MouseOver) || (buttonOpt.state & QStyle::State_On)) {
        if (buttonOpt.state & QStyle::State_Sunken || (buttonOpt.state & QStyle::State_On)) {
            m_background->setElementPrefix("pressed");
        } else {
            m_background->setElementPrefix("normal");
        }
        m_background->resizeFrame(size());

        if (m_isAnimating) {
            QPixmap buffer = m_background->framePixmap();

            QPainter bufferPainter(&buffer);
            bufferPainter.setCompositionMode(QPainter::CompositionMode_DestinationIn);
            QColor alphaColor(Qt::black);
            alphaColor.setAlphaF(qMin(qreal(0.95), m_alpha));
            bufferPainter.fillRect(buffer.rect(), alphaColor);
            bufferPainter.end();

            painter.drawPixmap(QPoint(0,0), buffer);

            buttonOpt.palette.setColor(QPalette::ButtonText, KColorUtils::mix(Plasma::Theme::defaultTheme()->color(Plasma::Theme::ButtonTextColor), Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor), 1-m_alpha));
        } else {
            m_background->paintFrame(&painter);
            buttonOpt.palette.setColor(QPalette::ButtonText, Plasma::Theme::defaultTheme()->color(Plasma::Theme::ButtonTextColor));
        }
    } else {
        buttonOpt.palette.setColor(QPalette::ButtonText, Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor));
    }

    if (hasFocus()) {
        m_background->setElementPrefix("focus");
        m_background->resizeFrame(size());
        m_background->paintFrame(&painter);
    }

    style()->drawControl(QStyle::CE_ToolButtonLabel, &buttonOpt, &painter, this);
}

void ToolButton::enterEvent(QEvent *event)
{
    Q_UNUSED(event)

    if (isChecked()) {
        return;
    }

    QPropertyAnimation *animation = m_animation.data();
    if (animation) {
        animation->stop();
        m_animation.clear();
    }

    m_fadeIn = true;
    m_isAnimating = true;

    animation = new QPropertyAnimation(this, "alphaValue");
    animation->setProperty("duration", 75);
    animation->setProperty("startValue", 0.0);
    animation->setProperty("endValue", 1.0);
    animation->start();
    m_animation = animation;

    connect(animation, SIGNAL(finished()), this, SLOT(animationFinished()));
}

void ToolButton::leaveEvent(QEvent *event)
{
    Q_UNUSED(event)

    if (isChecked()) {
        return;
    }

    QPropertyAnimation *animation = m_animation.data();
    if (!animation) {
        return;
    }

    if (m_isAnimating) {
        animation->stop();
    }

    m_fadeIn = false;
    m_isAnimating = true;

    animation->setProperty("duration", 150);
    animation->start(QAbstractAnimation::DeleteWhenStopped);
}

qreal ToolButton::alphaValue() const
{
    return m_alpha;
}

void ToolButton::setAlphaValue(qreal progress)
{
    m_alpha = m_fadeIn ? progress : 1 - progress;

    // explicit update
    update();
}

void ToolButton::animationFinished()
{
    m_isAnimating = false;
}

#include "toolbutton.moc"

