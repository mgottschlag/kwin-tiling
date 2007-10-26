/*
*   Copyright 2007 by Aaron Seigo <aseigo@kde.org>
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

#include "desktop.h"

#include <QAction>
#include <QPainter>
#include <QTimeLine>

#include <KAuthorized>
#include <KDebug>
#include <KRun>
#include <KWindowSystem>

#include "plasma/appletbrowser.h"
#include "plasma/phase.h"
#include "plasma/widgets/pushbutton.h"
#include "workspace/kworkspace.h"

#include "krunner_interface.h"
#include "ksmserver_interface.h"
#include "screensaver_interface.h"

using namespace Plasma;
/*
Tool::Tool(QGraphicsItem *parent)
    : QGraphicsItem(parent)
{
}
*/


ToolBox::ToolBox(QGraphicsItem *parent)
    : QGraphicsItem(parent),
      m_icon("configure"),
      m_size(50),
      m_showing(false),
      m_animId(0),
      m_animFrame(0)
{
    setAcceptsHoverEvents(true);
    setZValue(10000);
    setFlag(ItemClipsToShape, true);
    setFlag(ItemClipsChildrenToShape, false);

    connect(Plasma::Phase::self(), SIGNAL(movementComplete(QGraphicsItem*)), this, SLOT(toolMoved(QGraphicsItem*)));
}

/*QRectF ToolBox::sizeHint() const
{
    return boundingRect();
}*/

QRectF ToolBox::boundingRect() const
{
    return QRectF(0, 0, m_size*2, m_size*2);
}

void ToolBox::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

    QPainterPath p = shape();
    QRadialGradient gradient(QPoint(m_size*2, 0), m_size*3);
    gradient.setFocalPoint(QPointF(m_size*2, 0));
    gradient.setColorAt(0, QColor(255, 255, 255, 128));
    gradient.setColorAt(.9, QColor(128, 128, 128, 128));
    painter->save();
    painter->setPen(Qt::NoPen);
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setBrush(gradient);
    painter->drawPath(p);
    painter->restore();
    m_icon.paint(painter, QRect(m_size*2 - 34, 2, 32, 32));
}

QPainterPath ToolBox::shape() const
{
    QPainterPath path;
    int size = m_size + m_animFrame;
    path.moveTo(m_size*2, 0);
    path.arcTo(QRectF(m_size*2 - size, -size, size*2, size*2), 180, 90);
    return path;
}

void ToolBox::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    Q_UNUSED(event)

//    Plasma::Phase::self()->moveItem(this, Phase::SlideIn, QPoint(-25, -25));
    int x = -25; // pos().x();
    int y = 0; // pos().y();
    Plasma::Phase* phase = Plasma::Phase::self();
    foreach (QGraphicsItem* tool, QGraphicsItem::children()) {
//        kDebug() << "let's show and move" << (QObject*)tool << tool->geometry().toRect();
        tool->show();
        phase->moveItem(tool, Plasma::Phase::SlideIn, QPoint(x, y));
        //x += 0;
        y += tool->boundingRect().height() + 5;
    }

    if (m_animId) {
        phase->stopCustomAnimation(m_animId);
    }

    m_showing = true;
    m_animId = phase->customAnimation(m_size, 150, Plasma::Phase::EaseInCurve, this, "animate");
}

void ToolBox::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    Q_UNUSED(event)
//    Plasma::Phase::self->moveItem(this, Phase::SlideOut, boundingRect()QPoint(-50, -50));
    int x = 0; // pos().x() + geometry().width();
    int y = 0;
    Plasma::Phase* phase = Plasma::Phase::self();
    foreach (QGraphicsItem* tool, QGraphicsItem::children()) {
        phase->moveItem(tool, Plasma::Phase::SlideOut, QPoint(x, y));
    }

    if (m_animId) {
        phase->stopCustomAnimation(m_animId);
    }

    m_showing = false;
    m_animId = phase->customAnimation(m_size, 150, Plasma::Phase::EaseOutCurve, this, "animate");
}

void ToolBox::animate(qreal progress)
{
    if (m_showing) {
        m_animFrame = m_size * progress;
    } else {
        m_animFrame = m_size * (1 - progress);
    }

    //kDebug() << "animating at" << progress << "for" << m_animFrame;

    if (progress >= 1) {
        m_animId = 0;
    }

    update();
}

void ToolBox::toolMoved(QGraphicsItem *item)
{
    //kDebug() << "geometry is now " << static_cast<Plasma::Widget*>(item)->geometry();
    if (!m_showing &&
        QGraphicsItem::children().indexOf(static_cast<Plasma::Widget*>(item)) != -1) {
        item->hide();
    }
}

void ToolBox::addTool(Plasma::Widget *tool)
{
    if (!tool) {
        return;
    }

    tool->hide();
    tool->setPos(pos());
    tool->setZValue(zValue() + 1);
    tool->setParentItem(this);
}

DefaultDesktop::DefaultDesktop(QObject *parent, const QVariantList &args)
    : Containment(parent, args),
      m_engineExplorerAction(0),
      m_appletBrowserAction(0),
      m_runCommandAction(0),
      m_lockAction(0),
      m_logoutAction(0),
      m_toolbox(0),
      m_appletBrowser(0)
{
    kDebug() << "!!! loading desktop";
}

DefaultDesktop::~DefaultDesktop()
{
}

