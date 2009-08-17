/*
  Copyright (c) 2009 Chani Armitage <chani@kde.org>

  insert GPL blurb here
*/

#include "mousepluginwidget.h"

#include <KDebug>

MousePluginWidget::MousePluginWidget(const KPluginInfo &plugin, QWidget *parent)
    :QWidget(parent),
    m_plugin(plugin)
{
    m_ui.setupUi(this);

    //read plugin data
    m_ui.name->setText(plugin.name());
    m_ui.description->setText(plugin.comment());

    //prettiness
    m_ui.aboutButton->setIcon(KIcon("dialog-information"));
    m_ui.inputButton->setIcon(KIcon("configure"));
    m_ui.configButton->setIcon(KIcon("configure"));

    //FIXME rtl: the button position is probably all wrong
    if (qApp->isLeftToRight()) {
        m_ui.clearButton->setIcon(KIcon("edit-clear-locationbar-rtl"));
    } else {
        m_ui.clearButton->setIcon(KIcon("edit-clear-locationbar-ltr"));
    }

    //connect
    connect(m_ui.inputButton, SIGNAL(triggerChanged(QString,QString)), this, SLOT(changeTrigger(QString,QString)));
    connect(m_ui.clearButton, SIGNAL(clicked()), this, SLOT(clearTrigger()));
}

void MousePluginWidget::setTrigger(const QString &trigger)
{
    m_ui.inputButton->setTrigger(trigger);
}

void MousePluginWidget::clearTrigger()
{
    QString oldTrigger = m_ui.inputButton->trigger();
    m_ui.inputButton->setTrigger(QString());
    m_ui.configButton->setEnabled(false);
    emit triggerChanged(m_plugin.pluginName(), oldTrigger, QString());
}

void MousePluginWidget::changeTrigger(const QString &oldTrigger, const QString& newTrigger)
{
    emit triggerChanged(m_plugin.pluginName(), oldTrigger, newTrigger);
}


// vim: sw=4 sts=4 et tw=100
