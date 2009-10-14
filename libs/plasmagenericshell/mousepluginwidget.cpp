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

#include "mousepluginwidget.h"

#include <Plasma/Containment>

#include <KAboutData>
#include <KAboutApplicationDialog>
#include <KDebug>

#include <QDialog>
#include <QDialogButtonBox>
#include <QLayout>

MousePluginWidget::MousePluginWidget(const KPluginInfo &plugin, const QString &trigger, QWidget *parent)
    :QWidget(parent),
    m_plugin(plugin),
    m_configDlg(0),
    m_containment(0),
    m_lastConfigLocation(trigger),
    m_tempConfigParent(QString(), KConfig::SimpleConfig)
{
    m_ui.setupUi(this);

    //read plugin data
    m_ui.name->setText(plugin.name());
    m_ui.description->setText(plugin.comment());

    if (plugin.property("X-Plasma-HasConfigurationInterface").toBool()) {
        m_tempConfig = KConfigGroup(&m_tempConfigParent, "test");
    } else {
        m_ui.configButton->setVisible(false);
    }

    setTrigger(trigger);

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
    connect(m_ui.configButton, SIGNAL(clicked()), this, SLOT(configure()));
    connect(m_ui.clearButton, SIGNAL(clicked()), this, SLOT(clearTrigger()));
    connect(m_ui.aboutButton, SIGNAL(clicked()), this, SLOT(showAbout()));
}

MousePluginWidget::~MousePluginWidget()
{
    delete m_pluginInstance.data();
}

void MousePluginWidget::setContainment(Plasma::Containment *ctmt)
{
    //note: since the old plugin's parent is the old containment,
    //we let that containment take care of deleting it
    m_containment = ctmt;
}

void MousePluginWidget::setTrigger(const QString &trigger)
{
    m_ui.inputButton->setTrigger(trigger);
    updateConfig(trigger);
}

void MousePluginWidget::clearTrigger()
{
    QString oldTrigger = m_ui.inputButton->trigger();
    setTrigger(QString());
    emit triggerChanged(m_plugin.pluginName(), oldTrigger, QString());
}

void MousePluginWidget::changeTrigger(const QString &oldTrigger, const QString& newTrigger)
{
    updateConfig(newTrigger);
    emit triggerChanged(m_plugin.pluginName(), oldTrigger, newTrigger);
}

void MousePluginWidget::updateConfig(const QString &trigger)
{
    m_ui.configButton->setEnabled(!trigger.isEmpty());
}

void MousePluginWidget::configure()
{
    if (!m_pluginInstance) {
        Plasma::ContainmentActions * pluginInstance = Plasma::ContainmentActions::load(m_containment, m_plugin.pluginName());
        if (!pluginInstance) {
            //FIXME tell user
            kDebug() << "failed to load plugin!";
            return;
        }

        m_pluginInstance = pluginInstance;

        if (m_lastConfigLocation.isEmpty()) {
            pluginInstance->restore(m_tempConfig);
        } else {
            KConfigGroup cfg = m_containment->config();
            cfg = KConfigGroup(&cfg, "ActionPlugins");
            cfg = KConfigGroup(&cfg, m_lastConfigLocation);
            pluginInstance->restore(cfg);
        }
    }

    if (!m_configDlg) {
        m_configDlg = new QDialog(this);
        QLayout *lay = new QVBoxLayout(m_configDlg);
        m_configDlg->setLayout(lay);
        m_configDlg->setWindowModality(Qt::WindowModal);

        //put the config in the dialog
        QWidget *w = m_pluginInstance.data()->createConfigurationInterface(m_configDlg);
        if (w) {
            lay->addWidget(w);
        }
	const QString title = w->windowTitle();

        m_configDlg->setWindowTitle(title.isEmpty() ? i18n("Configure Plugin") :title);
        //put buttons below
        QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
        lay->addWidget(buttons);

        //TODO other signals?
        connect(buttons, SIGNAL(accepted()), this, SLOT(acceptConfig()));
        connect(buttons, SIGNAL(rejected()), this, SLOT(rejectConfig()));
    }

    m_configDlg->show();
}

void MousePluginWidget::acceptConfig()
{
    kDebug() << "accept";
    if (m_pluginInstance) {
        m_pluginInstance.data()->configurationAccepted();
    }

    m_configDlg->deleteLater();
    m_configDlg = 0;
    emit configChanged(m_ui.inputButton->trigger());
}

void MousePluginWidget::rejectConfig()
{
    kDebug() << "reject";
    m_configDlg->deleteLater();
    m_configDlg = 0;
}

void MousePluginWidget::prepareForSave()
{
    if (!m_ui.configButton->isVisible() || m_pluginInstance || m_lastConfigLocation.isEmpty()) {
        return;
    }

    KConfigGroup cfg = m_containment->config();
    cfg = KConfigGroup(&cfg, "ActionPlugins");
    cfg = KConfigGroup(&cfg, m_lastConfigLocation);
    cfg.copyTo(&m_tempConfig);
    //kDebug() << "copied to temp";
}

void MousePluginWidget::save()
{
    if (!m_ui.configButton->isVisible()) {
        return;
    }

    QString trigger = m_ui.inputButton->trigger();
    if (!trigger.isEmpty()) {
        KConfigGroup cfg = m_containment->config();
        cfg = KConfigGroup(&cfg, "ActionPlugins");
        cfg = KConfigGroup(&cfg, trigger);
        if (m_pluginInstance) {
            m_pluginInstance.data()->save(cfg);
        } else if (!m_lastConfigLocation.isEmpty()) {
            m_tempConfig.copyTo(&cfg);
            //kDebug() << "copied from temp";
        }
    }
    m_lastConfigLocation = trigger;
}

//copied from appletbrowser.cpp
//FIXME add a feature to KAboutApplicationDialog to delete the object
/* This is just a wrapper around KAboutApplicationDialog that deletes
the KAboutData object that it is associated with, when it is deleted.
This is required to free memory correctly when KAboutApplicationDialog
is called with a temporary KAboutData that is allocated on the heap.
(see the code below, in AppletBrowserWidget::infoAboutApplet())
*/
class KAboutApplicationDialog2 : public KAboutApplicationDialog
{
public:
    KAboutApplicationDialog2(KAboutData *ab, QWidget *parent = 0)
    : KAboutApplicationDialog(ab, parent), m_ab(ab) {}

    ~KAboutApplicationDialog2()
    {
        delete m_ab;
    }

private:
    KAboutData *m_ab;
};

void MousePluginWidget::showAbout()
{
    KAboutData *aboutData = new KAboutData(m_plugin.name().toUtf8(),
            m_plugin.name().toUtf8(),
            ki18n(m_plugin.name().toUtf8()),
            m_plugin.version().toUtf8(), ki18n(m_plugin.comment().toUtf8()),
            m_plugin.fullLicense().key(), ki18n(QByteArray()), ki18n(QByteArray()), m_plugin.website().toLatin1(),
            m_plugin.email().toLatin1());

    aboutData->setProgramIconName(m_plugin.icon());

    aboutData->addAuthor(ki18n(m_plugin.author().toUtf8()), ki18n(QByteArray()), m_plugin.email().toLatin1());

    KAboutApplicationDialog *aboutDialog = new KAboutApplicationDialog2(aboutData, this);
    aboutDialog->show();
}

// vim: sw=4 sts=4 et tw=100
