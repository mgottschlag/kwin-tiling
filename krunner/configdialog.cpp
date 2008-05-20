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

#include <QDialogButtonBox>
#include <QTabWidget>
#include <QVBoxLayout>

#include <KConfigGroup>
#include <KDebug>
#include <KGlobal>
#include <KPluginInfo>
#include <KPluginSelector>
#include <KServiceTypeTrader>

#include <plasma/runnermanager.h>

KRunnerConfigDialog::KRunnerConfigDialog(Plasma::RunnerManager* manager, QWidget* parent)
    : KDialog(parent),
      m_manager(manager)
{
    setButtons(Ok | Cancel);
    m_sel = new KPluginSelector(this);
    setMainWidget(m_sel);
    setInitialSize(QSize(400, 500));

    setWindowTitle(i18n("KRunner Settings"));

    connect(m_sel, SIGNAL(configCommitted(const QByteArray&)), this, SLOT(updateRunner(const QByteArray&)));
    connect(this, SIGNAL(okClicked()), this, SLOT(accept()));

    KService::List offers = KServiceTypeTrader::self()->query("Plasma/Runner");
    QList<KPluginInfo> effectinfos = KPluginInfo::fromServices(offers);
    m_sel->addPlugins(effectinfos, KPluginSelector::ReadConfigFile, i18n("Runners"), QString(), KSharedConfig::openConfig("krunnerrc"));

    KConfigGroup config(KGlobal::config(), "ConfigurationDialog");
    restoreDialogSize(config);
}

void KRunnerConfigDialog::updateRunner(const QByteArray& name)
{
    QString _name(name);
    Plasma::AbstractRunner* runner = m_manager->runner(_name);
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
}

#include "configdialog.moc"

