/*
 *   Copyright (C) 2007 Petri Damsten <damu@iki.fi>
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

#include "system-monitor.h"
#include "monitorbutton.h"
#include "applet.h"
#include <QTimer>
#include <QGraphicsLinearLayout>
#include <KPushButton>
#include <Plasma/Containment>
#include <Plasma/Corona>

#define APPLETS 5
static const char *sm_applets[][2] = {
    { "media-flash", "sm_temperature" },
    //{ "media-flash", "sm_ram" },
    { "cpu", "sm_cpu" },
    { "hwinfo", "sm_hwinfo" },
    { "network-workgroup", "sm_net" },
    { "drive-harddisk", "sm_hdd" }
};

SystemMonitor::SystemMonitor(QObject *parent, const QVariantList &args)
    : Plasma::PopupApplet(parent, args), m_layout(0), m_buttons(0), m_widget(0)
{
    resize(234 + 20 + 23, 80 + 20 + 25);
    setMinimumSize(QSize(234 + 20 + 23, 32 + 20 + 25));
    setAspectRatioMode(Plasma::IgnoreAspectRatio);
}

SystemMonitor::~SystemMonitor()
{
    QStringList appletNames;
    foreach (Plasma::Applet *applet, m_applets) {
        appletNames << applet->objectName();
        applet->destroy();
    }
    KConfigGroup cg = config();
    cg.writeEntry("applets", appletNames);

}

void SystemMonitor::init()
{
    KConfigGroup cg = config();
    QStringList appletNames = cg.readEntry("applets", QStringList());

    m_widget = new QGraphicsWidget(this);
    m_layout = new QGraphicsLinearLayout(Qt::Vertical);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_buttons = new QGraphicsLinearLayout(Qt::Horizontal);
    m_buttons->setContentsMargins(0, 0, 0, 0);
    m_buttons->setSpacing(5);

    for (int i = 0; i < APPLETS; ++i) {
        MonitorButton *button = new MonitorButton(m_widget);
        button->nativeWidget()->setObjectName(sm_applets[i][1]);
        button->nativeWidget()->setCheckable(true);
        button->setImage(sm_applets[i][0]);
        if (appletNames.contains(sm_applets[i][1])) {
            button->nativeWidget()->setChecked(true);
        }
        connect(button->nativeWidget(), SIGNAL(toggled(bool)), this, SLOT(toggled(bool)));
        m_buttons->addItem(button);
    }
    m_layout->addItem(m_buttons);
    foreach (const QString& applet, appletNames) {
        addApplet(applet);
    }
    m_widget->setLayout(m_layout);
    checkGeometry();

    m_widget->setMinimumSize(QSize(234 + 20 + 23, 32 + 20 + 25));
    setPopupIcon("utilities-system-monitor");
}

void SystemMonitor::toggled(bool toggled)
{
    removeApplet(sender()->objectName());
    if (toggled) {
        addApplet(sender()->objectName());
    }
}

void SystemMonitor::addApplet(const QString &name)
{
    kDebug() << "";
    if (name.isEmpty()) {
        return;
    }
    SM::Applet* applet = qobject_cast<SM::Applet*>(Plasma::Applet::load(name, 0, QVariantList() << "SM"));
    if (applet) {
        m_applets.append(applet);
        connect(applet, SIGNAL(geometryChecked()), this, SLOT(checkGeometry()));
        applet->setFlag(QGraphicsItem::ItemIsMovable, false);
        applet->init();
        applet->setBackgroundHints(Plasma::Applet::NoBackground);
        applet->setParentItem(m_widget);
        applet->setObjectName(name);
        m_layout->addItem(applet);
        //checkGeometry(applet->preferredSize().height());
    }
}

void SystemMonitor::removeApplet(const QString &name)
{
    qreal height = 0;
    foreach (SM::Applet *applet, m_applets) {
        if (applet->objectName() == name) {
            height -= applet->size().height();
            m_layout->removeItem(applet);
            m_applets.removeAll(applet);
            applet->destroy();
        }
    }
    if (height != 0) {
        checkGeometry(height);
    }
}

void SystemMonitor::checkGeometry(qreal height)
{
    QSizeF margins = size() - contentsRect().size();
    qreal minHeight = 32 + 20 + 25; // m_buttons->minimumHeight();
    //kDebug() << minHeight;

    foreach (SM::Applet *applet, m_applets) {
        //kDebug() << applet->minSize() << applet->minimumSize()
        //         << applet->metaObject()->className() << applet->size() - applet->contentsRect().size();
        minHeight += applet->minSize().height() + m_layout->spacing();
    }
    m_widget->setMinimumSize(DEFAULT_MINIMUM_WIDTH, minHeight);

    QSizeF s(m_widget->size().width(), qMax(m_widget->size().height() + height, minHeight));
    if (m_applets.count() == 0) {
        // I want to be sure...
        s.setHeight(minHeight);
    }
    if (formFactor() != Plasma::Horizontal && formFactor() != Plasma::Vertical) {
        setMinimumSize(m_widget->minimumSize() + margins);
    }
    resize(s + margins);
    m_widget->resize(s);
    update();
    /*
    kDebug() << m_widget->size().height() << m_layout->geometry().height();
    foreach (SM::Applet *applet, m_applets) {
        kDebug() << applet->metaObject()->className() << applet->size().height();
    }
    for (int i = 0; i < m_layout->count(); ++i) {
        kDebug() << m_layout->itemAt(i)->geometry().top() << m_layout->itemAt(i)->geometry().height();
    }
    */
}

QList<QAction*> SystemMonitor::contextualActions()
{
    QList<QAction*> result;
    foreach (Plasma::Applet *applet, m_applets) {
        result << applet->action("configure");
    }
    return result;
}

QGraphicsWidget *SystemMonitor::graphicsWidget()
{
    return m_widget;
}

#include "system-monitor.moc"
