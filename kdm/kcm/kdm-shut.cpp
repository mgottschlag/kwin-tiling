/*
    Copyright (C) 1997-1998 Thomas Tanghus (tanghus@earthling.net)

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "kdm-shut.h"

#include <config.ci>

#include "kbackedcombobox.h"

#include <KDialog>
#include <KLineEdit>
#include <KLocale>
#include <KConfig>
#include <KUrlRequester>

#include <QGroupBox>
#include <QLabel>
#include <QGridLayout>
#include <QVBoxLayout>

extern KConfig *config;

KDMSessionsWidget::KDMSessionsWidget(QWidget *parent)
    : QWidget(parent)
{
    QString wtstr;


    QGroupBox *group0 = new QGroupBox(i18n("Allow Shutdown"), this);

    sdlcombo = new KComboBox(group0);
    sdlcombo->setEditable(false);
    sdllabel = new QLabel(i18nc("shutdown request origin", "&Local:"), group0);
    sdllabel->setBuddy(sdlcombo);
    sdlcombo->insertItem(SdAll, i18nc("@item:inlistbox allow shutdown", "Everybody"));
    sdlcombo->insertItem(SdRoot, i18nc("@item:inlistbox allow shutdown", "Only Root"));
    sdlcombo->insertItem(SdNone, i18nc("@item:inlistbox allow shutdown", "Nobody"));
    connect(sdlcombo, SIGNAL(activated(int)), SIGNAL(changed()));
    sdrcombo = new KComboBox(group0);
    sdrcombo->setEditable(false);
    sdrlabel = new QLabel(i18nc("shutdown request origin", "&Remote:"), group0);
    sdrlabel->setBuddy(sdrcombo);
    sdrcombo->insertItem(SdAll, i18nc("@item:inlistbox allow shutdown", "Everybody"));
    sdrcombo->insertItem(SdRoot, i18nc("@item:inlistbox allow shutdown", "Only Root"));
    sdrcombo->insertItem(SdNone, i18nc("@item:inlistbox allow shutdown", "Nobody"));
    connect(sdrcombo, SIGNAL(activated(int)), SIGNAL(changed()));
    group0->setWhatsThis(i18n(
        "Here you can select who is allowed to shutdown the computer using KDM. "
        "You can specify different values for local (console) and remote displays. "
        "Possible values are:<ul>"
        " <li><em>Everybody:</em> everybody can shutdown the computer using KDM</li>"
        " <li><em>Only root:</em> KDM will only allow shutdown after the user has "
            "entered the root password</li>"
        " <li><em>Nobody:</em> nobody can shutdown the computer using KDM</li></ul>"));


    QGroupBox *group1 = new QGroupBox(i18nc(
        "@title:group shell commands for shutdown", "Commands"), this);

    shutdown_lined = new KUrlRequester(group1);
    QLabel *shutdown_label = new QLabel(i18nc("command for ...", "H&alt:"), group1);
    shutdown_label->setBuddy(shutdown_lined);
    connect(shutdown_lined, SIGNAL(textChanged(QString)),
            SIGNAL(changed()));
    wtstr = i18n("Command to initiate the system halt. Typical value: /sbin/halt");
    shutdown_label->setWhatsThis(wtstr);
    shutdown_lined->setWhatsThis(wtstr);

    restart_lined = new KUrlRequester(group1);
    QLabel *restart_label = new QLabel(i18nc("command for ...", "Reb&oot:"), group1);
    restart_label->setBuddy(restart_lined);
    connect(restart_lined, SIGNAL(textChanged(QString)),
            SIGNAL(changed()));
    wtstr = i18n("Command to initiate the system reboot. Typical value: /sbin/reboot");
    restart_label->setWhatsThis(wtstr);
    restart_lined->setWhatsThis(wtstr);


    QGroupBox *group4 = new QGroupBox(i18nc("@title:group", "Miscellaneous"), this);

    bm_combo = new KBackedComboBox(group4);
    bm_combo->insertItem("None", i18nc("boot manager", "None"));
    bm_combo->insertItem("Grub", i18n("Grub"));
    bm_combo->insertItem("Grub2", i18n("Grub2"));
#if defined(__linux__) && (defined(__i386__) || defined(__amd64__))
    bm_combo->insertItem("Lilo", i18n("Lilo"));
#endif
    QLabel *bm_label = new QLabel(i18n("Boot manager:"), group4);
    bm_label->setBuddy(bm_combo);
    connect(bm_combo, SIGNAL(activated(int)), SIGNAL(changed()));
    wtstr = i18n("Enable boot options in the \"Shutdown...\" dialog.");
    bm_label->setWhatsThis(wtstr);
    bm_combo->setWhatsThis(wtstr);

    QBoxLayout *main = new QVBoxLayout(this);
    main->setMargin(10);
    QGridLayout *lgroup0 = new QGridLayout(group0);
    lgroup0->setSpacing(10);
    QGridLayout *lgroup1 = new QGridLayout(group1);
    lgroup1->setSpacing(10);
    QGridLayout *lgroup4 = new QGridLayout(group4);
    lgroup4->setSpacing(10);

    main->addWidget(group0);
    main->addWidget(group1);
    main->addWidget(group4);
    main->addStretch();

    lgroup0->setColumnMinimumWidth(2, KDialog::spacingHint() * 2);
    lgroup0->setColumnStretch(1, 1);
    lgroup0->setColumnStretch(4, 1);
    lgroup0->addWidget(sdllabel, 1, 0);
    lgroup0->addWidget(sdlcombo, 1, 1);
    lgroup0->addWidget(sdrlabel, 1, 3);
    lgroup0->addWidget(sdrcombo, 1, 4);

    lgroup1->setColumnMinimumWidth(2, KDialog::spacingHint() * 2);
    lgroup1->setColumnStretch(1, 1);
    lgroup1->setColumnStretch(4, 1);
    lgroup1->addWidget(shutdown_label, 1, 0);
    lgroup1->addWidget(shutdown_lined, 1, 1);
    lgroup1->addWidget(restart_label, 1, 3);
    lgroup1->addWidget(restart_lined, 1, 4);

    lgroup4->addWidget(bm_label, 1, 0);
    lgroup4->addWidget(bm_combo, 1, 1);
    lgroup4->setColumnStretch(2, 1);

    main->activate();

}

void KDMSessionsWidget::writeSD(KComboBox *combo, KConfigGroup group)
{
    QString what;
    switch (combo->currentIndex()) {
    case SdAll: what = "All"; break;
    case SdRoot: what = "Root"; break;
    default: what = "None"; break;
    }
    group.writeEntry("AllowShutdown", what);
}

void KDMSessionsWidget::save()
{
    writeSD(sdlcombo, config->group("X-:*-Core"));

    writeSD(sdrcombo, config->group("X-*-Core"));

    KConfigGroup configGrp = config->group("Shutdown");
    configGrp.writeEntry("HaltCmd", shutdown_lined->url().path(), KConfig::Persistent);
    configGrp.writeEntry("RebootCmd", restart_lined->url().path(), KConfig::Persistent);

    configGrp.writeEntry("BootManager", bm_combo->currentId());
}

void KDMSessionsWidget::readSD(KComboBox *combo, const QString &def, KConfigGroup group)
{
    QString str = group.readEntry("AllowShutdown", def);
    SdModes sdMode;
    if (str == "All")
        sdMode = SdAll;
    else if (str == "Root")
        sdMode = SdRoot;
    else
        sdMode = SdNone;
    combo->setCurrentIndex(sdMode);
}

void KDMSessionsWidget::load()
{
    readSD(sdlcombo, "All", config->group("X-:*-Core"));

    readSD(sdrcombo, "Root", config->group("X-*-Core"));

    KConfigGroup configGrp = config->group("Shutdown");
    restart_lined->setUrl(configGrp.readEntry("RebootCmd", REBOOT_CMD));
    shutdown_lined->setUrl(configGrp.readEntry("HaltCmd", HALT_CMD));

    bm_combo->setCurrentId(configGrp.readEntry("BootManager", "None"));
}

void KDMSessionsWidget::defaults()
{
    restart_lined->setUrl(KUrl(REBOOT_CMD));
    shutdown_lined->setUrl(KUrl(HALT_CMD));

    sdlcombo->setCurrentIndex(SdAll);
    sdrcombo->setCurrentIndex(SdRoot);

    bm_combo->setCurrentId("None");
}

#include "kdm-shut.moc"
