/*
 *  Copyright (C) 2009 Dario Freddi <drf@kde.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */
#include "remotewidgets.h"

#include "ui_generalpage.h"

#include <kauthaction.h>
#include <kmessagebox.h>
#include <kpluginfactory.h>
#include <kaboutdata.h>

using namespace KAuth;

K_PLUGIN_FACTORY(RemoteWidgetsModuleFactory, registerPlugin<RemoteWidgetsModule>();)
K_EXPORT_PLUGIN(RemoteWidgetsModuleFactory("kcmremotewidgets"))


RemoteWidgetsModule::RemoteWidgetsModule(QWidget *parent, const QVariantList &)
  : KCModule(RemoteWidgetsModuleFactory::componentData(), parent/*, name*/)
  , m_ui(new Ui::MainPage)
{
    KAboutData *about =
    new KAboutData("kcmremotewidgets", 0, ki18n("Define policies for remote widgets"),
                   0, KLocalizedString(), KAboutData::License_GPL,
                   ki18n("(c) 2009 Rob Scheepmaker"));

    about->addAuthor(ki18n("Rob Scheepmaker"), ki18n("Maintainer"), "r.scheepmaker@student.utwente.nl");
    about->addAuthor(ki18n("Dario Freddi"), ki18n("Developer"), "drf@kde.org");
    about->addAuthor(ki18n("Riccardo Iaconelli"), ki18n("Interface design"), "riccardo@kde.org");
    setAboutData(about);

    setButtons(Help|Apply);

    setNeedsAuthorization(true);

    m_ui->setupUi(this);
}

RemoteWidgetsModule::~RemoteWidgetsModule()
{
    delete m_ui;
}


void RemoteWidgetsModule::save()
{

}

void RemoteWidgetsModule::load()
{

}

void RemoteWidgetsModule::defaults()
{
}

#include "remotewidgets.moc"
