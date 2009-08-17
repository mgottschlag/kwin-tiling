/*
  Copyright (c) 2009 Chani Armitage <chani@kde.org>

  insert GPL blurb here
*/

#include "contextactions.h"

#include <KConfigDialog>
#include <KDebug>
#include <KMessageBox>

#include <Plasma/Containment>
#include <plasma/contextaction.h>

ContextActions::ContextActions(Plasma::Containment *containment, KConfigDialog *parent)
    :QWidget(parent),
    m_mouseDlg(0),
    m_containment(containment)
{
    m_ui.setupUi(this);

    reloadConfig();

    KPluginInfo::List plugins = Plasma::ContextAction::listContextActionInfo();
    foreach (const KPluginInfo& info, plugins) {
        QListWidgetItem *item = new QListWidgetItem(m_ui.pluginList);
        QString text = info.name();
        QString trigger = m_plugins.key(info.pluginName());
        if (! trigger.isEmpty()) {
            text += " (" + trigger + ")";
        }
        item->setText(text);
        item->setData(Qt::UserRole, info.pluginName());
    }

    connect(m_ui.pluginList, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)), this, SLOT(currentItemChanged(QListWidgetItem*)));
    connect(m_ui.triggerButton, SIGNAL(clicked()), this, SLOT(getMouseInput()));

    parent->addPage(this, i18n("Mouse Plugins"), "contextactions");

    connect(parent, SIGNAL(applyClicked()), this, SLOT(configAccepted()));
    connect(parent, SIGNAL(okClicked()), this, SLOT(configAccepted()));
}

ContextActions::~ContextActions()
{
}

void ContextActions::settingsChanged(bool isModified)
{
    kDebug() << "#############################################";
    emit modified(isModified);
}

void ContextActions::configAccepted()
{
    foreach (const QString &trigger, m_modifiedKeys) {
        m_containment->setContextAction(trigger, m_plugins.value(trigger));
    }
    m_modifiedKeys.clear();
}

void ContextActions::containmentChanged(Plasma::Containment *c)
{
    if (c && m_containment != c) {
        m_containment=c;
        reloadConfig();
    }
}

void ContextActions::reloadConfig()
{
    if (! m_containment) {
        return;
    }
    m_modifiedKeys.clear();
    m_plugins.clear();

    foreach (const QString &key, m_containment->contextActionTriggers()) {
        Plasma::ContextAction *ca = m_containment->contextAction(key);
        if (ca) {
            m_plugins.insert(key, ca->pluginName());
        }
    }
}

void ContextActions::currentItemChanged(QListWidgetItem *current)
{
    QString plugin = current->data(Qt::UserRole).toString();
    QString trigger = m_plugins.key(plugin); //FIXME linear time
    m_ui.triggerButton->setText(trigger);
}

void ContextActions::getMouseInput()
{
    if (!m_mouseDlg) {
        m_mouseDlg = new QDialog(this);
        m_mouseUi.setupUi(m_mouseDlg);
        m_mouseUi.inputLabel->installEventFilter(this);
    }
    m_mouseDlg->exec();
}

bool ContextActions::eventFilter(QObject *watched, QEvent *event)
{
    if (watched != m_mouseUi.inputLabel) {
        kDebug() << "can't happen";
        return false;
    }

    switch (event->type()) {
        case QEvent::MouseButtonRelease:
        case QEvent::Wheel:
            setTrigger(Plasma::Containment::eventToString(event));
            m_mouseDlg->accept();
        case QEvent::MouseButtonPress:
            event->accept();
            return true;
        default:
            return false;
    }

}

void ContextActions::setTrigger(const QString &newTrigger)
{
    QListWidgetItem *current = m_ui.pluginList->currentItem();
    QString plugin = current->data(Qt::UserRole).toString();
    QString oldTrigger = m_plugins.key(plugin); //FIXME linear time

    if (newTrigger == oldTrigger) {
        return;
    }

    if (m_plugins.contains(newTrigger)) {
        int ret = KMessageBox::warningContinueCancel(this,
                i18n("This trigger is assigned to another plugin."), QString(), KGuiItem(i18n("Reassign")));
        if (ret != KMessageBox::Continue) {
            return;
        }
    }

    m_plugins.remove(oldTrigger);
    m_plugins.insert(newTrigger, plugin);
    m_modifiedKeys << newTrigger << oldTrigger;

    //FIXME update list
    m_ui.triggerButton->setText(newTrigger);
    emit modified(true);
}


//FIXME containment can be changed under us by BackgroundDialog
//and to complicate things, it could happen just after or just before we save?

