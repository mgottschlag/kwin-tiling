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

#include <Plasma/Frame>
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
    background = new Plasma::Frame();
    background->setFrameShadow(Plasma::Frame::Raised);
    background->setMinimumSize(QSize(600, 115));
    background->setMaximumSize(QSize(600, 115));

    // mainLayout to correctly setup the background
    QGraphicsLinearLayout *mainLayout = new QGraphicsLinearLayout(this);
    mainLayout->addItem(background);

    arrowsLayout = new QGraphicsLinearLayout(background);
    stripLayout = new QGraphicsLinearLayout();

    leftArrow = new Plasma::PushButton(this);
    leftArrow->nativeWidget()->setIcon(KIcon("arrow-left"));
    leftArrow->setMaximumSize(IconSize(KIconLoader::Panel),
                              IconSize(KIconLoader::Panel));
    connect(leftArrow, SIGNAL(clicked()), this, SLOT(goLeft()));

    rightArrow = new Plasma::PushButton(this);
    rightArrow->nativeWidget()->setIcon(KIcon("arrow-right"));
    rightArrow->setMaximumSize(IconSize(KIconLoader::Panel),
                               IconSize(KIconLoader::Panel));
    connect(rightArrow, SIGNAL(clicked()), this, SLOT(goRight()));

    leftArrow->setEnabled(false);
    rightArrow->setEnabled(false);

    arrowsLayout->addItem(leftArrow);
    arrowsLayout->addItem(stripLayout);
    arrowsLayout->addItem(rightArrow);
}

StripWidget::~StripWidget()
{
}

void StripWidget::createIcon(Plasma::QueryMatch *match, int idx)
{
    // create new IconWidget for favourite strip
    Plasma::IconWidget *fav = new Plasma::IconWidget();
    fav->setText(match->text());
    fav->setIcon(match->icon());
    fav->setMinimumSize(QSize(100, 100));
    fav->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
    connect(fav, SIGNAL(activated()), this, SLOT(launchFavourite()));

    // set an action to be able to remove from favourites
    QAction *action = new QAction(fav);
    action->setIcon(KIcon("list-remove"));
    fav->addIconAction(action);
    connect(action, SIGNAL(triggered()), this, SLOT(removeFavourite()));

    m_favouriteMap.insert(fav, match);
    stripLayout->insertItem(idx, fav);
}

void StripWidget::add(Plasma::QueryMatch match)
{
    // add to layout and data structures
    Plasma::QueryMatch *newMatch = new Plasma::QueryMatch(match);
    m_favouritesMatches.append(newMatch);

    int idx = stripLayout->count();
    if (idx > 4) {
        leftArrow->setEnabled(true);
        rightArrow->setEnabled(true);
    } else {
        createIcon(newMatch, idx);
    }
}

void StripWidget::remove(Plasma::IconWidget *favourite)
{
    Plasma::QueryMatch *match = m_favouriteMap.value(favourite);
    m_favouritesMatches.removeOne(match);
    m_favouriteMap.remove(favourite);

    // must be deleteLater because the IconWidget will return from the action?
    favourite->deleteLater();
    delete match;

    // the IconWidget was not removed yet
    if (m_favouritesMatches.size() < 5) {
        leftArrow->setEnabled(false);
        rightArrow->setEnabled(false);
    } else {
        // adds the new item to the end of the list
        int idx = m_favouritesMatches.indexOf(match);
        int newpos = (idx + 5) % m_favouritesMatches.size();

        match = m_favouritesMatches[newpos];
        // must be 5 here because at this point the icon was not deleted yet
        createIcon(match, 5);
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
    Plasma::QueryMatch *match = m_favouriteMap.value(icon);
    runnermg->run(*match);
}

void StripWidget::goLeft()
{
    // discover the item that will be removed
    Plasma::IconWidget *icon = static_cast<Plasma::IconWidget*>(stripLayout->itemAt(0));
    Plasma::QueryMatch *match = m_favouriteMap.value(icon);

    // removes the first item
    m_favouriteMap.remove(icon);
    icon->hide();
    delete icon;

    // adds the new item to the end of the list
    int idx = m_favouritesMatches.indexOf(match);
    int newpos = (idx + 5) % m_favouritesMatches.size();
    match = m_favouritesMatches[newpos];
    createIcon(match, 4);
}

void StripWidget::goRight()
{
    // discover the item that will be removed
    Plasma::IconWidget *icon = static_cast<Plasma::IconWidget*>(stripLayout->itemAt(4));
    Plasma::QueryMatch *match = m_favouriteMap.value(icon);

    // removes the first item
    m_favouriteMap.remove(icon);
    icon->hide();
    delete icon;

    icon = static_cast<Plasma::IconWidget*>(stripLayout->itemAt(0));
    match = m_favouriteMap.value(icon);

    // adds the new item to the end of the list
    int idx = m_favouritesMatches.indexOf(match);
    int size = m_favouritesMatches.size();
    int newpos = (idx + size - 1) % size;
    match = m_favouritesMatches[newpos];
    createIcon(match, 0);
}
