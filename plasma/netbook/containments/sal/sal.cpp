/*
 *   Copyright 2009 by Aaron Seigo <aseigo@kde.org>
 *   Copyright 2009 by Artur Duque de Souza <morpheuz@gmail.com>
 *   Copyright 2009 by Marco Martin <notmart@gmail.com>
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

#include "sal.h"
#include "stripwidget.h"

#include <QPainter>
#include <QAction>

#include <KDebug>
#include <KIcon>

#include <Plasma/Theme>
#include <Plasma/Frame>
#include <Plasma/Corona>
#include <Plasma/FrameSvg>
#include <Plasma/LineEdit>
#include <Plasma/IconWidget>
#include <Plasma/RunnerManager>
#include <Plasma/QueryMatch>


SearchLaunch::SearchLaunch(QObject *parent, const QVariantList &args)
    : Containment(parent, args)
{
    setContainmentType(Containment::DesktopContainment);
    setHasConfigurationInterface(false);
}

SearchLaunch::~SearchLaunch()
{
    delete tedit;
    delete runnermg;
}

void SearchLaunch::init()
{
    Containment::init();
    connect(this, SIGNAL(appletAdded(Plasma::Applet*,QPointF)),
            this, SLOT(layoutApplet(Plasma::Applet*,QPointF)));
    connect(this, SIGNAL(appletRemoved(Plasma::Applet*)),
            this, SLOT(appletRemoved(Plasma::Applet*)));

    runnermg = new Plasma::RunnerManager(this);
    connect(runnermg, SIGNAL(matchesChanged(const QList<Plasma::QueryMatch>&)),
            this, SLOT(setQueryMatches(const QList<Plasma::QueryMatch>&)));

    // before this text edit goes to panel, we'll try here
    tedit = new Plasma::LineEdit(this);
    connect(tedit, SIGNAL(returnPressed()), this, SLOT(doSearch()));

    m_background = new Plasma::FrameSvg(this);
    m_background->setImagePath("widgets/translucentbackground");
}

void SearchLaunch::themeUpdated()
{
    qreal left, top, right, bottom;
    m_background->getMargins(left, top, right, bottom);
    m_mainLayout->setContentsMargins(left, top, right, bottom);
}

void SearchLaunch::doSearch()
{
    runnermg->launchQuery(tedit->text());
    queryCounter = 0;

    foreach (Plasma::IconWidget *icon, m_items) {
        delete icon;
    }

    m_items.clear();
    m_matches.clear();
}

void SearchLaunch::setQueryMatches(const QList<Plasma::QueryMatch> &m)
{
    if (m.isEmpty()) {
        return;
    }

    // just add new QueryMatch
    int i;
    for (i = queryCounter; i < m.size(); i++) {
        Plasma::QueryMatch match = m[i];

        // create new IconWidget with information from the match
        Plasma::IconWidget *icon = new Plasma::IconWidget();
        icon->setText(match.text());
        icon->setIcon(match.icon());
        icon->setMinimumSize(QSize(100, 100));
        icon->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        icon->setDrawBackground(true);
        connect(icon, SIGNAL(activated()), this, SLOT(launch()));

        // create action to add to favourites strip
        QAction *action = new QAction(icon);
        action->setIcon(KIcon("favorites"));
        icon->addIconAction(action);
        connect(action, SIGNAL(triggered()), this, SLOT(addFavourite()));

        // add to layout and data structures
        m_items.append(icon);
        m_matches.append(match);
        m_launchGrid->addItem(icon, i / 7, i % 7);
    }
    queryCounter = i;
}

void SearchLaunch::launch()
{
    Plasma::IconWidget *icon = static_cast<Plasma::IconWidget*>(sender());
    int idx = m_items.indexOf(icon);
    Plasma::QueryMatch match = m_matches[idx];
    runnermg->run(match);
}

void SearchLaunch::addFavourite()
{
    Plasma::IconWidget *icon = static_cast<Plasma::IconWidget*>(sender()->parent());
    int idx = m_items.indexOf(icon);
    Plasma::QueryMatch match = m_matches[idx];
    m_stripWidget->add(match);
}

QList<QAction*> SearchLaunch::contextualActions()
{
    QList<QAction*> actions;
    return actions;
}

void SearchLaunch::layoutApplet(Plasma::Applet* applet, const QPointF &pos)
{
    Q_UNUSED(pos);

    // this gets called whenever an applet is added, and we add it to our layout
    QGraphicsLinearLayout *lay = dynamic_cast<QGraphicsLinearLayout*>(layout());

    if (!lay) {
        return;
    }

    lay->addItem(applet);
    applet->setBackgroundHints(NoBackground);
    connect(applet, SIGNAL(sizeHintChanged(Qt::SizeHint)), this, SLOT(updateSize()));
}

void SearchLaunch::appletRemoved(Plasma::Applet* applet)
{
    //shrink the SearchLaunch if possible
    if (formFactor() == Plasma::Horizontal) {
        resize(size().width() - applet->size().width(), size().height());
    } else {
        resize(size().width(), size().height() - applet->size().height());
    }
    layout()->setMaximumSize(size());
}

void SearchLaunch::updateSize()
{
    Plasma::Applet *applet = qobject_cast<Plasma::Applet *>(sender());

    if (applet) {
        if (formFactor() == Plasma::Horizontal) {
            const int delta = applet->preferredWidth() - applet->size().width();
            //setting the preferred width when delta = 0 and preferredWidth() < minimumWidth()
            // leads to the same thing as setPreferredWidth(minimumWidth())
            if (delta != 0) {
                setPreferredWidth(preferredWidth() + delta);
            }
        } else if (formFactor() == Plasma::Vertical) {
            const int delta = applet->preferredHeight() - applet->size().height();
            if (delta != 0) {
                setPreferredHeight(preferredHeight() + delta);
            }
        }

        resize(preferredSize());
    }
}


void SearchLaunch::constraintsEvent(Plasma::Constraints constraints)
{
    kDebug() << "constraints updated with" << constraints << "!!!!!!";

    if (constraints & Plasma::FormFactorConstraint ||
        constraints & Plasma::StartupCompletedConstraint) {

        Plasma::FormFactor form = formFactor();
        Qt::Orientation layoutDirection = form == Plasma::Vertical ? \
            Qt::Vertical : Qt::Horizontal;
        Qt::Orientation layoutOtherDirection = form == Plasma::Vertical ? \
            Qt::Horizontal : Qt::Vertical;

        // create our layout!
        if (!layout()) {
            // create main layout
            m_mainLayout = new QGraphicsLinearLayout();
            m_mainLayout->setOrientation(layoutOtherDirection);
            m_mainLayout->setSpacing(4);
            m_mainLayout->setSizePolicy(QSizePolicy(QSizePolicy::Expanding,
                                                    QSizePolicy::Expanding));
            setLayout(m_mainLayout);

            // create launch grid and make it centered
            QGraphicsLinearLayout *gridLayout = new QGraphicsLinearLayout();
            gridLayout->setSizePolicy(QSizePolicy(QSizePolicy::Expanding,
                                                  QSizePolicy::Expanding));

            Plasma::Frame *gridBackground = new Plasma::Frame();
            gridBackground->setFrameShadow(Plasma::Frame::Sunken);
            gridLayout->addItem(gridBackground);

            m_launchGrid = new QGraphicsGridLayout();
            m_launchGrid->setSizePolicy(QSizePolicy(QSizePolicy::Expanding,
                                                    QSizePolicy::Expanding));
            gridBackground->setLayout(m_launchGrid);

            m_stripWidget = new StripWidget(runnermg, this);

            QGraphicsLinearLayout *m_favourites = new QGraphicsLinearLayout();
            m_favourites->setOrientation(layoutDirection);
            m_favourites->addStretch();
            m_favourites->addStretch();
            m_favourites->insertItem(1, m_stripWidget);

            // add our layouts to main vertical layout
            m_mainLayout->addItem(tedit);
            tedit->setZValue(9999999);
            m_mainLayout->addItem(m_favourites);
            m_mainLayout->addItem(gridLayout);

            // correctly set margins
            themeUpdated();
        }
    }

    if (constraints & Plasma::LocationConstraint) {
        setFormFactorFromLocation(location());
    }

}

void SearchLaunch::paintInterface(QPainter *painter,
                                 const QStyleOptionGraphicsItem *option,
                                 const QRect& contentsRect)
{
    Q_UNUSED(contentsRect)
    Containment::paintInterface(painter, option, contentsRect);
    m_background->resizeFrame(contentsRect.size());
    m_background->paintFrame(painter, contentsRect.topLeft());
}

void SearchLaunch::setFormFactorFromLocation(Plasma::Location loc) {
    switch (loc) {
    case Plasma::BottomEdge:
    case Plasma::TopEdge:
        //kDebug() << "setting horizontal form factor";
        setFormFactor(Plasma::Horizontal);
        break;
    case Plasma::RightEdge:
    case Plasma::LeftEdge:
        //kDebug() << "setting vertical form factor";
        setFormFactor(Plasma::Vertical);
        break;
    case Plasma::Floating:
        setFormFactor(Plasma::Planar);
        kDebug() << "Floating is unimplemented.";
        break;
    default:
        kDebug() << "invalid location!!";
    }
}

K_EXPORT_PLASMA_APPLET(sal, SearchLaunch)

#include "sal.moc"

