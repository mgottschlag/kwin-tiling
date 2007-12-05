/*  
    Copyright 2007 Robert Knight <robertknight@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

// Own
#include "simpleapplet/simpleapplet.h"
#include "simpleapplet/menuview.h"

// Qt
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsView>
#include <QtDebug>

// KDE
#include <KIcon>

// Plasma
#include <plasma/layouts/boxlayout.h>
#include <plasma/widgets/icon.h>
#include <plasma/containment.h>

// Local
#include "core/applicationmodel.h"

MenuLauncherApplet::MenuLauncherApplet(QObject *parent, const QVariantList &args)
    : Plasma::Applet(parent,args),
      m_menuview(0)
{
    Plasma::HBoxLayout *layout = new Plasma::HBoxLayout(this);
    layout->setMargin(0);
    m_icon = new Plasma::Icon(KIcon("start-here"), QString(), this);
    m_icon->setFlag(ItemIsMovable, false);
    connect(m_icon, SIGNAL(pressed(bool, QGraphicsSceneMouseEvent*)), this, SLOT(toggleMenu(bool,QGraphicsSceneMouseEvent*)));
}

MenuLauncherApplet::~MenuLauncherApplet()
{
    delete m_menuview;
}

QSizeF MenuLauncherApplet::contentSizeHint() const
{
    return QSizeF(48,48);
}

Qt::Orientations MenuLauncherApplet::expandingDirections() const
{
    return 0;
}

void MenuLauncherApplet::toggleMenu(bool pressed, QGraphicsSceneMouseEvent *event)
{
    if (!pressed) {
        return;
    }

    if (!m_menuview) {
        m_menuview = new Kickoff::MenuView();
        ApplicationModel *appModel = new ApplicationModel(m_menuview);
        appModel->setDuplicatePolicy(ApplicationModel::ShowLatestOnlyPolicy);
        m_menuview->setModel(appModel);
    }

    QPointF scenePos = mapToScene(boundingRect().topLeft());
    QWidget *viewWidget = event->widget() ? event->widget()->parentWidget() : 0;
    QGraphicsView *view = qobject_cast<QGraphicsView*>(viewWidget);
    QPoint globalPos;
    if (view) {
        QPoint viewPos = view->mapFromScene(scenePos);
        globalPos = view->mapToGlobal(viewPos);
        globalPos.ry() -= m_menuview->sizeHint().height();
    }

    m_menuview->setAttribute(Qt::WA_DeleteOnClose);
    m_menuview->popup(globalPos);
}

#include "simpleapplet.moc"
