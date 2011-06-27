/*
 *  Copyright (C) 2009 Marco Martin <notmart@gmail.com>
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

#include <KDebug>
#include <KAboutData>
#include <KMessageBox>
#include <KPluginFactory>
#include <KRun>
#include <KStandardDirs>
#include <KUrl>

using namespace KAuth;

K_PLUGIN_FACTORY(WorkspaceOptionsModuleFactory, registerPlugin<WorkspaceOptionsModule>();)
K_EXPORT_PLUGIN(WorkspaceOptionsModuleFactory("kcmworkspaceoptions"))


WorkspaceOptionsModule::WorkspaceOptionsModule(QWidget *parent, const QVariantList &)
  : KCModule(WorkspaceOptionsModuleFactory::componentData(), parent),
    m_kwinConfig( KSharedConfig::openConfig("kwinrc")),
    m_ownConfig( KSharedConfig::openConfig("workspaceoptionsrc")),
    m_plasmaDesktopAutostart("plasma-desktop"),
    m_plasmaNetbookAutostart("plasma-netbook"),
    m_krunnerAutostart("krunner"),
    m_ui(new Ui::MainPage)
{
    KAboutData *about =
    new KAboutData("kcmworkspaceoptions", 0, ki18n("Global options for the Plasma Workspace"),
                   0, KLocalizedString(), KAboutData::License_GPL,
                   ki18n("(c) 2009 Marco Martin"));

    about->addAuthor(ki18n("Marco Martin"), ki18n("Maintainer"), "notmart@gmail.com");

    setAboutData(about);

    setButtons(Help|Apply);

    m_ui->setupUi(this);

    connect(m_ui->formFactor, SIGNAL(currentIndexChanged(int)), this, SLOT(changed()));
    connect(m_ui->dashboardMode, SIGNAL(currentIndexChanged(int)), this, SLOT(changed()));
    connect(m_ui->showToolTips, SIGNAL(toggled(bool)), this, SLOT(changed()));
    connect(m_ui->formFactor, SIGNAL(currentIndexChanged(int)), this, SLOT(formFactorChanged(int)));

    //enable the combobox if both plasma-desktop and plasma-netbook are present
    if (KStandardDirs::findExe("plasma-desktop").isNull() || KStandardDirs::findExe("plasma-netbook").isNull()) {
        m_ui->formFactor->setEnabled(false);
    }
}

WorkspaceOptionsModule::~WorkspaceOptionsModule()
{
    delete m_ui;
}


void WorkspaceOptionsModule::save()
{
    {
        KConfig config("plasmarc");
        KConfigGroup cg(&config, "PlasmaToolTips");
        cg.writeEntry("Delay", m_ui->showToolTips->isChecked() ? 0.7 : -1);
    }

    const bool isDesktop = m_ui->formFactor->currentIndex() == 0;

    m_plasmaDesktopAutostart.setAutostarts(isDesktop);
    m_plasmaDesktopAutostart.setStartPhase(KAutostart::BaseDesktop);
    m_plasmaDesktopAutostart.setCommand("plasma-desktop");
    m_plasmaDesktopAutostart.setAllowedEnvironments(QStringList()<<"KDE");

    m_plasmaNetbookAutostart.setAutostarts(!isDesktop);
    m_plasmaNetbookAutostart.setStartPhase(KAutostart::BaseDesktop);
    m_plasmaNetbookAutostart.setCommand("plasma-netbook");
    m_plasmaNetbookAutostart.setAllowedEnvironments(QStringList()<<"KDE");

    m_krunnerAutostart.setAutostarts(isDesktop);
    m_krunnerAutostart.setStartPhase(KAutostart::BaseDesktop);
    m_krunnerAutostart.setCommand("krunner");
    m_krunnerAutostart.setAllowedEnvironments(QStringList()<<"KDE");

    KConfigGroup winCg(m_kwinConfig, "Windows");

    winCg.writeEntry("BorderlessMaximizedWindows", !isDesktop);
    if (!isDesktop) {
        winCg.writeEntry("Placement", "Maximizing");
    } else {
        winCg.writeEntry("Placement", "Smart");
    }
    winCg.sync();

    KConfigGroup ownButtonsCg(m_ownConfig, "TitleBarButtons");
    KConfigGroup ownPresentWindowsCg(m_ownConfig, "Effect-PresentWindows");
    KConfigGroup ownCompositingCg(m_ownConfig, "Compositing");
    KConfigGroup kwinStyleCg(m_kwinConfig, "Style");
    KConfigGroup kwinPresentWindowsCg(m_kwinConfig, "Effect-PresentWindows");
    KConfigGroup kwinCompositingCg(m_kwinConfig, "Compositing");


    QString desktopTitleBarButtonsLeft = ownButtonsCg.readEntry("DesktopLeft", "MS");
    QString desktopTitleBarButtonsRight = ownButtonsCg.readEntry("DesktopRight", "HIA__X");

    QString netbookTitleBarButtonsLeft = ownButtonsCg.readEntry("NetbookLeft", "MS");
    QString netbookTitleBarButtonsRight = ownButtonsCg.readEntry("NetbookRight", "HA__X");


    bool desktopPresentWindowsTabbox = false;
    bool desktopBoxSwitchTabbox = true;
    bool desktopCoverSwitchTabbox = false;
    bool desktopFlipSwitchTabbox = false;

    bool netbookPresentWindowsTabbox = true;
    bool netbookBoxSwitchTabbox = false;
    bool netbookCoverSwitchTabbox = false;
    bool netbookFlipSwitchTabbox = false;


    int desktopPresentWindowsLayoutMode = 0;
    int netbookPresentWindowsLayoutMode = 1;

    bool desktopUnredirectFullscreen = ownCompositingCg.readEntry("DesktopUnredirectFullscreen", true);
    bool netbookUnredirectFullscreen = ownCompositingCg.readEntry("NetbookUnredirectFullscreen", false);

    if (m_currentlyIsDesktop) {
        //save the user preferences on titlebar buttons
        desktopTitleBarButtonsLeft = kwinStyleCg.readEntry("ButtonsOnLeft", "MS");
        desktopTitleBarButtonsRight = kwinStyleCg.readEntry("ButtonsOnRight", "HIA__X");
        ownButtonsCg.writeEntry("DesktopLeft", desktopTitleBarButtonsLeft);
        ownButtonsCg.writeEntry("DesktopRight", desktopTitleBarButtonsRight);

        //Unredirect fullscreen
        desktopUnredirectFullscreen = kwinCompositingCg.readEntry("UnredirectFullscreen", true);
        ownCompositingCg.writeEntry("DesktopUnredirectFullscreen", desktopUnredirectFullscreen);

        //desktop grid effect
        desktopPresentWindowsLayoutMode = kwinPresentWindowsCg.readEntry("LayoutMode", 0);
        ownPresentWindowsCg.writeEntry("DesktopLayoutMode", desktopPresentWindowsLayoutMode);

        //box switch effect
        desktopPresentWindowsTabbox = kwinPresentWindowsCg.readEntry("TabBox", false);
        ownPresentWindowsCg.writeEntry("DesktopTabBox", desktopPresentWindowsTabbox);

        KConfigGroup ownBoxSwitchCg( m_ownConfig, "Effect-BoxSwitch" );
        KConfigGroup kwinBoxSwitchCg( m_kwinConfig, "Effect-BoxSwitch" );
        desktopBoxSwitchTabbox = kwinBoxSwitchCg.readEntry("TabBox", desktopBoxSwitchTabbox);
        ownBoxSwitchCg.writeEntry( "DesktopTabBox", desktopBoxSwitchTabbox );
        ownBoxSwitchCg.sync();

        KConfigGroup ownCoverSwitchCg( m_ownConfig, "Effect-CoverSwitch" );
        KConfigGroup kwinCoverSwitchCg( m_kwinConfig, "Effect-CoverSwitch" );
        desktopCoverSwitchTabbox = kwinCoverSwitchCg.readEntry("TabBox", desktopCoverSwitchTabbox);
        ownCoverSwitchCg.writeEntry( "DesktopTabBox", desktopCoverSwitchTabbox );
        ownCoverSwitchCg.sync();

        KConfigGroup ownFlipSwitchCg( m_ownConfig, "Effect-FlipSwitch" );
        KConfigGroup kwinFlipSwitchCg( m_kwinConfig, "Effect-FlipSwitch" );
        desktopFlipSwitchTabbox = kwinFlipSwitchCg.readEntry("TabBox", desktopFlipSwitchTabbox);
        ownFlipSwitchCg.writeEntry( "DesktopTabBox", desktopFlipSwitchTabbox );
        ownFlipSwitchCg.sync();
    } else {
        //save the user preferences on titlebar buttons
        netbookTitleBarButtonsLeft = kwinStyleCg.readEntry("ButtonsOnLeft", "MS");
        netbookTitleBarButtonsRight = kwinStyleCg.readEntry("ButtonsOnRight", "HA__X");
        ownButtonsCg.writeEntry("NetbookLeft", netbookTitleBarButtonsLeft);
        ownButtonsCg.writeEntry("NetbookRight", netbookTitleBarButtonsRight);

        //Unredirect fullscreen
        netbookUnredirectFullscreen = kwinCompositingCg.readEntry("UnredirectFullscreen", true);
        ownCompositingCg.writeEntry("NetbookUnredirectFullscreen", netbookUnredirectFullscreen);

        //desktop grid effect
        desktopPresentWindowsLayoutMode = kwinPresentWindowsCg.readEntry("LayoutMode", 0);
        ownPresentWindowsCg.writeEntry("NetbookLayoutMode", desktopPresentWindowsLayoutMode);


        //box switch effect
        netbookPresentWindowsTabbox = kwinPresentWindowsCg.readEntry("TabBox", false);
        ownPresentWindowsCg.writeEntry("NetbookTabBox", netbookPresentWindowsTabbox);

        KConfigGroup ownBoxSwitchCg( m_ownConfig, "Effect-BoxSwitch" );
        KConfigGroup kwinBoxSwitchCg( m_kwinConfig, "Effect-BoxSwitch" );
        netbookBoxSwitchTabbox = kwinBoxSwitchCg.readEntry("TabBox", netbookBoxSwitchTabbox);
        ownBoxSwitchCg.writeEntry( "NetbookTabBox", netbookBoxSwitchTabbox );
        ownBoxSwitchCg.sync();

        KConfigGroup ownCoverSwitchCg( m_ownConfig, "Effect-CoverSwitch" );
        KConfigGroup kwinCoverSwitchCg( m_kwinConfig, "Effect-CoverSwitch" );
        netbookCoverSwitchTabbox = kwinCoverSwitchCg.readEntry("TabBox", netbookCoverSwitchTabbox);
        ownCoverSwitchCg.writeEntry( "NetbookTabBox", netbookCoverSwitchTabbox );
        ownCoverSwitchCg.sync();

        KConfigGroup ownFlipSwitchCg( m_ownConfig, "Effect-FlipSwitch" );
        KConfigGroup kwinFlipSwitchCg( m_kwinConfig, "Effect-FlipSwitch" );
        netbookFlipSwitchTabbox = kwinFlipSwitchCg.readEntry("TabBox", netbookFlipSwitchTabbox);
        ownFlipSwitchCg.writeEntry( "NetbookTabBox", netbookFlipSwitchTabbox );
        ownFlipSwitchCg.sync();
    }

    ownButtonsCg.sync();
    ownPresentWindowsCg.sync();
    ownCompositingCg.sync();

    kwinStyleCg.writeEntry("CustomButtonPositions", true);
    if (isDesktop) {
        //kill/enable the minimize button, unless configured differently
        kwinStyleCg.writeEntry("ButtonsOnLeft", desktopTitleBarButtonsLeft);
        kwinStyleCg.writeEntry("ButtonsOnRight", desktopTitleBarButtonsRight);

        // enable unredirect fullscreen, unless configured differently
        kwinCompositingCg.writeEntry("UnredirectFullscreen", desktopUnredirectFullscreen);

        //present windows mode
        kwinPresentWindowsCg.writeEntry("LayoutMode", desktopPresentWindowsLayoutMode);

        //what to use as tabbox
        kwinPresentWindowsCg.writeEntry("TabBox", desktopPresentWindowsTabbox);

        KConfigGroup kwinBoxSwitchCg( m_kwinConfig, "Effect-BoxSwitch" );
        kwinBoxSwitchCg.writeEntry( "TabBox", desktopBoxSwitchTabbox );
        kwinBoxSwitchCg.sync();

        KConfigGroup kwinCoverSwitchCg( m_kwinConfig, "Effect-CoverSwitch" );
        kwinCoverSwitchCg.writeEntry( "TabBox", desktopCoverSwitchTabbox );
        kwinCoverSwitchCg.sync();

        KConfigGroup kwinFlipSwitchCg( m_kwinConfig, "Effect-FlipSwitch" );
        kwinFlipSwitchCg.writeEntry( "TabBox", desktopFlipSwitchTabbox );
        kwinFlipSwitchCg.sync();
    } else {
        //kill/enable the minimize button, unless configured differently
        kwinStyleCg.writeEntry("ButtonsOnLeft", netbookTitleBarButtonsLeft);
        kwinStyleCg.writeEntry("ButtonsOnRight", netbookTitleBarButtonsRight);

        // disable unredirect fullscreen, unless configured differently
        kwinCompositingCg.writeEntry("UnredirectFullscreen", netbookUnredirectFullscreen);

        //present windows mode
        kwinPresentWindowsCg.writeEntry("LayoutMode", netbookPresentWindowsLayoutMode);

        //what to use as tabbox
        kwinPresentWindowsCg.writeEntry("TabBox", netbookPresentWindowsTabbox);

        KConfigGroup kwinBoxSwitchCg( m_kwinConfig, "Effect-BoxSwitch" );
        kwinBoxSwitchCg.writeEntry( "TabBox", netbookBoxSwitchTabbox );
        kwinBoxSwitchCg.sync();

        KConfigGroup kwinCoverSwitchCg( m_kwinConfig, "Effect-CoverSwitch" );
        kwinCoverSwitchCg.writeEntry( "TabBox", netbookCoverSwitchTabbox );
        kwinCoverSwitchCg.sync();

        KConfigGroup kwinFlipSwitchCg( m_kwinConfig, "Effect-FlipSwitch" );
        kwinFlipSwitchCg.writeEntry( "TabBox", netbookFlipSwitchTabbox );
        kwinFlipSwitchCg.sync();
    }

    kwinStyleCg.sync();
    kwinPresentWindowsCg.sync();
    kwinCompositingCg.sync();

    // Reload KWin.
    QDBusMessage message = QDBusMessage::createSignal( "/KWin", "org.kde.KWin", "reloadConfig" );
    QDBusConnection::sessionBus().send(message);


    if (isDesktop && !m_currentlyIsDesktop) {
        if (KRun::run("plasma-desktop", KUrl::List(), 0)) {
            QDBusInterface interface("org.kde.plasma-netbook", "/MainApplication");
            interface.call(QDBus::NoBlock, "quit");
            KRun::run("krunner", KUrl::List(), 0);
        }
    } else if (!isDesktop && m_currentlyIsDesktop) {
        if (KRun::run("plasma-netbook", KUrl::List(), 0)) {
            QDBusInterface interface("org.kde.plasma-desktop", "/MainApplication");
            interface.call(QDBus::NoBlock, "quit");
            QDBusInterface krunnerInterface("org.kde.krunner", "/MainApplication");
            krunnerInterface.call(QDBus::NoBlock, "quit");
        }
    }
    m_currentlyIsDesktop = isDesktop;

    const bool fixedDashboard = m_ui->dashboardMode->currentIndex() == 1;
    if (m_currentlyFixedDashboard != fixedDashboard) {
        // confirm with th euser that this is really the change they want if going from
        // separate-dashboard back to dashboard-follows-desktop
        const QString message = i18n("Turning off the show independant widget set feature will "
                                     "result in all widgets that were on the dashboard to be removed. "
                                     "Are you sure you wish to make this change?");
        if (!m_currentlyFixedDashboard ||
            KMessageBox::Yes == KMessageBox::warningYesNo(this, message, i18n("Turn off independant widgets?"),
                                                          KGuiItem(i18n("Turn off independant widgets")),
                                                          KStandardGuiItem::cancel())) {
            QDBusInterface interface("org.kde.plasma-desktop", "/App");
            interface.call(QDBus::NoBlock, "setFixedDashboard", fixedDashboard);
            m_currentlyFixedDashboard = fixedDashboard;
        } else {
            m_ui->dashboardMode->setCurrentIndex(m_currentlyFixedDashboard ? 1 : 0);
        }
    }
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
    m_currentlyFixedDashboard = false;

    if (interface.isValid()) {
        m_currentlyFixedDashboard = interface.call("fixedDashboard").arguments().first().toBool();
    }

    m_ui->dashboardMode->setCurrentIndex(m_currentlyFixedDashboard ? 1 : 0);

    KConfig config("plasmarc");
    KConfigGroup cg(&config, "PlasmaToolTips");
    m_ui->showToolTips->setChecked(cg.readEntry("Delay", 0.7) > 0);
}

void WorkspaceOptionsModule::defaults()
{
    m_ui->formFactor->setCurrentIndex(0);
    m_ui->dashboardMode->setCurrentIndex(0);
}

void WorkspaceOptionsModule::formFactorChanged(int newFormFactorIndex)
{
    m_ui->dashboardMode->setEnabled(newFormFactorIndex == 0);
    m_ui->dashboardLabel->setEnabled(newFormFactorIndex == 0);
}

#include "workspaceoptions.moc"
