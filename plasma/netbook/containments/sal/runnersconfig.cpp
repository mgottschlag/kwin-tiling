/*
 *   Copyright 2008 Ryan P. Bitanga <ryan.bitanga@gmail.com>
 *   Copyright 2009 Marco Martin <notmart@gmail.com>
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

#include "runnersconfig.h"

#include <QPushButton>

#include <KDebug>
#include <KGlobal>
#include <KPluginInfo>
#include <KServiceTypeTrader>

#include <Plasma/RunnerManager>


RunnersConfig::RunnersConfig(Plasma::RunnerManager *manager, QWidget *parent)
    : KPluginSelector(parent),
      m_manager(manager)
{
   connect(this, SIGNAL(configCommitted(QByteArray)), this, SLOT(updateRunner(QByteArray)));

    KService::List offers = KServiceTypeTrader::self()->query("Plasma/Runner");
    QList<KPluginInfo> runnerInfo = KPluginInfo::fromServices(offers);
    addPlugins(runnerInfo, KPluginSelector::ReadConfigFile, i18n("Available Features"), QString(), KGlobal::config());
}

void RunnersConfig::updateRunner(const QByteArray &name)
{
    Plasma::AbstractRunner *runner = m_manager->runner(name);
    //Update runner if runner is loaded
    if (runner) {
        runner->reloadConfiguration();
    }
}

RunnersConfig::~RunnersConfig()
{
}

void RunnersConfig::accept()
{
    save();
    m_manager->reloadConfiguration();
    close();
}

#include "runnersconfig.moc"

