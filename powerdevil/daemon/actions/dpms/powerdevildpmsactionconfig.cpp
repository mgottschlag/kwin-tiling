/***************************************************************************
 *   Copyright (C) 2010 by Dario Freddi <drf@kde.org>                      *
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

#include "powerdevildpmsactionconfig.h"

#include <KIntSpinBox>
#include <KLocalizedString>
#include <KPluginFactory>

K_PLUGIN_FACTORY(PowerDevilDPMSConfigFactory, registerPlugin<PowerDevilDPMSActionConfig>(); )
K_EXPORT_PLUGIN(PowerDevilDPMSConfigFactory("powerdevildpmsaction_config"))

PowerDevilDPMSActionConfig::PowerDevilDPMSActionConfig(QObject* parent, const QVariantList& )
        : ActionConfig(parent)
{

}
PowerDevilDPMSActionConfig::~PowerDevilDPMSActionConfig()
{

}

void PowerDevilDPMSActionConfig::save()
{
    configGroup().writeEntry("idleTime", m_spinBox->value() * 60);

    configGroup().sync();
}

void PowerDevilDPMSActionConfig::load()
{
    m_spinBox->setValue(configGroup().readEntry<int>("idleTime", 600000) / 60);
}

QList< QPair< QString, QWidget* > > PowerDevilDPMSActionConfig::buildUi()
{
    QList< QPair< QString, QWidget* > > retlist;

    m_spinBox = new KIntSpinBox(0, 180, 1, 0, 0);
    m_spinBox->setMaximumWidth(150);
    m_spinBox->setMinimum(1);
    m_spinBox->setMaximum(360);
    m_spinBox->setSuffix(i18n(" min"));
    retlist.append(qMakePair< QString, QWidget* >(i18n("Switch off after"), m_spinBox));

    connect(m_spinBox, SIGNAL(valueChanged(int)), this, SLOT(setChanged()));

    return retlist;
}

