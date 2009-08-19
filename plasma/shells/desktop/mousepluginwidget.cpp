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

MousePluginWidget::MousePluginWidget(const KPluginInfo &plugin, QWidget *parent)
    :QWidget(parent),
    m_plugin(plugin),
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
    delete m_pluginInstance;
}

void MousePluginWidget::setContainment(Plasma::Containment *ctmt)
{
    //note: since the old plugin's parent is the old containment,
    //we let that containment take care of deleting it

    m_containment = ctmt;

    m_pluginInstance = Plasma::ContainmentActions::load(ctmt, m_plugin.pluginName());
    if (! m_pluginInstance) {
        //FIXME tell user
        kDebug() << "failed to load plugin!";
        return;
    }

    if (m_pluginInstance->hasConfigurationInterface()) {
        QString trigger = m_ui.inputButton->trigger();
        if (trigger.isEmpty()) {
            //FIXME m_pluginInstance->restore(KConfigGroup());
        } else {
            //FIXME I'm assuming the config was successfully saved here already
            KConfigGroup cfg = ctmt->config();
            cfg = KConfigGroup(&cfg, "ActionPlugins");
            cfg = KConfigGroup(&cfg, m_ui.inputButton->trigger());
            m_pluginInstance->restore(cfg);
        }
    } else {
        //well, we don't need it then.
        delete m_pluginInstance;
        m_pluginInstance = 0;
        m_ui.configButton->setVisible(false);
    }
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
    if (! m_pluginInstance) {
        //FIXME tell user
        kDebug() << "failed to load plugin!";
        return;
    }

    if (! m_configDlg) {
        m_configDlg = new QDialog(this);
        QLayout *lay = new QVBoxLayout(m_configDlg);
        m_configDlg->setLayout(lay);
        m_configDlg->setWindowModality(Qt::WindowModal);

        //put the config in the dialog
        QWidget *w = m_pluginInstance->createConfigurationInterface(m_configDlg);
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
    m_pluginInstance->configurationAccepted();
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

void MousePluginWidget::save()
{
    QString trigger = m_ui.inputButton->trigger();
    if (m_pluginInstance && !trigger.isEmpty()) {
        KConfigGroup cfg = m_containment->config();
        cfg = KConfigGroup(&cfg, "ActionPlugins");
        cfg = KConfigGroup(&cfg, trigger);
        m_pluginInstance->save(cfg);
    }
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
