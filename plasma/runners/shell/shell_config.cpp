
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
    connect(m_ui->cbRunAsOther, SIGNAL(clicked(bool)), this, SLOT(slotUpdateUser(bool)) );
    connect(m_ui->cbPriority, SIGNAL(clicked(bool)), this, SLOT(slotPriority(bool)));
    load();
}

ShellConfig::~ShellConfig()
{
    save();
}

void ShellConfig::load()
{
    KCModule::load();

    //FIXME: This shouldn't be hardcoded!
    KSharedConfig::Ptr cfg = KSharedConfig::openConfig( "krunnerrc" );
    KConfigGroup conf = cfg->group( "Runners" );
    KConfigGroup grp = KConfigGroup( &conf, "Shell");

    m_ui->cbRunInTerminal->setChecked(grp.readEntry("RunInTerminal", false));
    m_ui->cbRunAsOther->setChecked(grp.readEntry("RunAsOther", false));
    m_ui->cbPriority->setChecked(grp.readEntry("Priority", false));
    m_ui->cbRealtime->setChecked(grp.readEntry("RealTime", false));
    //m_ui->lePassword->text();
    //m_ui->leUsername->text();

    //TODO load
    emit changed(false);
}

void ShellConfig::save()
{
    kDebug()<<" save :";
    //FIXME: This shouldn't be hardcoded!
    KSharedConfig::Ptr cfg = KSharedConfig::openConfig( "krunnerrc" );
    KConfigGroup conf = cfg->group( "Runners" );
    KConfigGroup grp = KConfigGroup( &conf, "Shell");
    grp.writeEntry("RunInTerminal", m_ui->cbRunInTerminal->isChecked());
    bool runAsOther = m_ui->cbRunAsOther->isChecked();
    grp.writeEntry("RunAsOther", runAsOther);
    grp.writeEntry("Priority", m_ui->cbPriority->isChecked());
    grp.writeEntry("RealTime", m_ui->cbRealtime->isChecked());
    //m_ui->lePassword->text();
    //m_ui->leUsername->text();
    grp.sync();
    emit changed(false);
}

void ShellConfig::slotUpdateUser(bool b)
{
    m_ui->leUsername->setEnabled(b);
    m_ui->lePassword->setEnabled(b);
}

void ShellConfig::slotPriority(bool b)
{
    m_ui->slPriority->setEnabled(b);
    m_ui->textLabel1->setEnabled(b);
}

void ShellConfig::defaults()
{
    m_ui->cbRunInTerminal->setChecked(false);
    m_ui->cbRunAsOther->setChecked(false);
    m_ui->cbPriority->setChecked(false);
    m_ui->cbRealtime->setChecked(false);
    m_ui->lePassword->clear();
    m_ui->leUsername->clear();
    emit changed(true);
}


#include "shell_config.moc"
