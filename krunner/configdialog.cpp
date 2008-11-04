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

#include "interface.h"
#include "krunnersettings.h"
#include "interfaces/quicksand/qs_dialog.h"

KRunnerConfigDialog::KRunnerConfigDialog(Plasma::RunnerManager *manager, QWidget *parent)
    : KDialog(parent),
      m_preview(0),
      m_manager(manager)
{
    setButtons(Ok | Cancel);
    QTabWidget *m_tab = new QTabWidget(this);
    setMainWidget(m_tab);

    QWidget *m_generalSettings = new QWidget(this);
    QVBoxLayout *genLayout = new QVBoxLayout();
    QGroupBox *grp = new QGroupBox(i18n("Display"), m_generalSettings);

    QVBoxLayout *displayLayout = new QVBoxLayout();
    QHBoxLayout *dispButtonLayout = new QHBoxLayout();
    QButtonGroup *displayButtons = new QButtonGroup(this);

    m_interfaceType = KRunnerSettings::interface();

    QRadioButton *commandButton = new QRadioButton(i18n("Command Oriented"), grp);
    QRadioButton *taskButton = new QRadioButton(i18n("Task Oriented"), grp);

    displayButtons->addButton(commandButton, KRunnerSettings::EnumInterface::CommandOriented);
    displayButtons->addButton(taskButton, KRunnerSettings::EnumInterface::TaskOriented);

    if (m_interfaceType == KRunnerSettings::EnumInterface::CommandOriented) {
        commandButton->setChecked(true);
    } else {
        taskButton->setChecked(true);
    }

    connect(displayButtons, SIGNAL(buttonClicked(int)), this, SLOT(setInterface(int)));

    QPushButton *previewButton = new QPushButton(i18n("Preview"), m_generalSettings);
    dispButtonLayout->addWidget(commandButton);
    dispButtonLayout->addWidget(taskButton);
    displayLayout->addLayout(dispButtonLayout);
    displayLayout->addWidget(previewButton);
    grp->setLayout(displayLayout);

    genLayout->addWidget(grp);
    genLayout->addStretch();
    m_generalSettings->setLayout(genLayout);
    m_tab->addTab(m_generalSettings, i18n("General Settings"));

    connect(previewButton, SIGNAL(clicked()), this, SLOT(previewInterface()));

    m_sel = new KPluginSelector(m_tab);
    m_tab->addTab(m_sel, i18n("Plugins"));
    setInitialSize(QSize(400, 500));

    setWindowTitle(i18n("KRunner Settings"));

    connect(m_sel, SIGNAL(configCommitted(const QByteArray&)), this, SLOT(updateRunner(const QByteArray&)));
    connect(this, SIGNAL(okClicked()), this, SLOT(accept()));

    KService::List offers = KServiceTypeTrader::self()->query("Plasma/Runner");
    QList<KPluginInfo> effectinfos = KPluginInfo::fromServices(offers);
    m_sel->addPlugins(effectinfos, KPluginSelector::ReadConfigFile, i18n("Available Features"), QString(), KSharedConfig::openConfig("krunnerrc"));

    KConfigGroup config(KGlobal::config(), "ConfigurationDialog");
    restoreDialogSize(config);
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
    KConfigGroup config(KGlobal::config(), "ConfigurationDialog");
    saveDialogSize(config);
    KGlobal::config()->sync();
}

void KRunnerConfigDialog::accept()
{
    m_sel->save();
    m_manager->reloadConfiguration();
    KRunnerSettings::setInterface(m_interfaceType);
    KRunnerSettings::self()->writeConfig();
    close();
}

#include "configdialog.moc"

