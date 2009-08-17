/*
  Copyright (c) 2009 Chani Armitage <chani@kde.org>

  insert GPL blurb here
*/

#include "contextactions.h"
#include "mousepluginwidget.h"

#include <KConfigDialog>
#include <KDebug>
#include <KMessageBox>

#include <Plasma/Containment>
#include <plasma/contextaction.h>

ContextActions::ContextActions(Plasma::Containment *containment, KConfigDialog *parent)
    :QWidget(parent),
    m_containment(containment)
{
    m_ui.setupUi(this);

    Q_ASSERT(m_containment);
    foreach (const QString &key, m_containment->contextActionTriggers()) {
        Plasma::ContextAction *ca = m_containment->contextAction(key);
        if (ca) {
            m_plugins.insert(key, ca->pluginName());
        }
    }

    //can't seem to do anything to pluginList in designer
    QVBoxLayout *lay = new QVBoxLayout(m_ui.pluginList);

    //FIXME this feels wrong
    KConfigGroup cfg(&(m_containment->config()), "ContextActions");

    KPluginInfo::List plugins = Plasma::ContextAction::listContextActionInfo();
    foreach (const KPluginInfo& info, plugins) {
        MousePluginWidget *item = new MousePluginWidget(info);
        lay->addWidget(item);
        item->setObjectName(info.pluginName());
        QString trigger = m_plugins.key(info.pluginName());
        item->setTrigger(trigger);
        //FIXME make a truly unique config group
        item->setConfigGroup(KConfigGroup(&cfg, info.pluginName()));
        item->setContainment(m_containment);
        connect(parent, SIGNAL(containmentPluginChanged(Plasma::Containment*)), item, SLOT(setContainment(Plasma::Containment*)));
        connect(item, SIGNAL(triggerChanged(QString,QString,QString)), this, SLOT(setTrigger(QString,QString,QString)));
        connect(item, SIGNAL(configChanged(QString)), this, SLOT(configChanged(QString)));
        connect(this, SIGNAL(save()), item, SLOT(save()));
    }

    connect(parent, SIGNAL(applyClicked()), this, SLOT(configAccepted()));
    connect(parent, SIGNAL(okClicked()), this, SLOT(configAccepted()));
    connect(parent, SIGNAL(containmentPluginChanged(Plasma::Containment*)), this, SLOT(containmentPluginChanged(Plasma::Containment*)));
}

ContextActions::~ContextActions()
{
}

void ContextActions::configChanged(const QString &trigger)
{
    m_modifiedKeys << trigger;
    emit modified(true);
}

void ContextActions::configAccepted()
{
    //FIXME only save changed configs
    emit save();

    foreach (const QString &trigger, m_modifiedKeys) {
        m_containment->setContextAction(trigger, m_plugins.value(trigger));
    }
    m_modifiedKeys.clear();
}

void ContextActions::setTrigger(const QString &plugin, const QString &oldTrigger, const QString &newTrigger)
{
    if (newTrigger == oldTrigger) {
        return;
    }

    //FIXME!!! config will break
    //do I need to copyTo the new group-name or is there an easier way?

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
void ContextActions::containmentPluginChanged(Plasma::Containment *c)
{
    kDebug() << "!!!!!!!!!";
    Q_ASSERT(c);
    m_containment=c;
}

