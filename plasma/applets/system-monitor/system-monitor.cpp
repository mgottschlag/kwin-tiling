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
#include <QGraphicsLinearLayout>
#include <KPushButton>
#include <Plasma/Containment>
#include <Plasma/Corona>

SystemMonitor::SystemMonitor(QObject *parent, const QVariantList &args)
    : Plasma::PopupApplet(parent, args), m_layout(0), m_buttons(0), m_widget(0)
{
    resize(234 + 20 + 23, 80 + 20 + 25);
    setMinimumSize(QSize(234 + 20 + 23, 32 + 20 + 25));
    setAspectRatioMode(Plasma::IgnoreAspectRatio);
}

SystemMonitor::~SystemMonitor()
{
}

void SystemMonitor::saveState(KConfigGroup &group) const
{
    QStringList appletNames;
    foreach (SM::Applet *applet, m_applets) {
        applet->saveConfig(group);
        appletNames << applet->objectName();
    }

    group.writeEntry("applets", appletNames);
}

void SystemMonitor::createConfigurationInterface(KConfigDialog *parent)
{
    foreach (Plasma::Applet *applet, m_applets) {
        applet->createConfigurationInterface(parent);
    }
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

    QMap<QString, KPluginInfo> appletsFound;
    KPluginInfo::List appletList = listAppletInfo("System Information");
    foreach (const KPluginInfo &pluginInfo, appletList) {
        if (pluginInfo.pluginName().startsWith("sm_") && !pluginInfo.isHidden()) {
            appletsFound.insert(pluginInfo.pluginName(), pluginInfo);
        }
    }

    foreach (const KPluginInfo &pluginInfo, appletsFound) {
        MonitorButton *button = new MonitorButton(m_widget);
        button->setObjectName(pluginInfo.pluginName());
        button->setCheckable(true);
        button->setImage(pluginInfo.icon());
        if (appletNames.contains(pluginInfo.pluginName())) {
            button->setChecked(true);
        }
        connect(button, SIGNAL(toggled(bool)), this, SLOT(toggled(bool)));
        m_buttons->addItem(button);
        m_monitorButtons << button;
        KGlobal::locale()->insertCatalog(pluginInfo.pluginName());
    }

    m_layout->addItem(m_buttons);
    foreach (const QString& applet, appletNames) {
        if (appletsFound.contains(applet)) {
            addApplet(applet);
        }
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
        SM::Applet * applet = addApplet(sender()->objectName());

        if (applet) {
            Plasma::Constraints constraints(Plasma::ImmutableConstraint |
                                            Plasma::StartupCompletedConstraint);
            applet->updateConstraints(constraints);
            applet->flushPendingConstraintsEvents();
        }
    }
}

SM::Applet *SystemMonitor::addApplet(const QString &name)
{
    if (name.isEmpty()) {
        return 0;
    }

    Plasma::Applet* plasmaApplet = Plasma::Applet::load(name, 0, QVariantList() << "SM");
    SM::Applet* applet = qobject_cast<SM::Applet*>(plasmaApplet);
    if (applet) {
        m_applets.append(applet);
        connect(applet, SIGNAL(geometryChecked()), this, SLOT(checkGeometry()));
        connect(applet, SIGNAL(destroyed(QObject*)), this, SLOT(appletRemoved(QObject*)));
        applet->setFlag(QGraphicsItem::ItemIsMovable, false);
        applet->setBackgroundHints(Plasma::Applet::NoBackground);
        applet->setParentItem(m_widget);
        applet->setObjectName(name);
        connect(applet, SIGNAL(configNeedsSaving()), this, SIGNAL(configNeedsSaving()));
        m_layout->addItem(applet);
        applet->init();

        KConfigGroup cg = config();
        saveState(cg);
        emit configNeedsSaving();
    } else if (plasmaApplet) {
        delete plasmaApplet;
    }

    return applet;
}

void SystemMonitor::removeApplet(const QString &name)
{
    foreach (SM::Applet *applet, m_applets) {
        if (applet->objectName() == name) {
            applet->destroy();
        }
    }
}

void SystemMonitor::appletRemoved(QObject *object)
{
    SM::Applet *applet = static_cast<SM::Applet*>(object);

    foreach (SM::Applet *a, m_applets) {
        if (a == applet) {
            m_layout->removeItem(applet);
            m_applets.removeAll(applet);
            checkGeometry();

            KConfigGroup cg = config();
            saveState(cg);
            emit configNeedsSaving();
        }
    }

    // sanity check the buttons
    QSet<QString> running;
    foreach (SM::Applet *a, m_applets) {
        running << a->objectName();
    }

    foreach (MonitorButton* button, m_monitorButtons) {
        if (!running.contains(button->objectName())) {
            kDebug() << "unchecking" << button->objectName();
            button->setChecked(false);
        }
    }
}

void SystemMonitor::checkGeometry()
{
    QSizeF margins = size() - contentsRect().size();
    qreal minHeight = m_buttons->minimumHeight();
    //kDebug() << minHeight;

    foreach (SM::Applet *applet, m_applets) {
        //kDebug() << applet->minSize() << applet->minimumSize()
        //         << applet->metaObject()->className() << applet->size() - applet->contentsRect().size();
        minHeight += applet->preferredSize().height() + m_layout->spacing();
    }
    m_widget->setMinimumSize(PREFERRED, minHeight);

    QSizeF s(m_widget->size().width(), minHeight);
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

QGraphicsWidget *SystemMonitor::graphicsWidget()
{
    return m_widget;
}

void SystemMonitor::constraintsEvent(Plasma::Constraints constraints)
{
    Plasma::Constraints passOn = Plasma::NoConstraint;

    if (constraints & Plasma::ImmutableConstraint) {
        foreach (MonitorButton* button, m_monitorButtons) {
            button->setEnabled(immutability() == Plasma::Mutable);
        }

        passOn |= Plasma::ImmutableConstraint;
    }

    if (constraints & Plasma::StartupCompletedConstraint) {
        passOn |= Plasma::StartupCompletedConstraint;
    }

    if (passOn != Plasma::NoConstraint) {
        foreach (Plasma::Applet *applet, m_applets) {
            applet->updateConstraints(passOn);
            if (passOn & Plasma::StartupCompletedConstraint) {
                applet->flushPendingConstraintsEvents();
            }
        }
    }

    PopupApplet::constraintsEvent(constraints);
}

#include "system-monitor.moc"
