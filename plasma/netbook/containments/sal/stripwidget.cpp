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

#include <QGraphicsGridLayout>
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
#include <Plasma/ScrollWidget>

#include "griditemview.h"

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
    setEnabledBorders(Plasma::FrameSvg::TopBorder|Plasma::FrameSvg::BottomBorder);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    m_arrowsLayout = new QGraphicsLinearLayout(this);
    setFocusPolicy(Qt::StrongFocus);

    m_leftArrow = new Plasma::ToolButton(this);
    m_leftArrow->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    m_leftArrow->setPreferredWidth(KIconLoader::SizeMedium);
    m_leftArrow->setImage("widgets/arrows", "left-arrow");
    connect(m_leftArrow, SIGNAL(clicked()), this, SLOT(goLeft()));

    m_rightArrow = new Plasma::ToolButton(this);
    m_rightArrow->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    m_rightArrow->setPreferredWidth(KIconLoader::SizeMedium);
    m_rightArrow->setImage("widgets/arrows", "right-arrow");
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

    m_iconsBackground = new GridItemView(this);
    m_iconsBackground->setOrientation(Qt::Horizontal);
    m_iconsBackground->setIconSize(KIconLoader::SizeLarge);
    connect(m_iconsBackground, SIGNAL(itemSelected(Plasma::IconWidget *)), this, SLOT(selectFavourite(Plasma::IconWidget *)));
    connect(m_iconsBackground, SIGNAL(itemActivated(Plasma::IconWidget *)), this, SLOT(launchFavourite(Plasma::IconWidget *)));

    scrollingLayout->addItem(leftSpacer);
    scrollingLayout->addItem(m_iconsBackground);
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
    Q_UNUSED(idx)
    // create new IconWidget for favourite strip

    Plasma::IconWidget *fav = new Plasma::IconWidget(this);
    fav->hide();
    fav->setTextBackgroundColor(QColor());
    fav->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    fav->installEventFilter(this);
    fav->setText(match->text());
    fav->setIcon(match->icon());

    connect(fav, SIGNAL(activated()), this, SLOT(launchFavourite()));

    // set an action to be able to remove from favourites
    QAction *action = new QAction(fav);
    action->setIcon(KIcon("list-remove"));
    fav->addIconAction(action);
    connect(action, SIGNAL(triggered()), this, SLOT(removeFavourite()));

    m_favouritesIcons.insert(fav, match);
    m_iconsBackground->insertItem(fav, -1);
}

void StripWidget::add(Plasma::QueryMatch match, const QString &query)
{
    // add to layout and data structures
    Plasma::QueryMatch *newMatch = new Plasma::QueryMatch(match);
    m_favouritesMatches.append(newMatch);
    m_favouritesQueries.insert(newMatch, query);

    int idx = m_iconsBackground->count();
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
        m_iconsBackground->setMinimumSize(icon->size().width()*(m_iconsBackground->count()-2), icon->size().height());
        m_iconsBackground->setMaximumSize(m_iconsBackground->minimumSize());

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

void StripWidget::selectFavourite(Plasma::IconWidget *icon)
{
    QRectF iconRectToMainWidget = icon->mapToItem(m_scrollingWidget, icon->boundingRect()).boundingRect();

    m_scrollWidget->ensureRectVisible(iconRectToMainWidget);
}

void StripWidget::goRight()
{
    QRectF rect(m_scrollWidget->boundingRect());
    rect.moveLeft(rect.right() - m_scrollingWidget->pos().x());
    rect.setWidth(rect.width()/4);

    m_scrollWidget->ensureRectVisible(rect);
}

void StripWidget::goLeft()
{
    QRectF rect(m_scrollWidget->boundingRect());
    rect.setWidth(rect.width()/4);
    rect.moveRight(- m_scrollingWidget->pos().x());

    m_scrollWidget->ensureRectVisible(rect);
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
        KConfigGroup config(&stripGroup, QString("favourite-%1").arg(id));
        config.writeEntry("runnerid", match->runner()->id());
        config.writeEntry("query", m_favouritesQueries.value(match));
        config.writeEntry("matchId", match->id());
        ++id;
    }
}

