/*
 *   Copyright 2009 by Artur Duque de Souza <asouza@kde.org>
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

#include <QToolButton>
#include <QAction>

#include <KIcon>
#include <KIconLoader>

#include <Plasma/Frame>
#include <Plasma/ToolButton>
#include <Plasma/IconWidget>
#include <Plasma/QueryMatch>
#include <Plasma/AbstractRunner>
#include <Plasma/RunnerManager>
#include <Plasma/ItemBackground>
#include <Plasma/ScrollWidget>


StripWidget::StripWidget(Plasma::RunnerManager *rm, QGraphicsWidget *parent)
    : Plasma::Frame(parent),
      m_runnermg(rm),
      m_scrollWidget(0),
      m_scrollingWidget(0),
      m_offset(0),
      m_currentIcon(0),
      m_currentIconIndex(-1)
{
    setFrameShadow(Plasma::Frame::Raised);
    setEnabledBorders(Plasma::FrameSvg::BottomBorder);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    //FIXME: layout problems, do it right
    //setPreferredSize(500, 128);

    m_arrowsLayout = new QGraphicsLinearLayout(this);
    m_stripLayout = new QGraphicsLinearLayout();
    m_hoverIndicator = new Plasma::ItemBackground(this);
    m_hoverIndicator->hide();
    m_hoverIndicator->setZValue(-100);
    setAcceptHoverEvents(true);
    setFocusPolicy(Qt::StrongFocus);

    m_leftArrow = new Plasma::ToolButton(this);
    m_leftArrow->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    m_leftArrow->setPreferredWidth(KIconLoader::SizeMedium);
    m_leftArrow->nativeWidget()->setIcon(KIcon("arrow-left"));
    connect(m_leftArrow, SIGNAL(clicked()), this, SLOT(goLeft()));

    m_rightArrow = new Plasma::ToolButton(this);
    m_rightArrow->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    m_rightArrow->setPreferredWidth(KIconLoader::SizeMedium);
    m_rightArrow->nativeWidget()->setIcon(KIcon("arrow-right"));
    connect(m_rightArrow, SIGNAL(clicked()), this, SLOT(goRight()));

    m_leftArrow->setEnabled(false);
    m_rightArrow->setEnabled(false);

    m_scrollWidget = new Plasma::ScrollWidget(this);
    m_scrollWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scrollWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scrollingWidget = new QGraphicsWidget(m_scrollWidget);
    m_scrollingWidget->installEventFilter(this);
    QGraphicsLinearLayout *scrollingLayout = new QGraphicsLinearLayout(m_scrollingWidget);
    QGraphicsWidget *leftSpacer = new QGraphicsWidget(m_scrollingWidget);
    QGraphicsWidget *rightSpacer = new QGraphicsWidget(m_scrollingWidget);
    leftSpacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    rightSpacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    /*leftSpacer->setPreferredSize(0, 0);
    rightSpacer->setPreferredSize(0, 0);*/
    scrollingLayout->addItem(leftSpacer);
    scrollingLayout->addItem(m_stripLayout);
    scrollingLayout->addItem(rightSpacer);

    m_scrollWidget->setWidget(m_scrollingWidget);

    m_arrowsLayout->addItem(m_leftArrow);
    m_arrowsLayout->addItem(m_scrollWidget);
    m_arrowsLayout->addItem(m_rightArrow);
}

StripWidget::~StripWidget()
{
}

void StripWidget::createIcon(Plasma::QueryMatch *match, int idx)
{
    // create new IconWidget for favourite strip

    Plasma::IconWidget *fav = new Plasma::IconWidget(this);
    fav->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    fav->installEventFilter(this);
    fav->setText(match->text());
    fav->setIcon(match->icon());

    qreal left, top, right, bottom;
    m_hoverIndicator->getContentsMargins(&left, &top, &right, &bottom);
    fav->setContentsMargins(left, top, right, bottom);

    fav->setMinimumSize(fav->sizeFromIconSize(KIconLoader::SizeLarge));
    fav->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    connect(fav, SIGNAL(activated()), this, SLOT(launchFavourite()));

    fav->setMinimumSize(fav->size().height(), fav->size().height());
    fav->setPos(fav->size().width()/2-fav->size().width()/2, fav->size().height()/2-fav->size().height()/2);

    // set an action to be able to remove from favourites
    QAction *action = new QAction(fav);
    action->setIcon(KIcon("list-remove"));
    fav->addIconAction(action);
    connect(action, SIGNAL(triggered()), this, SLOT(removeFavourite()));

    m_favouritesIcons.insert(fav, match);
    m_stripLayout->insertItem(idx, fav);
    m_stripLayout->activate();
    m_scrollWidget->setMinimumHeight(m_scrollingWidget->effectiveSizeHint(Qt::PreferredSize).height());
}

