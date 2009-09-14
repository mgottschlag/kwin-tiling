/*
 *   Copyright (c) 2009 Chani Armitage <chani@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
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

#include "mouseplugins.h"
#include "mousepluginwidget.h"

#include <KConfigDialog>
#include <KDebug>
#include <KMessageBox>

#include <Plasma/Containment>
#include <plasma/containmentactions.h>

MousePlugins::MousePlugins(Plasma::Containment *containment, KConfigDialog *parent)
    :QWidget(parent),
    m_containment(containment)
{
    m_ui.setupUi(this);

    Q_ASSERT(m_containment);
    foreach (const QString &key, m_containment->containmentActionsTriggers()) {
        QString plugin = m_containment->containmentActions(key);
        if (!plugin.isEmpty()) {
            m_plugins.insert(key, plugin);
        }
    }

    //can't seem to do anything to pluginList in designer
    QVBoxLayout *lay = new QVBoxLayout(m_ui.pluginList);

    KPluginInfo::List plugins = Plasma::ContainmentActions::listContainmentActionsInfo();
    foreach (const KPluginInfo& info, plugins) {
        QString trigger = m_plugins.key(info.pluginName());
        MousePluginWidget *item = new MousePluginWidget(info, trigger);
        lay->addWidget(item);
        item->setObjectName(info.pluginName());
        item->setContainment(m_containment);
        connect(parent, SIGNAL(containmentPluginChanged(Plasma::Containment*)), item, SLOT(setContainment(Plasma::Containment*)));
        connect(item, SIGNAL(triggerChanged(QString,QString,QString)), this, SLOT(setTrigger(QString,QString,QString)));
        connect(item, SIGNAL(configChanged(QString)), this, SLOT(configChanged(QString)));
        connect(this, SIGNAL(aboutToSave()), item, SLOT(prepareForSave()));
        connect(this, SIGNAL(save()), item, SLOT(save()));
    }

    connect(parent, SIGNAL(applyClicked()), this, SLOT(configAccepted()));
    connect(parent, SIGNAL(okClicked()), this, SLOT(configAccepted()));
    connect(parent, SIGNAL(containmentPluginChanged(Plasma::Containment*)), this, SLOT(containmentPluginChanged(Plasma::Containment*)));
}

MousePlugins::~MousePlugins()
{
}

void MousePlugins::configChanged(const QString &trigger)
{
    m_modifiedKeys << trigger;
    emit modified(true);
}

void MousePlugins::configAccepted()
{
    //FIXME there has *got* to be a more efficient way...
    emit aboutToSave();

    KConfigGroup baseCfg = m_containment->config();
    baseCfg = KConfigGroup(&baseCfg, "ActionPlugins");
    foreach (const QString &trigger, m_modifiedKeys) {
        KConfigGroup cfg = KConfigGroup(&baseCfg, trigger);
        cfg.deleteGroup();
    }

    emit save();

    foreach (const QString &trigger, m_modifiedKeys) {
        m_containment->setContainmentActions(trigger, m_plugins.value(trigger));
    }
    m_modifiedKeys.clear();
}

void MousePlugins::setTrigger(const QString &plugin, const QString &oldTrigger, const QString &newTrigger)
{
    if (newTrigger == oldTrigger) {
        return;
    }

    if (!newTrigger.isEmpty() && m_plugins.contains(newTrigger)) {
        int ret = KMessageBox::warningContinueCancel(this,
                i18n("This trigger is assigned to another plugin."), QString(), KGuiItem(i18n("Reassign")));
        if (ret == KMessageBox::Continue) {
            //clear it from the UI
            MousePluginWidget *w = findChild<MousePluginWidget*>(m_plugins.value(newTrigger));
            Q_ASSERT(w);
            w->setTrigger(QString());
        } else {
            //undo
            MousePluginWidget *w = qobject_cast<MousePluginWidget*>(sender());
            Q_ASSERT(w);
            w->setTrigger(oldTrigger);
            return;
        }
    }

    if (!oldTrigger.isEmpty()) {
        m_plugins.remove(oldTrigger);
        m_modifiedKeys << oldTrigger;
    }
    if (!newTrigger.isEmpty()) {
        m_plugins.insert(newTrigger, plugin);
        m_modifiedKeys << newTrigger;
    }

    emit modified(true);
}


//FIXME this function assumes only the containment plugin is changing, not the config.
//so, no switching activities.
//if the config location *was* changed we'd have to figure out what to do with unsaved changes!
void MousePlugins::containmentPluginChanged(Plasma::Containment *c)
{
    kDebug() << "!!!!!!!!!";
    Q_ASSERT(c);
    m_containment=c;
}