void StripWidget::restore(KConfigGroup &cg)
{
    kDebug() << "----------------> Restoring Stuff...";

    KConfigGroup stripGroup(&cg, "stripwidget");

    // get all the favourites
    QList<KConfigGroup> favouritesConfigs;
    foreach (const QString &favouriteGroup, stripGroup.groupList()) {
        if (favouriteGroup.startsWith("favourite-")) {
            KConfigGroup favouriteConfig(&stripGroup, favouriteGroup);
            favouritesConfigs.append(favouriteConfig);
        }
    }

    QVector<QString> runnerIds;
    QVector<QString> queries;
    QVector<QString> matchIds;

    if (favouritesConfigs.isEmpty()) {
        runnerIds.resize(4);
        queries.resize(4);
        matchIds.resize(4);
        matchIds << "services_kde-konqbrowser.desktop" << "services_kde4-KMail.desktop" << "services_kde4-systemsettings.desktop" << "places";
        queries << "konqueror" << "kmail" << "systemsettings" << "home";
        runnerIds << "services" << "services" << "services" << "places";
    } else {
        runnerIds.resize(favouritesConfigs.size());
        queries.resize(favouritesConfigs.size());
        matchIds.resize(favouritesConfigs.size());
        QMutableListIterator<KConfigGroup> it(favouritesConfigs);
        int i = 0;
        while (it.hasNext()) {
            KConfigGroup &favouriteConfig = it.next();

            runnerIds[i] = favouriteConfig.readEntry("runnerid");
            queries[i] = favouriteConfig.readEntry("query");
            matchIds[i] = favouriteConfig.readEntry("matchId");
            ++i;
        }
    }

    QString currentQuery;
    for (int i = 0; i < queries.size(); ++i ) {
        // perform the query
        if (currentQuery == queries[i] || m_runnermg->execQuery(queries[i], runnerIds[i])) {
            currentQuery = queries[i];
            // find our match
            Plasma::QueryMatch match(m_runnermg->searchContext()->match(matchIds[i]));

            // we should verify some other saved information to avoid putting the
            // wrong item if the search result is different!
            if (match.isValid()) {
                add(match, queries[i]);
            }
        }
    }
}

bool StripWidget::eventFilter(QObject *watched, QEvent *event)
{
    Plasma::IconWidget *icon = qobject_cast<Plasma::IconWidget *>(watched);
    if (icon && event->type() == QEvent::GraphicsSceneHoverEnter) {
        if (icon) {
            m_iconsBackground->setCurrentItem(icon);
        }
    //FIXME: we probably need a specialized widget instead this ugly filter code
    } else if (watched == m_scrollingWidget && event->type() == QEvent::GraphicsSceneResize) {
        QGraphicsSceneResizeEvent *re = static_cast<QGraphicsSceneResizeEvent *>(event);

        bool wider = (re->newSize().width() > m_scrollWidget->size().width());
        if (wider) {
             m_leftArrow->setEnabled(m_scrollingWidget->pos().x() < 0);
             m_rightArrow->setEnabled(m_scrollingWidget->geometry().right() > m_scrollWidget->size().width());
        }
        m_scrollWidget->setMinimumHeight(re->newSize().height());
    //pass click only if the user didn't move the mouse FIXME: we need sendevent there
    } else if (icon && event->type() == QEvent::GraphicsSceneMouseMove) {
        QGraphicsSceneMouseEvent *me = static_cast<QGraphicsSceneMouseEvent *>(event);

        QPointF deltaPos = me->pos() - me->lastPos();
        m_scrollingWidget->setPos(qBound(qMin((qreal)0,-m_scrollingWidget->size().width()+m_scrollWidget->size().width()), m_scrollingWidget->pos().x()+deltaPos.x(), (qreal)0),
                                 m_scrollingWidget->pos().y());
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

void StripWidget::focusInEvent(QFocusEvent *event)
{
    Q_UNUSED(event)

    m_iconsBackground->setFocus();
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
