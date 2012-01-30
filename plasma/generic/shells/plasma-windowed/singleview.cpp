/*
 *   Copyright 2007-2008 Aaron Seigo <aseigo@kde.org>
 *   Copyright 2009 Marco Martin <notmart@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2 as
 *   published by the Free Software Foundation
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

#include "singleview.h"

#include <QAction>
#include <QDir>
#include <QFileInfo>

#include <KDebug>
#include <KStandardAction>
#include <KAction>
#include <KIconLoader>


#include <Plasma/Applet>
#include <Plasma/Containment>
#include <Plasma/Corona>
#include <Plasma/PopupApplet>
#include <Plasma/WindowEffects>

SingleView::SingleView(Plasma::Corona *corona, Plasma::Containment *containment, const QString &pluginName, int appletId, const QVariantList &appletArgs, QWidget *parent)
    : QGraphicsView(parent),
      m_applet(0),
      m_containment(containment),
      m_corona(corona)
{
    setScene(m_corona);
    QFileInfo info(pluginName);
    if (!info.isAbsolute()) {
        info = QFileInfo(QDir::currentPath() + '/' + pluginName);
    }

    if (info.exists()) {
        m_applet = Plasma::Applet::loadPlasmoid(info.absoluteFilePath(), appletId, appletArgs);
    }

    if (!m_applet) {
        m_applet = Plasma::Applet::load(pluginName, appletId, appletArgs);
    }

    if (!m_applet) {
        kDebug() << "failed to load" << pluginName;
        return;
    }

    m_containment->addApplet(m_applet, QPointF(-1, -1), false);
    m_containment->resize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);

    m_applet->setPos(0, 0);
    m_applet->setFlag(QGraphicsItem::ItemIsMovable, false);
    setSceneRect(m_applet->sceneBoundingRect());
    setWindowTitle(m_applet->name());
    setWindowIcon(SmallIcon(m_applet->icon()));
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setFrameStyle(QFrame::NoFrame);

    QAction *action = m_applet->action("remove");
    delete action;

    m_applet->addAction(QString("remove"), KStandardAction::quit(this, SLOT(hide()), m_applet));
    // enforce the applet being our size
    connect(m_applet, SIGNAL(geometryChanged()), this, SLOT(updateGeometry()));
}

SingleView::~SingleView()
{
    m_containment->destroy(false);
}


void SingleView::setContainment(Plasma::Containment *c)
{
    if (m_containment) {
        disconnect(m_containment, 0, this, 0);
    }

    m_containment = c;
    updateGeometry();
}


void SingleView::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event)
    updateGeometry();
    emit geometryChanged();
}

void SingleView::closeEvent(QCloseEvent *event)
{
    if (m_applet) {
        KConfigGroup dummy;
        m_containment->save(dummy);
        emit storeApplet(m_applet);
        m_applet = 0;
    }

    QGraphicsView::closeEvent(event);
    deleteLater();
}

Plasma::Applet *SingleView::applet()
{
    return m_applet;
}

Plasma::Location SingleView::location() const
{
    return m_containment->location();
}

Plasma::FormFactor SingleView::formFactor() const
{
    return m_containment->formFactor();
}

void SingleView::updateGeometry()
{
    if (!m_containment) {
        return;
    }

    //kDebug() << "New applet geometry is" << m_applet->geometry();

    if (m_applet) {
        if (m_applet->size().toSize() != size()) {
            m_applet->resize(size());
        }

        setSceneRect(m_applet->sceneBoundingRect());
    }

    if ((windowFlags() & Qt::FramelessWindowHint) &&
            applet()->backgroundHints() != Plasma::Applet::NoBackground) {

        // TODO: Use the background's mask for blur
        QRegion mask;
        mask += QRect(QPoint(), size());

        Plasma::WindowEffects::enableBlurBehind(winId(), true, mask);
    }
}

#include "singleview.moc"

