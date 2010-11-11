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
#include "mouseinputbutton.h"

#include <KConfigDialog>
#include <KDebug>
#include <KMessageBox>

#include <Plasma/Containment>
#include <plasma/containmentactions.h>

MousePlugins::MousePlugins(Plasma::Containment *containment, KConfigDialog *parent)
    :QWidget(parent),
    m_containment(containment)
{
    Q_ASSERT(m_containment);
    m_ui.setupUi(this);
    m_ui.addButton->setIcon(KIcon("list-add"));
    m_ui.addButton->setDefaultText(i18n("Add Action..."), i18n("Add another mouse action"));
    QGridLayout *lay = qobject_cast<QGridLayout*>(m_ui.pluginList->layout());
    lay->setMargin(0);
    lay->setColumnStretch(1, 1); //make the plugin list take the extra space

    //stupid hack because you can't *insert* rows into a gridlayout
    lay->removeWidget(m_ui.addButton);
    lay->removeItem(m_ui.verticalSpacer);

    foreach (const QString &trigger, m_containment->containmentActionsTriggers()) {
        QString plugin = m_containment->containmentActions(trigger);
        if (!plugin.isEmpty()) {
            MousePluginWidget *item = new MousePluginWidget(plugin, trigger, lay, this);
            item->setContainment(m_containment);
            connect(parent, SIGNAL(containmentPluginChanged(Plasma::Containment*)), item, SLOT(setContainment(Plasma::Containment*)));
            connect(item, SIGNAL(triggerChanged(QString,QString)), this, SLOT(setTrigger(QString,QString)));
            connect(item, SIGNAL(configChanged(QString)), this, SLOT(configChanged(QString)));
            m_plugins.insert(trigger, item); //FIXME what kind of pointer?
        }
    }

    //stupid hack because you can't *insert* rows into a gridlayout
    lay->addWidget(m_ui.addButton, lay->rowCount(), 0);
    lay->addItem(m_ui.verticalSpacer, lay->rowCount(), 0);

    connect(m_ui.addButton, SIGNAL(triggerChanged(QString,QString)), this, SLOT(addTrigger(QString,QString)));
    connect(parent, SIGNAL(applyClicked()), this, SLOT(configAccepted()));
    connect(parent, SIGNAL(okClicked()), this, SLOT(configAccepted()));
    connect(parent, SIGNAL(containmentPluginChanged(Plasma::Containment*)), this, SLOT(containmentPluginChanged(Plasma::Containment*)));
}

MousePlugins::~MousePlugins()
{
}

void MousePlugins::addTrigger(const QString&, const QString &trigger)
{
    m_ui.addButton->reset();

    //check for duplicate
    if (m_plugins.contains(trigger)) {
        int ret = KMessageBox::warningContinueCancel(this,
                i18n("This trigger is already assigned to another action."), QString(), KGuiItem(i18n("Reassign")));
        if (ret == KMessageBox::Continue) {
            //clear it from the UI
            MousePluginWidget *w = m_plugins.value(trigger);
            Q_ASSERT(w);
            w->setTrigger(QString());
        } else {
            //don't add anything
            return;
        }
    }

    QGridLayout *lay = qobject_cast<QGridLayout*>(m_ui.pluginList->layout());

    //stupid hack because you can't *insert* rows into a gridlayout
    lay->removeWidget(m_ui.addButton);
    lay->removeItem(m_ui.verticalSpacer);

    //insert a new row
    MousePluginWidget *item = new MousePluginWidget(QString(), trigger, lay, this);
    item->setContainment(m_containment);
    connect(parent(), SIGNAL(containmentPluginChanged(Plasma::Containment*)), item, SLOT(setContainment(Plasma::Containment*)));
    connect(item, SIGNAL(triggerChanged(QString,QString)), this, SLOT(setTrigger(QString,QString)));
    connect(item, SIGNAL(configChanged(QString)), this, SLOT(configChanged(QString)));
    m_plugins.insert(trigger, item); //FIXME what kind of pointer?
    m_modifiedKeys << trigger;

    //stupid hack because you can't *insert* rows into a gridlayout
    lay->addWidget(m_ui.addButton, lay->rowCount(), 0);
    lay->addItem(m_ui.verticalSpacer, lay->rowCount(), 0);

    emit modified(true);
}

void MousePlugins::configChanged(const QString &trigger)
{
    m_modifiedKeys << trigger;
    emit modified(true);
}

void MousePlugins::configAccepted()
{
    //FIXME there has *got* to be a more efficient way...
    //let them back up their config
    foreach (const QString &trigger, m_modifiedKeys) {
        MousePluginWidget *p = m_plugins.value(trigger);
        if (p) {
            p->prepareForSave();
        }
    }

    KConfigGroup baseCfg = m_containment->containmentActionsConfig();
    foreach (const QString &trigger, m_modifiedKeys) {
        //delete any old config (who knows what it could be from)
        KConfigGroup cfg = KConfigGroup(&baseCfg, trigger);
        cfg.deleteGroup();
        //save the new config
        MousePluginWidget *p = m_plugins.value(trigger);
        if (p) {
            p->save();
        } else {
            m_containment->setContainmentActions(trigger, QString());
        }
    }

    m_modifiedKeys.clear();
}

void MousePlugins::setTrigger(const QString &oldTrigger, const QString &newTrigger)
{
    if (newTrigger == oldTrigger) {
        return;
    }

    MousePluginWidget *plugin = qobject_cast<MousePluginWidget*>(sender());
    Q_ASSERT(plugin);
    if (!newTrigger.isEmpty() && m_plugins.contains(newTrigger)) {
        int ret = KMessageBox::warningContinueCancel(this,
                i18n("This trigger is assigned to another plugin."), QString(), KGuiItem(i18n("Reassign")));
        if (ret == KMessageBox::Continue) {
            //clear it from the UI
            MousePluginWidget *w = m_plugins.value(newTrigger);
            Q_ASSERT(w);
            w->setTrigger(QString());
        } else {
            //undo
            plugin->setTrigger(oldTrigger);
            return;
        }
    }
    //FIXME we can't just remove them now, they're pointers!
    //...or can we? can we get them back when they set a trigger again?
    //er, that plugin string coming in.. that's not what we wanna set any more.
    //tbh we never needed it.. unless there was no old trigger.
    //so... uhm.. yes. we're now always going to yank the sender from this. feels wrong.

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

