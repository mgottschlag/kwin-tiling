/*
 *   Copyright 2009 by Artur Duque de Souza <morpheuz@gmail.com>
 *
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

#include "stripwidget.h"

#include <Plasma/FrameSvg>
#include <Plasma/Theme>
#include <Plasma/PushButton>
#include <Plasma/IconWidget>
#include <Plasma/QueryMatch>
#include <Plasma/RunnerManager>

#include <KIcon>
#include <KPushButton>

#include <QAction>

StripWidget::StripWidget(Plasma::RunnerManager *rm, QGraphicsItem *parent)
    : QGraphicsWidget(parent), runnermg(rm)
{
    // FIXME: it's not working =/
    background = new Plasma::FrameSvg(this);
    background->setImagePath("widgets/background");
    background->resizeFrame(geometry().size());

    stripLayout = new QGraphicsLinearLayout(this);

    leftArrow = new Plasma::PushButton(this);
    leftArrow->nativeWidget()->setIcon(KIcon("arrow-left"));
    leftArrow->setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum));
    leftArrow->setMaximumSize(IconSize(KIconLoader::MainToolbar),
                              IconSize(KIconLoader::MainToolbar));

    rightArrow = new Plasma::PushButton(this);
    rightArrow->nativeWidget()->setIcon(KIcon("arrow-right"));
    rightArrow->setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum));
    rightArrow->setMaximumSize(IconSize(KIconLoader::MainToolbar),
                               IconSize(KIconLoader::MainToolbar));

    leftArrow->setEnabled(false);
    rightArrow->setEnabled(false);

    stripLayout->addItem(leftArrow);
    stripLayout->setAlignment(leftArrow, Qt::AlignLeft);
    stripLayout->addItem(rightArrow);
    stripLayout->setAlignment(rightArrow, Qt::AlignRight);
}

StripWidget::~StripWidget()
{
}

void StripWidget::resizeEvent(QGraphicsSceneResizeEvent *event)
{
    QGraphicsWidget::resizeEvent(event);
    background->resizeFrame(boundingRect().size());
}

void StripWidget::add(Plasma::QueryMatch match)
{
    // create new IconWidget for favourite strip
    Plasma::IconWidget *fav = new Plasma::IconWidget();
    fav->setText(match.text());
    fav->setIcon(match.icon());
    fav->setMinimumSize(QSize(120, 120));
    fav->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed, QSizePolicy::DefaultType);
    connect(fav, SIGNAL(activated()), this, SLOT(launchFavourite()));

    // set an action to be able to remove from favourites
    QAction *action = new QAction(fav);
    action->setIcon(KIcon("list-remove"));
    fav->addIconAction(action);
    connect(action, SIGNAL(triggered()), this, SLOT(removeFavourite()));

    // add to layout and data structures
    m_favourites.append(fav);
    m_favouritesMatches.append(match);

    int idx = stripLayout->count() - 1;
    stripLayout->insertItem(idx, fav);

    if (stripLayout->count() > 7) {
        leftArrow->setEnabled(true);
        rightArrow->setEnabled(true);
    }
}

void StripWidget::remove(Plasma::IconWidget *favourite)
{
    int idx = m_favourites.indexOf(favourite);
    m_favouritesMatches.removeAt(idx);
    m_favourites.removeAt(idx);

    // must be deleteLater because the IconWidget will return from the action?
    favourite->deleteLater();

    // the IconWidget was not removed yet
    if (stripLayout->count() <= 8) {
        leftArrow->setEnabled(false);
        rightArrow->setEnabled(false);
    }
}

void StripWidget::removeFavourite()
{
    Plasma::IconWidget *icon = static_cast<Plasma::IconWidget*>(sender()->parent());
    remove(icon);
}

void StripWidget::launchFavourite()
{
    Plasma::IconWidget *icon = static_cast<Plasma::IconWidget*>(sender());
    int idx = m_favourites.indexOf(icon);
    Plasma::QueryMatch match = m_favouritesMatches[idx];
    runnermg->run(match);
}
