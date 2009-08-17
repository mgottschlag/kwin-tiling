/*
  Copyright (c) 2009 Chani Armitage <chani@kde.org>

  insert GPL blurb here
*/

#include "mousepluginwidget.h"

#include <KDebug>

#include <QDialog>
#include <QDialogButtonBox>
#include <QLayout>

MousePluginWidget::MousePluginWidget(const KPluginInfo &plugin, QWidget *parent)
    :QWidget(parent),
    m_plugin(plugin),
    m_pluginInstance(0),
    m_configDlg(0)
{
    m_ui.setupUi(this);

    //read plugin data
    m_ui.name->setText(plugin.name());
    m_ui.description->setText(plugin.comment());

    //prettiness
    m_ui.aboutButton->setIcon(KIcon("dialog-information"));
    m_ui.inputButton->setIcon(KIcon("configure"));
    m_ui.configButton->setIcon(KIcon("configure"));

    //FIXME how do I know whether there's a config interface?

    //FIXME rtl: the button position is probably all wrong
    if (qApp->isLeftToRight()) {
        m_ui.clearButton->setIcon(KIcon("edit-clear-locationbar-rtl"));
    } else {
        m_ui.clearButton->setIcon(KIcon("edit-clear-locationbar-ltr"));
    }

    //connect
    connect(m_ui.inputButton, SIGNAL(triggerChanged(QString,QString)), this, SLOT(changeTrigger(QString,QString)));
    connect(m_ui.configButton, SIGNAL(clicked()), this, SLOT(configure()));
    connect(m_ui.clearButton, SIGNAL(clicked()), this, SLOT(clearTrigger()));
}

void MousePluginWidget::setConfigGroup(KConfigGroup cfg)
{
    //transplant the settings
    if (m_config.isValid()) {
        if (cfg.isValid()) {
            kDebug() << "transplanting settings";
            m_config.copyTo(&cfg);
        }
        //clean up the old config
        m_config.deleteGroup();
    }
    m_config = cfg;
}

KConfigGroup MousePluginWidget::configGroup()
{
    return m_config;
}

void MousePluginWidget::setContainment(Plasma::Containment *ctmt)
{
    if (m_pluginInstance) {
        m_pluginInstance->setContainment(ctmt);
    } else {
        m_pluginInstance = Plasma::ContextAction::load(m_plugin.pluginName());
        if (! m_pluginInstance) {
            //FIXME tell user
            kDebug() << "failed to load plugin!";
            return;
        }
        if (m_pluginInstance->hasConfigurationInterface()) {
            m_pluginInstance->setParent(this);
            m_pluginInstance->setContainment(ctmt);
            m_pluginInstance->restore(m_config);
        } else {
            //well, we don't need it then.
            delete m_pluginInstance;
            m_pluginInstance = 0;
            m_ui.configButton->setVisible(false);
        }
    }
}

void MousePluginWidget::setTrigger(const QString &trigger)
{
    m_ui.inputButton->setTrigger(trigger);
    m_ui.configButton->setEnabled(!trigger.isEmpty());
}

void MousePluginWidget::clearTrigger()
{
    QString oldTrigger = m_ui.inputButton->trigger();
    setTrigger(QString());
    emit triggerChanged(m_plugin.pluginName(), oldTrigger, QString());
}

void MousePluginWidget::changeTrigger(const QString &oldTrigger, const QString& newTrigger)
{
    m_ui.configButton->setEnabled(!newTrigger.isEmpty());
    emit triggerChanged(m_plugin.pluginName(), oldTrigger, newTrigger);
}

void MousePluginWidget::configure()
{
    if (! m_pluginInstance) {
        //FIXME tell user
        kDebug() << "failed to load plugin!";
        return;
    }

    if (! m_configDlg) {
        m_configDlg = new QDialog(this);
        QLayout *lay = new QVBoxLayout(m_configDlg);
        m_configDlg->setLayout(lay);
        //FIXME modality?

        //put the config in the dialog
        QWidget *w = m_pluginInstance->createConfigurationInterface(m_configDlg);
        if (w) {
            lay->addWidget(w);
        }

        //put buttons below
        QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
        lay->addWidget(buttons);

        //TODO other signals?
        connect(buttons, SIGNAL(accepted()), this, SLOT(acceptConfig()));
        connect(buttons, SIGNAL(rejected()), this, SLOT(rejectConfig()));
    }

    //FIXME er... async?
    m_configDlg->show();
}

void MousePluginWidget::acceptConfig()
{
    kDebug() << "accept";
    m_pluginInstance->configurationAccepted();
    m_configDlg->deleteLater();
    m_configDlg = 0;
    //FIXME what about empty trigger?
    emit configChanged(m_ui.inputButton->trigger());
}

void MousePluginWidget::rejectConfig()
{
    kDebug() << "reject";
    m_configDlg->deleteLater();
    m_configDlg = 0;
}

void MousePluginWidget::save()
{
    if (m_pluginInstance && m_config.isValid()) {
        m_pluginInstance->save(m_config);
    }
}

// vim: sw=4 sts=4 et tw=100
