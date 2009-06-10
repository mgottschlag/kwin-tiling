/*
*   Copyright 2007 by Alex Merry <alex.merry@kdemail.net>
*   Copyright 2008 by Alexis Ménard <darktears31@gmail.com>
*   Copyright 2008 by Aaron Seigo <aseigo@kde.org>
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

#include "newspaper.h"
#include "appletoverlay.h"

#include <limits>

#include <QApplication>
#include <QGraphicsLinearLayout>
#include <QPainter>
#include <QBitmap>
#include <QDesktopWidget>
#include <QGridLayout>
#include <QLabel>
#include <QComboBox>
#include <QAction>
#include <QGraphicsLayout>


#include <KDebug>
#include <KIcon>
#include <KDialog>
#include <KIntNumInput>
#include <KMessageBox>

#include <Plasma/Corona>
#include <Plasma/FrameSvg>
#include <Plasma/Theme>
#include <Plasma/View>
#include <Plasma/ScrollWidget>
#include <Plasma/PopupApplet>

using namespace Plasma;


Newspaper::Newspaper(QObject *parent, const QVariantList &args)
    : Containment(parent, args),
      m_orientation(Qt::Vertical),
      m_appletOverlay(0)
{
    setContainmentType(Containment::CustomContainment);

    connect(Plasma::Theme::defaultTheme(), SIGNAL(themeChanged()), this, SLOT(themeUpdated()));
    connect(this, SIGNAL(appletAdded(Plasma::Applet*,QPointF)),
            this, SLOT(layoutApplet(Plasma::Applet*,QPointF)));

    //FIXME:eh, actually useless for a custom containment...
    connect(this, SIGNAL(toolBoxVisibilityChanged(bool)), this, SLOT(updateConfigurationMode(bool)));

    //FIXME: this thing is temporary
    QAction *config = new QAction(i18n("configuration mode"), this);
    config->setCheckable(true);
    connect(config, SIGNAL(toggled(bool)), this, SLOT(updateConfigurationMode(bool)));
    m_actions.append(config);
}

Newspaper::~Newspaper()
{
}

//FIXME: this must die
QList<QAction*> Newspaper::contextualActions()
{
    return m_actions;
}

void Newspaper::init()
{
    m_externalLayout = new QGraphicsLinearLayout(this);
    m_scrollWidget = new Plasma::ScrollWidget(this);
    m_externalLayout->addItem(m_scrollWidget);
    m_mainWidget = new QGraphicsWidget(m_scrollWidget);
    m_scrollWidget->setWidget(m_mainWidget);
    m_mainLayout = new QGraphicsLinearLayout(Qt::Horizontal, m_mainWidget);
    m_leftLayout = new QGraphicsLinearLayout(Qt::Vertical);
    m_rightLayout = new QGraphicsLinearLayout(Qt::Vertical);
    m_mainLayout->addItem(m_leftLayout);
    m_mainLayout->addItem(m_rightLayout);

    QGraphicsWidget *spacer1 = new QGraphicsWidget(m_mainWidget);
    spacer1->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    QGraphicsWidget *spacer2 = new QGraphicsWidget(m_mainWidget);
    spacer2->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_leftLayout->addItem(spacer1);
    m_rightLayout->addItem(spacer2);

    m_background = new Plasma::FrameSvg(this);
    m_background->setImagePath("widgets/background");

    Containment::init();
    setHasConfigurationInterface(true);
    themeUpdated();

    QAction *a = action("add widgets");

    if (a) {
        addToolBoxAction(a);
    }
}

void Newspaper::themeUpdated()
{
    qreal left, top, right, bottom;
    m_background->getMargins(left, top, right, bottom);
    m_externalLayout->setContentsMargins(left, top, right, bottom);
}

void Newspaper::layoutApplet(Plasma::Applet* applet, const QPointF &pos)
{
    QGraphicsLinearLayout *lay;

    if (m_orientation == Qt::Horizontal) {
        if (pos.y() >= size().height()/4) {
            lay = m_rightLayout;
        } else {
            lay = m_leftLayout;
        }
    } else {
        if (pos.x() >= size().width()/4) {
            lay = m_rightLayout;
        } else {
            lay = m_leftLayout;
        }
    }

    int insertIndex = -1;

    //if pos is (-1,-1) insert at the end of the Newspaper
    if (pos != QPoint(-1, -1)) {
        for (int i = 0; i < lay->count(); ++i) {
            QRectF siblingGeometry = lay->itemAt(i)->geometry();
            if (m_orientation == Qt::Horizontal) {
                qreal middle = (siblingGeometry.left() + siblingGeometry.right()) / 2.0;
                if (pos.x() < middle) {
                    insertIndex = i;
                    break;
                } else if (pos.x() <= siblingGeometry.right()) {
                    insertIndex = i + 1;
                    break;
                }
            } else { //Vertical
                qreal middle = (siblingGeometry.top() + siblingGeometry.bottom()) / 2.0;
                if (pos.y() < middle) {
                    insertIndex = i;
                    break;
                } else if (pos.y() <= siblingGeometry.bottom()) {
                    insertIndex = i + 1;
                    break;
                }
            }
        }
    }

    if (insertIndex == -1) {
        lay->insertItem(lay->count()-1, applet);
    } else {
        lay->insertItem(qMin(insertIndex, lay->count()-1), applet);
    }


    connect(applet, SIGNAL(sizeHintChanged(Qt::SizeHint)), this, SLOT(updateSize()));
    applet->setBackgroundHints(NoBackground);
}

void Newspaper::updateSize()
{
    if (m_orientation == Qt::Horizontal) {
        m_mainWidget->resize(m_mainWidget->effectiveSizeHint(Qt::PreferredSize).width(), m_mainWidget->size().height());
    } else {
        m_mainWidget->resize(m_mainWidget->size().width(), m_mainWidget->effectiveSizeHint(Qt::PreferredSize).height());
    }
}

void Newspaper::constraintsEvent(Plasma::Constraints constraints)
{
    kDebug() << "constraints updated with" << constraints << "!!!!!!";

    if (constraints & Plasma::FormFactorConstraint ||
        constraints & Plasma::StartupCompletedConstraint) {

        foreach (Applet *applet, applets()) {
            applet->setBackgroundHints(NoBackground);
        }
    }

    if (constraints & Plasma::SizeConstraint && m_appletOverlay) {
        m_appletOverlay->resize(size());
    }
}

void Newspaper::updateConfigurationMode(bool config)
{
    if (config && !m_appletOverlay) {
        m_appletOverlay = new AppletOverlay(this, this);
        m_appletOverlay->resize(size());
    } else if (!config) {
        delete m_appletOverlay;
        m_appletOverlay = 0;
    }
}


void Newspaper::paintInterface(QPainter *painter,
                           const QStyleOptionGraphicsItem *option,
                           const QRect& contentsRect)
{
    Q_UNUSED(contentsRect)

    Containment::paintInterface(painter, option, contentsRect);

    m_background->resizeFrame(contentsRect.size());
    m_background->paintFrame(painter, contentsRect.topLeft());
}


K_EXPORT_PLASMA_APPLET(newspaper, Newspaper)

#include "newspaper.moc"

