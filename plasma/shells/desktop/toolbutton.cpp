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

//KDE
#include <KColorUtils>

//Plasma
#include <Plasma/PaintUtils>
#include <Plasma/Theme>
#include <Plasma/FrameSvg>
#include <Plasma/Animator>

ToolButton::ToolButton(QWidget *parent)
    : QToolButton(parent),
      m_action(0),
      m_animationId(0),
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

    if (m_animationId || (buttonOpt.state & QStyle::State_MouseOver) || (buttonOpt.state & QStyle::State_On)) {
        if (buttonOpt.state & QStyle::State_Sunken || (buttonOpt.state & QStyle::State_On)) {
            m_background->setElementPrefix("toolbutton-pressed");
        } else {
            m_background->setElementPrefix("normal");
        }
        m_background->resizeFrame(size());

        if (m_animationId) {
            QPixmap buffer = m_background->framePixmap();

            QPainter bufferPainter(&buffer);
            bufferPainter.setCompositionMode(QPainter::CompositionMode_DestinationIn);
            QColor alphaColor(Qt::black);
            alphaColor.setAlphaF(qMin(0.95, m_alpha));
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

    style()->drawControl(QStyle::CE_ToolButtonLabel, &buttonOpt, &painter, this);
}

void ToolButton::enterEvent(QEvent *event)
{
    if (isChecked()) {
        return;
    }

    const int FadeInDuration = 75;

    if (m_animationId) {
        Plasma::Animator::self()->stopElementAnimation(m_animationId);
    }

    m_fadeIn = true;
    m_animationId = Plasma::Animator::self()->customAnimation(
        40 / (1000 / FadeInDuration), FadeInDuration,
        Plasma::Animator::LinearCurve, this, "animationUpdate");
}

void ToolButton::leaveEvent(QEvent *event)
{
    if (isChecked()) {
        return;
    }

    const int FadeOutDuration = 150;

    if (m_animationId) {
        Plasma::Animator::self()->stopElementAnimation(m_animationId);
    }

    m_fadeIn = false;
    m_animationId = Plasma::Animator::self()->customAnimation(
        40 / (1000 / FadeOutDuration), FadeOutDuration,
        Plasma::Animator::LinearCurve, this, "animationUpdate");
}


void ToolButton::animationUpdate(qreal progress)
{
    if (qFuzzyCompare(progress, 1)) {
        m_animationId = 0;
        m_fadeIn = true;
    }

    m_alpha = m_fadeIn ? progress : 1 - progress;

    // explicit update
    update();
}

#include "toolbutton.moc"

