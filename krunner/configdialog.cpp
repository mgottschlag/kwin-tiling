/*
 *   Copyright 2008 Ryan P. Bitanga <ryan.bitanga@gmail.com>
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

#include "configdialog.h"

#include <QButtonGroup>
#include <QGroupBox>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QRadioButton>
#include <QTabWidget>
#include <QVBoxLayout>

#include <KConfigGroup>
#include <KDebug>
#include <KGlobal>
#include <KPluginInfo>
#include <KPluginSelector>
#include <KServiceTypeTrader>

#include <Plasma/RunnerManager>

#include "interfaces/default/interface.h"
#include "krunnersettings.h"
#include "interfaces/quicksand/qs_dialog.h"

KRunnerConfigDialog::KRunnerConfigDialog(Plasma::RunnerManager *manager, QWidget *parent)
    : KTabWidget(parent),
      m_preview(0),
      m_manager(manager)
{
    m_sel = new KPluginSelector(this);
    addTab(m_sel, i18n("Plugins"));

    QWidget *m_generalSettings = new QWidget(this);
    //QVBoxLayout *genLayout = new QVBoxLayout(m_generalSettings);

    m_interfaceType = KRunnerSettings::interface();
    m_uiOptions.setupUi(m_generalSettings);

    QButtonGroup *positionButtons = new QButtonGroup(m_generalSettings);
    positionButtons->addButton(m_uiOptions.topEdgeButton);
    positionButtons->addButton(m_uiOptions.freeFloatingButton);

    QButtonGroup *displayButtons = new QButtonGroup(m_generalSettings);
    displayButtons->addButton(m_uiOptions.commandButton, KRunnerSettings::EnumInterface::CommandOriented);
    displayButtons->addButton(m_uiOptions.taskButton, KRunnerSettings::EnumInterface::TaskOriented);
    connect(displayButtons, SIGNAL(buttonClicked(int)), this, SLOT(setInterface(int)));

    if (m_interfaceType == KRunnerSettings::EnumInterface::CommandOriented) {
        m_uiOptions.commandButton->setChecked(true);
    } else {
        m_uiOptions.taskButton->setChecked(true);
    }

    connect(m_uiOptions.previewButton, SIGNAL(clicked()), this, SLOT(previewInterface()));

    addTab(m_generalSettings, i18n("User Interface"));

    connect(m_sel, SIGNAL(configCommitted(const QByteArray&)), this, SLOT(updateRunner(const QByteArray&)));

    KService::List offers = KServiceTypeTrader::self()->query("Plasma/Runner");
    QList<KPluginInfo> runnerInfo = KPluginInfo::fromServices(offers);
    m_sel->addPlugins(runnerInfo, KPluginSelector::ReadConfigFile, i18n("Available Features"), QString(), KSharedConfig::openConfig("krunnerrc"));
}

void KRunnerConfigDialog::previewInterface()
{
    delete m_preview;
    switch (m_interfaceType) {
    case KRunnerSettings::EnumInterface::CommandOriented:
        m_preview = new Interface(m_manager, this);
        break;
    default:
        m_preview = new QsDialog(m_manager, this);
        break;
    }

    m_preview->setCenterPositioned(m_uiOptions.freeFloatingButton->isChecked());
    m_preview->show();
}

void KRunnerConfigDialog::setInterface(int type)
{
    m_interfaceType = type;
}

void KRunnerConfigDialog::updateRunner(const QByteArray &name)
{
    Plasma::AbstractRunner *runner = m_manager->runner(name);
    //Update runner if runner is loaded
    if (runner) {
        runner->reloadConfiguration();
    }
}

KRunnerConfigDialog::~KRunnerConfigDialog()
{
}

void KRunnerConfigDialog::accept()
{
    m_sel->save();
    m_manager->reloadConfiguration();
    KRunnerSettings::setInterface(m_interfaceType);
    KRunnerSettings::setFreeFloating(m_uiOptions.freeFloatingButton->isChecked());
    KRunnerSettings::self()->writeConfig();
    close();
}

#include "configdialog.moc"

