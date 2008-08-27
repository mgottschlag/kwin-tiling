
/***************************************************************************
 *   Copyright 2008 by Montel Laurent <montel@kde.org>                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#include "shell_config.h"

#include <QGridLayout>

#include <KConfigGroup>
#include <KDebug>
#include <KPluginFactory>
#include <KPluginLoader>

#include <plasma/abstractrunner.h>

K_EXPORT_RUNNER_CONFIG(shell, ShellConfig)

ShellConfigForm::ShellConfigForm(QWidget* parent) : QWidget(parent)
{
  setupUi(this);
}

ShellConfig::ShellConfig(QWidget* parent, const QVariantList& args) :
    KCModule(ConfigFactory::componentData(), parent, args)
{
    m_ui = new ShellConfigForm(this);

    QGridLayout* layout = new QGridLayout(this);

    layout->addWidget(m_ui, 0, 0);

    connect(m_ui->cbRunInTerminal, SIGNAL(toggled(bool)), this, SLOT(setRunInTerminal(bool)));

    load();
}

ShellConfig::~ShellConfig()
{
}


void ShellConfig::setRunInTerminal(bool inTerminal)
{
    m_inTerminal = inTerminal;
}

void ShellConfig::load()
{
    KCModule::load();

    //TODO load
    emit changed(false);
}

void ShellConfig::save()
{
    //TODO save
    emit changed(false);
}

void ShellConfig::defaults()
{
    //TODO default
    emit changed(true);
}


#include "shell_config.moc"