void StripWidget::add(Plasma::QueryMatch match, const QString &query)
{
    // add to layout and data structures
    Plasma::QueryMatch *newMatch = new Plasma::QueryMatch(match);
    m_favouritesMatches.append(newMatch);
    m_favouritesQueries.insert(newMatch, query);

    int idx = m_stripLayout->count();
    createIcon(newMatch, idx);
}

void StripWidget::remove(Plasma::IconWidget *favourite)
{
    Plasma::QueryMatch *match = m_favouritesIcons.value(favourite);
    m_favouritesMatches.removeOne(match);
    m_favouritesQueries.remove(match);
    m_favouritesIcons.remove(favourite);

    // must be deleteLater because the IconWidget will return from the action?
    favourite->deleteLater();
    delete match;
}

void StripWidget::removeFavourite()
{
    Plasma::IconWidget *icon = static_cast<Plasma::IconWidget*>(sender()->parent());

    if (icon) {
        m_stripLayout->setMinimumSize(icon->size().width()*(m_stripLayout->count()-2), icon->size().height());
        m_stripLayout->setMaximumSize(m_stripLayout->minimumSize());

        remove(icon);
    }
}

void StripWidget::launchFavourite()
{
    Plasma::IconWidget *icon = static_cast<Plasma::IconWidget*>(sender());
    Plasma::QueryMatch *match = m_favouritesIcons.value(icon);

    Plasma::RunnerContext context;
    context.setQuery(m_favouritesQueries.value(match));
    match->run(context);
}

void StripWidget::launchFavourite(Plasma::IconWidget *icon)
{
    Plasma::QueryMatch *match = m_favouritesIcons.value(icon);

    Plasma::RunnerContext context;
    context.setQuery(m_favouritesQueries.value(match));
    match->run(context);
}

void StripWidget::goRight()
{
    //FIXME: no hardcoded and animated
    QPointF oldPos = m_scrollingWidget->pos();
    m_scrollingWidget->setPos(qMax(oldPos.x() - 100, -(m_scrollingWidget->size().width() - m_scrollWidget->size().width())), oldPos.y());
}

void StripWidget::goLeft()
{
    QPointF oldPos = m_scrollingWidget->pos();
    m_scrollingWidget->setPos(qMax(oldPos.x() + 100, (qreal)0.0), oldPos.y());
}

void StripWidget::save(KConfigGroup &cg)
{
    kDebug() << "----------------> Saving Stuff...";

    // erase the old stuff before saving the new one
    KConfigGroup oldGroup(&cg, "stripwidget");
    oldGroup.deleteGroup();

    KConfigGroup stripGroup(&cg, "stripwidget");

    int id = 0;
    foreach(Plasma::QueryMatch *match, m_favouritesMatches) {
        // Write now just saves one for tests. Later will save
        // all the strip
        KConfigGroup config(&stripGroup, QString(id));
        config.writeEntry("runnerid", match->runner()->id());
        config.writeEntry("query", m_favouritesQueries.value(match));
        config.writeEntry("matchId", match->id());
        id++;
    }
}