void DefaultDesktop::init()
{
    Containment::init();
    m_toolbox = new ToolBox(this);
    //m_toolbox->updateGeometry();
    m_toolbox->setPos(geometry().width() - m_toolbox->boundingRect().width(), 0);

    Plasma::PushButton *tool = new Plasma::PushButton("Add Widgets", this);
    tool->resize(tool->sizeHint());
    m_toolbox->addTool(tool);
    connect(tool, SIGNAL(clicked()), this, SLOT(launchAppletBrowser()));

    tool = new Plasma::PushButton("Engine Explorer", this);
    connect(tool, SIGNAL(clicked()), this, SLOT(launchExplorer()));
    tool->resize(tool->sizeHint());
    m_toolbox->addTool(tool);

    tool = new Plasma::PushButton("Zoom In", this);
    connect(tool, SIGNAL(clicked()), this, SIGNAL(zoomIn()));
    tool->resize(tool->sizeHint());
    m_toolbox->addTool(tool);

    tool = new Plasma::PushButton("Zoom Out", this);
    connect(tool, SIGNAL(clicked()), this, SIGNAL(zoomOut()));
    tool->resize(tool->sizeHint());
    m_toolbox->addTool(tool);
}

void DefaultDesktop::constraintsUpdated(Plasma::Constraints constraints)
{
    if (constraints & Plasma::ScreenConstraint && m_toolbox) {
        m_toolbox->setPos(geometry().width() - m_toolbox->boundingRect().width(), 0);
    }
}

void DefaultDesktop::launchExplorer()
{
    KRun::run("plasmaengineexplorer", KUrl::List(), 0);
}

void DefaultDesktop::launchAppletBrowser()
{
    if (!m_appletBrowser) {
        //TODO: should we delete this after some point, so as to conserve memory
        //      and any possible processing tha tmight end up in AppletBrowser?
        m_appletBrowser = new Plasma::AppletBrowser(this);
    }

    KWindowSystem::setOnDesktop(m_appletBrowser->winId(), KWindowSystem::currentDesktop());
    m_appletBrowser->show();
    KWindowSystem::activateWindow(m_appletBrowser->winId());
}

void DefaultDesktop::runCommand()
{
    if (!KAuthorized::authorizeKAction("run_command")) {
        return;
    }

    QString interface("org.kde.krunner");
    org::kde::krunner::Interface krunner(interface, "/Interface",
                                         QDBusConnection::sessionBus());
    if (krunner.isValid()) {
        krunner.display();
    }
}

void DefaultDesktop::lockScreen()
{
    if (!KAuthorized::authorizeKAction("lock_screen")) {
        return;
    }

    QString interface("org.freedesktop.ScreenSaver");
    org::freedesktop::ScreenSaver screensaver(interface, "/ScreenSaver",
                                              QDBusConnection::sessionBus());
    if (screensaver.isValid()) {
        screensaver.Lock();
    }
}

QList<QAction*> DefaultDesktop::contextActions()
{
    //FIXME: several items here ... probably all junior jobs =)
    //  - pretty up the menu with separators
    //  - should we offer "Switch User" here?

    if (!m_appletBrowserAction) {
        m_engineExplorerAction = new QAction(i18n("Engine Explorer"), this);
        connect(m_engineExplorerAction, SIGNAL(triggered(bool)), this, SLOT(launchExplorer()));

        m_appletBrowserAction = new QAction(i18n("Add Applet..."), this);
        connect(m_appletBrowserAction, SIGNAL(triggered(bool)), this, SLOT(launchAppletBrowser()));

        m_runCommandAction = new QAction(i18n("Run Command..."), this);
        connect(m_runCommandAction, SIGNAL(triggered(bool)), this, SLOT(runCommand()));

        m_lockAction = new QAction(i18n("Lock Screen"), this);
        m_lockAction->setIcon(KIcon("system-lock-screen"));
        connect(m_lockAction, SIGNAL(triggered(bool)), this, SLOT(lockScreen()));

        m_logoutAction = new QAction(i18n("Logout"), this);
        m_logoutAction->setIcon(KIcon("system-log-out"));
        connect(m_logoutAction, SIGNAL(triggered(bool)), this, SLOT(logout()));
    }

    QList<QAction*> actions;

    actions.append(m_engineExplorerAction);
    actions.append(m_appletBrowserAction);

    if (KAuthorized::authorizeKAction("run_command")) {
        actions.append(m_runCommandAction);
    }

    if (KAuthorized::authorizeKAction("lock_screen")) {
        actions.append(m_lockAction);
    }

    if (KAuthorized::authorizeKAction("logout")) {
        actions.append(m_logoutAction);
    }

    return actions;
}

void DefaultDesktop::logout()
{
    if (!KAuthorized::authorizeKAction("logout")) {
        return;
    }

    QString interface("org.kde.ksmserver");
    org::kde::KSMServerInterface smserver(interface, "/KSMServer",
                                          QDBusConnection::sessionBus());
    if (smserver.isValid()) {
        smserver.logout(KWorkSpace::ShutdownConfirmDefault,
                        KWorkSpace::ShutdownTypeDefault,
                        KWorkSpace::ShutdownModeDefault);
    }
}

K_EXPORT_PLASMA_APPLET(desktop, DefaultDesktop)

#include "desktop.moc"
