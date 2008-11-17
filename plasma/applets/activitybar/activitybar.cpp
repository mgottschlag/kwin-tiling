/***************************************************************************
 *   Copyright (C) 2008 by Marco Martin <notmart@gmail.com>                *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#include "activitybar.h"

#include <QGraphicsLinearLayout>

#include <KWindowSystem>

#include <Plasma/View>
#include <Plasma/Corona>
#include <Plasma/Context>
#include <Plasma/Containment>
#include <Plasma/TabBar>


ActivityBar::ActivityBar(QObject *parent, const QVariantList &args)
    : Plasma::Applet(parent, args),
      m_activeContainment(-1)
{
    resize(200, 60);
    setAspectRatioMode(Plasma::IgnoreAspectRatio);
    setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding));
}


ActivityBar::~ActivityBar()
{
}

void ActivityBar::init()
{
    QGraphicsLinearLayout *layout = new QGraphicsLinearLayout(this);
    m_tabBar = new Plasma::TabBar(this);
    layout->addItem(m_tabBar);
    layout->setContentsMargins(0,0,0,0);
    layout->setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding));

    if (containment()) {
        Plasma::Corona *c = containment()->corona();

        if (!c) {
            kDebug() << "No corona, can't happen";
            setFailedToLaunch(true);
	    return ;
        }

        int myScreen = containment()->screen();

        QList<Plasma::Containment*> containments = c->containments();
        foreach (Plasma::Containment *cont, containments) {
            if (cont->containmentType() == Plasma::Containment::PanelContainment) {
                continue;
            }

            m_containments.append(cont);

            if (cont->activity().isNull()) {
                m_tabBar->addTab(cont->name());
            } else {
                m_tabBar->addTab(cont->activity());
            }

            if (cont->screen() != -1 &&
                cont->screen() == myScreen &&
                (cont->desktop() == -1 || cont->desktop() == KWindowSystem::currentDesktop()-1)) {
                m_view = qobject_cast<Plasma::View *>(cont->view());
                m_activeContainment = m_containments.count() - 1;
                m_tabBar->setCurrentIndex(m_activeContainment);
            }

            connect(cont, SIGNAL(destroyed(QObject *)), this, SLOT(containmentDestroyed(QObject *)));
            connect(cont, SIGNAL(screenChanged(int, int, Plasma::Containment *)), this, SLOT(screenChanged(int, int, Plasma::Containment *)));
            connect(cont, SIGNAL(contextChanged(Plasma::Context *)), this, SLOT(contextChanged(Plasma::Context *)));
        }

        connect(c, SIGNAL(containmentAdded(Plasma::Containment *)), this, SLOT(containmentAdded(Plasma::Containment *)));
    }

    connect(m_tabBar, SIGNAL(currentChanged(int)), this, SLOT(switchContainment(int)));
    connect(KWindowSystem::self(), SIGNAL(currentDesktopChanged(int)), this, SLOT(currentDesktopChanged(int)));

    setPreferredSize(m_tabBar->nativeWidget()->sizeHint());
    setMinimumSize(m_tabBar->nativeWidget()->minimumSizeHint() + (size() - contentsRect().size()));
    //it seems setMinimumSize doesn't resize, weird...
    resize(qMax(size().width(), minimumSize().width()), qMax(size().height(), minimumSize().height()));
    emit sizeHintChanged(Qt::PreferredSize);
}

void ActivityBar::constraintsEvent(Plasma::Constraints constraints)
{
    if (constraints & Plasma::FormFactorConstraint) {
        if (formFactor() == Plasma::Vertical) {
            m_tabBar->nativeWidget()->setShape(QTabBar::RoundedWest);
            setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
        } else {
            m_tabBar->nativeWidget()->setShape(QTabBar::RoundedNorth);
            setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        }

        setPreferredSize(m_tabBar->nativeWidget()->sizeHint());
        emit sizeHintChanged(Qt::PreferredSize);
    }
}

void ActivityBar::switchContainment(int newActive)
{
    if (newActive == m_activeContainment) {
        return;
    }

    //FIXME: this whole thing sounds like an hack isn't it?
    if (!m_view) {
        m_view = qobject_cast<Plasma::View *>(m_containments[m_activeContainment]->view());
    }

    if (m_view) {
        m_activeContainment = newActive;
        Plasma::Containment *cont = m_containments[newActive];

        m_view->setContainment(cont);
    }
}

void ActivityBar::currentDesktopChanged(const int currentDesktop)
{
    Plasma::Corona *c = containment()->corona();
    if (!c) {
        return;
    }

    //-1 because kwindowsystem counts desktop from 1 :)
    Plasma::Containment *cont = c->containmentForScreen(containment()->screen(), currentDesktop - 1);

    if (!cont) {
        return;
    }

    int index = m_containments.indexOf(cont);

    if (index != -1 &&
        index != m_activeContainment) {
        m_activeContainment = index;
        m_tabBar->setCurrentIndex(index);
        m_view = qobject_cast<Plasma::View *>(cont->view());
    }
}

void ActivityBar::containmentAdded(Plasma::Containment *cont)
{
    if (cont->containmentType() == Plasma::Containment::PanelContainment || m_containments.contains(cont)) {
        return;
    }

    m_containments.append(cont);
    if (cont->activity().isNull()) {
        m_tabBar->addTab(cont->name());
    } else {
        m_tabBar->addTab(cont->activity());
    }

    connect(cont, SIGNAL(destroyed(QObject *)), this, SLOT(containmentDestroyed(QObject *)));
    connect(cont, SIGNAL(screenChanged(int, int, Plasma::Containment *)), this, SLOT(screenChanged(int, int, Plasma::Containment *)));
    connect(cont, SIGNAL(contextChanged(Plasma::Context *)), this, SLOT(contextChanged(Plasma::Context *)));

    setPreferredSize(m_tabBar->nativeWidget()->sizeHint());
    emit sizeHintChanged(Qt::PreferredSize);
}

void ActivityBar::containmentDestroyed(QObject *obj)
{
    Plasma::Containment *containment = static_cast<Plasma::Containment *>(obj);

    int index = m_containments.indexOf(containment);
    if (index != -1) {
        m_containments.removeAt(index);
        m_tabBar->removeTab(index);
 
        if (index < m_activeContainment) {
            --m_activeContainment;
        }
    }

    setPreferredSize(m_tabBar->nativeWidget()->sizeHint());
    emit sizeHintChanged(Qt::PreferredSize);
}

void ActivityBar::screenChanged(int wasScreen, int isScreen, Plasma::Containment *cont)
{
    Q_UNUSED(wasScreen)
    //Q_UNUSED(isScreen)

    int index = m_containments.indexOf(cont);

    //FIXME: how is supposed to work containment()->desktop() when the pervirtialthing is off?
    if (index != -1 &&
        index != m_activeContainment &&
        containment()->screen() == isScreen &&
        (cont->desktop() == -1 || cont->desktop() == KWindowSystem::currentDesktop()-1)) {
        m_activeContainment = index;
        m_tabBar->setCurrentIndex(index);
        m_view = qobject_cast<Plasma::View *>(cont->view());
    }
}

void ActivityBar::contextChanged(Plasma::Context *context)
{
    Plasma::Containment *cont = qobject_cast<Plasma::Containment *>(sender());

    if (!cont) {
        return;
    }

    int index = m_containments.indexOf(cont);
    if (index != -1) {
        m_tabBar->setTabText(index, context->currentActivity());
    }
}

#include "activitybar.moc"
