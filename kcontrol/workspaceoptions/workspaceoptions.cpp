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
#include "workspaceoptions.h"

#include "ui_mainpage.h"

#include <QDBusInterface>

#include <kmessagebox.h>
#include <kpluginfactory.h>
#include <kaboutdata.h>
#include <KRun>
#include <KUrl>

using namespace KAuth;

K_PLUGIN_FACTORY(WorkspaceOptionsModuleFactory, registerPlugin<WorkspaceOptionsModule>();)
K_EXPORT_PLUGIN(WorkspaceOptionsModuleFactory("kcmworkspaceoptions"))


WorkspaceOptionsModule::WorkspaceOptionsModule(QWidget *parent, const QVariantList &)
  : KCModule(WorkspaceOptionsModuleFactory::componentData(), parent),
    m_kwinConfig( KSharedConfig::openConfig("kwinrc")),
    m_plasmaDesktopAutostart("plasma-desktop"),
    m_plasmaNetbookAutostart("plasma-netbook"),
    m_ui(new Ui::MainPage)
{
    KAboutData *about =
    new KAboutData("kcmworkspaceoptions", 0, ki18n("Global options for the Plasma workspace"),
                   0, KLocalizedString(), KAboutData::License_GPL,
                   ki18n("(c) 2009 Marco MArtin"));

    about->addAuthor(ki18n("Marco Martin"), ki18n("Maintainer"), "notmart@gmail.com");

    setAboutData(about);

    setButtons(Help|Apply);

    m_ui->setupUi(this);

    connect(m_ui->formFactor, SIGNAL(currentIndexChanged(int)), this, SLOT(changed()));
    connect(m_ui->dashboardMode, SIGNAL(currentIndexChanged(int)), this, SLOT(changed()));
    connect(m_ui->formFactor, SIGNAL(currentIndexChanged(int)), this, SLOT(formFactorChanged(int)));
}

WorkspaceOptionsModule::~WorkspaceOptionsModule()
{
    delete m_ui;
}


void WorkspaceOptionsModule::save()
{
    const bool isDesktop = m_ui->formFactor->currentIndex() == 0;

    m_plasmaDesktopAutostart.setAutostarts(isDesktop);
    m_plasmaDesktopAutostart.setStartPhase(KAutostart::BaseDesktop);
    m_plasmaDesktopAutostart.setCommand("plasma-desktop");
    m_plasmaDesktopAutostart.setAllowedEnvironments(QStringList()<<"KDE");

    m_plasmaNetbookAutostart.setAutostarts(!isDesktop);
    m_plasmaNetbookAutostart.setStartPhase(KAutostart::BaseDesktop);
    m_plasmaNetbookAutostart.setCommand("plasma-netbook");
    m_plasmaNetbookAutostart.setAllowedEnvironments(QStringList()<<"KDE");

    KConfigGroup cg(m_kwinConfig, "Windows");

    cg.writeEntry("BorderlessMaximizedWindows", !isDesktop);
    if (!isDesktop) {
        cg.writeEntry("Placement", "Maximizing");
    } else {
        cg.writeEntry("Placement", "Smart");
    }

    cg.sync();

    // Reload KWin.
    QDBusMessage message = QDBusMessage::createSignal( "/KWin", "org.kde.KWin", "reloadConfig" );
    QDBusConnection::sessionBus().send(message);


    if (isDesktop && !m_currentlyIsDesktop) {
        if (KRun::run("plasma-desktop", KUrl::List(), 0)) {
            QDBusInterface interface("org.kde.plasma-netbook", "/MainApplication");
            interface.call("quit");
        }
    } else if (!isDesktop && m_currentlyIsDesktop) {
        if (KRun::run("plasma-netbook", KUrl::List(), 0)) {
            QDBusInterface interface("org.kde.plasma-desktop", "/MainApplication");
            interface.call("quit");
        }
    }
    m_currentlyIsDesktop = isDesktop;

    QDBusInterface interface("org.kde.plasma-desktop", "/App");
    interface.call("setFixedDashboard", (m_ui->dashboardMode->currentIndex() == 1));
}

void WorkspaceOptionsModule::load()
{
    if (m_plasmaDesktopAutostart.autostarts()) {
        m_ui->formFactor->setCurrentIndex(0);
    } else {
        m_ui->formFactor->setCurrentIndex(1);
    }

    m_currentlyIsDesktop = m_plasmaDesktopAutostart.autostarts();

    QDBusInterface interface("org.kde.plasma-desktop", "/App");
    bool fixedDashboard = interface.call("fixedDashboard").arguments().first().toBool();

    if (fixedDashboard) {
        m_ui->dashboardMode->setCurrentIndex(1);
    } else {
        m_ui->dashboardMode->setCurrentIndex(0);
    }
}

void WorkspaceOptionsModule::defaults()
{
    m_ui->formFactor->setCurrentIndex(0);
    m_ui->dashboardMode->setCurrentIndex(0);
}

void WorkspaceOptionsModule::formFactorChanged(int newFormFactorIndex)
{
    m_ui->dashboardMode->setEnabled(newFormFactorIndex == 0);
}

#include "workspaceoptions.moc"
