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
#include <Plasma/Frame>
#include <Plasma/SvgWidget>

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

    connect(this, SIGNAL(toolBoxVisibilityChanged(bool)), this, SLOT(updateConfigurationMode(bool)));
}

Newspaper::~Newspaper()
{
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


    Plasma::Svg *borderSvg = new Plasma::Svg(this);
    borderSvg->setImagePath("newspaper/border");

    m_topBorder = new Plasma::SvgWidget(this);
    m_topBorder->setSvg(borderSvg);
    m_topBorder->setElementID("top");
    m_topBorder->resize(m_topBorder->effectiveSizeHint(Qt::PreferredSize));
    m_topBorder->setZValue(900);
    m_topBorder->show();

    m_bottomBorder = new Plasma::SvgWidget(this);
    m_bottomBorder->setSvg(borderSvg);
    m_bottomBorder->setElementID("bottom");
    m_bottomBorder->resize(m_bottomBorder->effectiveSizeHint(Qt::PreferredSize));
    m_bottomBorder->setZValue(900);
    m_bottomBorder->show();


    QGraphicsWidget *spacer1 = new QGraphicsWidget(m_mainWidget);
    spacer1->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    QGraphicsWidget *spacer2 = new QGraphicsWidget(m_mainWidget);
    spacer2->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_leftLayout->addItem(spacer1);
    m_rightLayout->addItem(spacer2);

    m_background = new Plasma::FrameSvg(this);
    m_background->setImagePath("widgets/translucentbackground");

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
    updateSize();
    //applet->setBackgroundHints(NoBackground);
}

void Newspaper::updateSize()
{
    QSizeF hint = m_mainWidget->effectiveSizeHint(Qt::PreferredSize);
    if (m_orientation == Qt::Horizontal) {
        const qreal proposedWidth = hint.width();
        hint.scale(QWIDGETSIZE_MAX, m_mainWidget->size().height(), Qt::KeepAspectRatio);
        hint.setWidth(qMax(proposedWidth, hint.width()));
        m_mainWidget->resize(hint);
    } else {
        const qreal proposedHeight = hint.height();
        hint.scale(m_mainWidget->size().width(), QWIDGETSIZE_MAX, Qt::KeepAspectRatio);
        hint.setHeight(qMax(hint.height(), proposedHeight));
        m_mainWidget->resize(hint);
    }
}

void Newspaper::constraintsEvent(Plasma::Constraints constraints)
{
    kDebug() << "constraints updated with" << constraints << "!!!!!!";

    if (constraints & Plasma::SizeConstraint && m_appletOverlay) {
        m_appletOverlay->resize(size());
    }

    if (constraints & Plasma::SizeConstraint) {
        //FIXME: remove the hardcoded 2
        m_topBorder->resize(m_scrollWidget->size().width(), m_topBorder->size().height());
        m_topBorder->setPos(m_scrollWidget->pos() + QPoint(0, 2));

        m_bottomBorder->resize(m_scrollWidget->size().width(), m_bottomBorder->size().height());
        m_bottomBorder->setPos(m_scrollWidget->geometry().bottomLeft() - QPoint(0,  m_bottomBorder->size().height() + 2));
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