void StripWidget::restore(KConfigGroup &cg)
{
    kDebug() << "----------------> Restoring Stuff...";

    KConfigGroup stripGroup(&cg, "stripwidget");

    // get all the favourites
    QList<KConfigGroup> favouritesConfigs;
    foreach (const QString &favouriteGroup, stripGroup.groupList()) {
        KConfigGroup favouriteConfig(&stripGroup, favouriteGroup);
        favouritesConfigs.append(favouriteConfig);
    }


    QMutableListIterator<KConfigGroup> it(favouritesConfigs);
    while (it.hasNext()) {
        KConfigGroup &favouriteConfig = it.next();

        QString runnerId = favouriteConfig.readEntry("runnerid");
        QString query = favouriteConfig.readEntry("query");
        QString matchId = favouriteConfig.readEntry("matchId");

        // This is the "lazy mode". We do the queries again just for
        // that runner (should be fast). This way we are able to put
        // contacts and nepomuk stuff on the favourites strip.
        // Later we can improve load time for specific runners (services)
        // so we do not have to query again.

        // perform the query
        if (m_runnermg->execQuery(query, runnerId)) {
            // find our match
            Plasma::QueryMatch match(m_runnermg->searchContext()->match(matchId));

            // we should verify some other saved information to avoid putting the
            // wrong item if the search result is different!

            add(match, query);
        }
    }
}

bool StripWidget::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::GraphicsSceneHoverEnter) {
        Plasma::IconWidget *icon = qobject_cast<Plasma::IconWidget *>(watched);
        if (icon) {
            m_hoverIndicator->setTargetItem(icon);
        }
    //FIXME: we probably need a specialized widget instead this ugly filter code
    } else if (watched == m_scrollingWidget && event->type() == QEvent::GraphicsSceneResize) {
        QGraphicsSceneResizeEvent *re = static_cast<QGraphicsSceneResizeEvent *>(event);

        bool wider = (re->newSize().width() > m_scrollWidget->size().width());
        if (wider) {
             m_leftArrow->setEnabled(m_scrollingWidget->pos().x() < 0);
             m_rightArrow->setEnabled(m_scrollingWidget->geometry().right() > m_scrollWidget->size().width());
        }
    } else if (watched == m_scrollingWidget && event->type() == QEvent::GraphicsSceneMove) {
        QGraphicsSceneMoveEvent *me = static_cast<QGraphicsSceneMoveEvent *>(event);

        bool wider = (m_scrollingWidget->size().width() > m_scrollWidget->size().width());
        if (wider) {
             m_leftArrow->setEnabled(me->newPos().x() < 0);
             m_rightArrow->setEnabled(m_scrollingWidget->size().width()+me->newPos().x() > m_scrollWidget->size().width());
        }
    }

    return false;
}

void StripWidget::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Left: {
        m_currentIconIndex = (m_stripLayout->count() + m_currentIconIndex - 1) % m_stripLayout->count();
        m_currentIcon = static_cast<Plasma::IconWidget *>(m_stripLayout->itemAt(m_currentIconIndex));
        m_hoverIndicator->setTargetItem(m_currentIcon);
        break;
    }
    case Qt::Key_Right: {
        m_currentIconIndex = (m_currentIconIndex + 1) % m_stripLayout->count();
        m_currentIcon = static_cast<Plasma::IconWidget *>(m_stripLayout->itemAt(m_currentIconIndex));
        m_hoverIndicator->setTargetItem(m_currentIcon);
        break;
    }
    case Qt::Key_Enter:
    case Qt::Key_Return:
        launchFavourite(m_currentIcon);
    default:
        break;
    }
}

void StripWidget::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    Q_UNUSED(event)
    if (!hasFocus()) {
        m_hoverIndicator->hide();
    }
}

void StripWidget::focusInEvent(QFocusEvent *event)
{
    Q_UNUSED(event)
    Plasma::IconWidget *icon = static_cast<Plasma::IconWidget*>(m_stripLayout->itemAt(0));
    m_hoverIndicator->setTargetItem(icon);
    m_currentIconIndex = 0;
}

void StripWidget::focusOutEvent(QFocusEvent *event)
{
    Q_UNUSED(event)
    m_hoverIndicator->hide();
    m_currentIconIndex = -1;
}

void StripWidget::resizeEvent(QGraphicsSceneResizeEvent *event)
{
    if (!m_scrollWidget) {
        return;
    }

    bool wider = (m_scrollWidget->size().width() > m_scrollWidget->size().width());
    if (wider) {
            m_leftArrow->setEnabled(m_scrollingWidget->pos().x() < 0);
            m_rightArrow->setEnabled(m_scrollingWidget->geometry().right() > m_scrollWidget->size().width());
    }
    Plasma::Frame::resizeEvent(event);
}
