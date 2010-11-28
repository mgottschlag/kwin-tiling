/*  This file is part of the KDE project
    Copyright (C) 2006 Kevin Ottens <ervin@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.

*/

#include "kcmsolid.h"


#include <kaboutdata.h>
#include <kdeversion.h>

#include <QVBoxLayout>

#include "backendchooser.h"
#include <KPluginFactory>
#include <KPluginLoader>


K_PLUGIN_FACTORY(KcmSolidFactory,
        registerPlugin<KcmSolid>();
        )
K_EXPORT_PLUGIN(KcmSolidFactory("kcm_solid"))


KcmSolid::KcmSolid(QWidget *parent, const QVariantList &args)
    : KCModule(KcmSolidFactory::componentData(), parent, args),
      m_changedChooser(0)
{
    KAboutData *about = new KAboutData(
        "kcm_solid", 0, ki18n("Solid Configuration Module"),
        KDE_VERSION_STRING, KLocalizedString(), KAboutData::License_GPL,
        ki18n("Copyright 2006 Kevin Ottens"));
    about->addAuthor(ki18n("Kevin Ottens"), KLocalizedString(), "ervin@kde.org");
    setAboutData(about);
    setButtons(Apply | Default | Help);

    setLayout(new QVBoxLayout);
    layout()->setMargin(0);
    layout()->setSpacing(0);

    m_networkChooser = new BackendChooser(this, "SolidNetworkManager");
    m_remoteControlChooser = new BackendChooser(this, "SolidRemoteControlManager");
    m_modemChooser = new BackendChooser(this, "SolidModemManager");

    layout()->addWidget(m_networkChooser);
    layout()->addWidget(m_remoteControlChooser);
    layout()->addWidget(m_modemChooser);

    load();

    connect(m_networkChooser, SIGNAL(changed(bool)),
             this, SLOT(slotChooserChanged(bool)));
    connect(m_modemChooser, SIGNAL(changed(bool)),
             this, SLOT(slotChooserChanged(bool)));

}

void KcmSolid::load()
{
    m_networkChooser->load();
    m_remoteControlChooser->load();
    m_modemChooser->load();
}

void KcmSolid::save()
{
    m_networkChooser->save();
    m_modemChooser->save();
}

void KcmSolid::defaults()
{
    m_networkChooser->defaults();
    m_modemChooser->defaults();
}

void KcmSolid::slotChooserChanged(bool state)
{
    if (state)
    {
        m_changedChooser++;
    }
    else
    {
        m_changedChooser--;
    }


    emit changed(m_changedChooser!= 0);
}

#include "kcmsolid.moc"
