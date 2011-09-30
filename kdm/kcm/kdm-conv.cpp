/*
    Copyright (C) 2000 Oswald Buddenhagen <ossi@kde.org>
    Based on several other files.

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

#include "kdm-conv.h"

#include <KColorScheme>
#include <KComboBox>
#include <KDialog>
#include <KLocale>
#include <KConfig>
#include <KConfigGroup>

#include <QButtonGroup>
#include <QCheckBox>
#include <QGroupBox>
#include <QLabel>
#include <QListWidget>
#include <QRadioButton>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>

extern KConfig *config;


KDMConvenienceWidget::KDMConvenienceWidget(QWidget *parent)
    : QWidget(parent)
{
    QString wtstr;

    QLabel *paranoia = new QLabel(
        i18n("<big><b><center>Attention<br/>"
             "Read help</center></b></big>"), this);
    QPalette p;
    p.setBrush(QPalette::WindowText,
        KColorScheme(QPalette::Active, KColorScheme::Window)
            .foreground(KColorScheme::NegativeText));
    paranoia->setPalette(p);

    QSizePolicy vpref(QSizePolicy::Minimum, QSizePolicy::Fixed);

    alGroup = new QGroupBox(i18n("Enable Au&to-Login"), this);
    alGroup->setCheckable(true);
    alGroup->setSizePolicy(vpref);
    QVBoxLayout *laygroup2 = new QVBoxLayout(alGroup);
    laygroup2->setSpacing(KDialog::spacingHint());

    alGroup->setWhatsThis(i18n("Turn on the auto-login feature."
                               " This applies only to KDM's graphical login."
                               " Think twice before enabling this!"));
    connect(alGroup, SIGNAL(toggled(bool)), SIGNAL(changed()));

    userlb = new KComboBox(alGroup);

    QLabel *u_label = new QLabel(i18n("Use&r:"), alGroup);
    u_label->setBuddy(userlb);
    QHBoxLayout *hlpl1 = new QHBoxLayout();
    laygroup2->addItem(hlpl1);
    hlpl1->setSpacing(KDialog::spacingHint());
    hlpl1->addWidget(u_label);
    hlpl1->addWidget(userlb);
    hlpl1->addStretch(1);
    connect(userlb, SIGNAL(highlighted(int)), SIGNAL(changed()));
    wtstr = i18n("Select the user to be logged in automatically.");
    u_label->setWhatsThis(wtstr);
    userlb->setWhatsThis(wtstr);
    autoLockCheck = new QCheckBox(i18n("Loc&k session"), alGroup);
    laygroup2->addWidget(autoLockCheck);
    connect(autoLockCheck, SIGNAL(toggled(bool)), SIGNAL(changed()));
    autoLockCheck->setWhatsThis(i18n(
        "The automatically started session "
        "will be locked immediately (provided it is a KDE session). This can "
        "be used to obtain a super-fast login restricted to one user."));

    puGroup = new QGroupBox(i18nc("@title:group", "Preselect User"), this);

    puGroup->setSizePolicy(vpref);

    npRadio = new QRadioButton(i18nc("@option:radio preselected user", "&None"), puGroup);
    ppRadio = new QRadioButton(i18nc("@option:radio preselected user", "Prev&ious"), puGroup);
    ppRadio->setWhatsThis(i18n(
        "Preselect the user that logged in previously. "
        "Use this if this computer is usually used several consecutive times by one user."));
    spRadio = new QRadioButton(i18nc("@option:radio preselected user", "Specifi&ed:"), puGroup);
    spRadio->setWhatsThis(i18n(
        "Preselect the user specified in the combo box to the right. "
        "Use this if this computer is predominantly used by a certain user."));
    QButtonGroup *buttonGroup = new QButtonGroup(puGroup);
    connect(buttonGroup, SIGNAL(buttonClicked(int)), SLOT(slotPresChanged()));
    connect(buttonGroup, SIGNAL(buttonClicked(int)), SIGNAL(changed()));
    buttonGroup->addButton(npRadio);
    buttonGroup->addButton(ppRadio);
    buttonGroup->addButton(spRadio);
    QVBoxLayout *laygroup5 = new QVBoxLayout(puGroup);
    laygroup5->setSpacing(KDialog::spacingHint());
    laygroup5->addWidget(npRadio);
    laygroup5->addWidget(ppRadio);

    puserlb = new KComboBox(true, puGroup);

    connect(puserlb, SIGNAL(editTextChanged(QString)), SIGNAL(changed()));
    wtstr = i18n(
        "Select the user to be preselected for login. "
        "This box is editable, so you can specify an arbitrary non-existent "
        "user to mislead possible attackers.");
    puserlb->setWhatsThis(wtstr);
    QBoxLayout *hlpl = new QHBoxLayout();
    laygroup5->addItem(hlpl);
    hlpl->setSpacing(KDialog::spacingHint());
    hlpl->setMargin(0);
    hlpl->addWidget(spRadio);
    hlpl->addWidget(puserlb);
    hlpl->addStretch(1);
    // This is needed before the abuse below to ensure the combo is enabled in time
    connect(spRadio, SIGNAL(clicked(bool)), SLOT(slotPresChanged()));
    // Abuse the radio button text as a label for the combo
    connect(spRadio, SIGNAL(clicked(bool)), puserlb, SLOT(setFocus()));
    cbjumppw = new QCheckBox(i18nc("@option:check action", "Focus pass&word"), puGroup);
    laygroup5->addWidget(cbjumppw);
    cbjumppw->setWhatsThis(i18n(
        "When this option is on, KDM will place the cursor "
        "in the password field instead of the user field after preselecting a user. "
        "Use this to save one key press per login, if the preselection usually "
        "does not need to be changed."));
    connect(cbjumppw, SIGNAL(toggled(bool)), SIGNAL(changed()));

    npGroup = new QGroupBox(i18n("Enable Password-&Less Logins"), this);
    QVBoxLayout *laygroup3 = new QVBoxLayout(npGroup);
    laygroup3->setSpacing(KDialog::spacingHint());

    npGroup->setCheckable(true);

    npGroup->setWhatsThis(i18n(
        "When this option is checked, the checked users from "
        "the list below will be allowed to log in without entering their "
        "password. This applies only to KDM's graphical login. "
        "Think twice before enabling this!"));

    connect(npGroup, SIGNAL(toggled(bool)), SIGNAL(changed()));

    QLabel *pl_label = new QLabel(i18n("No password re&quired for:"), npGroup);
    laygroup3->addWidget(pl_label);
    npuserlv = new QListWidget(npGroup);
    laygroup3->addWidget(npuserlv);
    pl_label->setBuddy(npuserlv);
    npuserlv->setWhatsThis(i18n(
        "Check all users you want to allow a password-less login for. "
        "Entries denoted with '@' are user groups. Checking a group is like "
        "checking all users in that group."));

    btGroup = new QGroupBox(i18nc("@title:group", "Miscellaneous"), this);
    QVBoxLayout *laygroup4 = new QVBoxLayout(btGroup);
    laygroup4->setSpacing(KDialog::spacingHint());

    cbarlen = new QCheckBox(i18n("Automatically log in again after &X server crash"), btGroup);
    cbarlen->setWhatsThis(i18n(
        "When this option is on, a user will be "
        "logged in again automatically when their session is interrupted by an "
        "X server crash; note that this can open a security hole: if you use "
        "a screen locker than KDE's integrated one, this will make "
        "circumventing a password-secured screen lock possible."));
    //TODO a screen locker _other_ than
    laygroup4->addWidget(cbarlen);
    connect(cbarlen, SIGNAL(toggled(bool)), SIGNAL(changed()));

    QGridLayout *main = new QGridLayout(this);
    main->setSpacing(10);
    main->addWidget(paranoia, 0, 0);
    main->addWidget(alGroup, 1, 0);
    main->addWidget(puGroup, 2, 0);
    main->addWidget(npGroup, 0, 1, 4, 1);
    main->addWidget(btGroup, 4, 0, 1, 2);
    main->setColumnStretch(0, 1);
    main->setColumnStretch(1, 2);
    main->setRowStretch(3, 1);

    connect(userlb, SIGNAL(activated(QString)),
            SLOT(slotSetAutoUser(QString)));
    connect(puserlb, SIGNAL(editTextChanged(QString)),
            SLOT(slotSetPreselUser(QString)));
    connect(npuserlv, SIGNAL(itemClicked(QListWidgetItem*)),
            SLOT(slotUpdateNoPassUser(QListWidgetItem*)));

}

void KDMConvenienceWidget::slotPresChanged()
{
    puserlb->setEnabled(spRadio->isChecked());
    cbjumppw->setEnabled(!npRadio->isChecked());
}

void KDMConvenienceWidget::save()
{
    KConfigGroup configGrp = config->group("X-:0-Core");
    configGrp.writeEntry("AutoLoginEnable", alGroup->isChecked());
    configGrp.writeEntry("AutoLoginUser", userlb->currentText());
    configGrp.writeEntry("AutoLoginLocked", autoLockCheck->isChecked());

    configGrp = config->group("X-:*-Core");
    configGrp.writeEntry("NoPassEnable", npGroup->isChecked());
    configGrp.writeEntry("NoPassUsers", noPassUsers);

    config->group("X-*-Core").writeEntry("AutoReLogin", cbarlen->isChecked());

    configGrp = config->group("X-:*-Greeter");
    configGrp.writeEntry("PreselectUser",
                         npRadio->isChecked() ? "None" :
                         ppRadio->isChecked() ? "Previous" :
                         "Default");
    configGrp.writeEntry("DefaultUser", puserlb->currentText());
    configGrp.writeEntry("FocusPasswd", cbjumppw->isChecked());
}


void KDMConvenienceWidget::load()
{
    KConfigGroup configGrp = config->group("X-:0-Core");
    bool alenable = configGrp.readEntry("AutoLoginEnable", false);
    autoUser = configGrp.readEntry("AutoLoginUser");
    autoLockCheck->setChecked(configGrp.readEntry("AutoLoginLocked", false));
    if (autoUser.isEmpty())
        alenable = false;
    alGroup->setChecked(alenable);

    configGrp = config->group("X-:*-Core");
    npGroup->setChecked(configGrp.readEntry("NoPassEnable", false));
    noPassUsers = configGrp.readEntry("NoPassUsers", QStringList());

    cbarlen->setChecked(config->group("X-*-Core").readEntry("AutoReLogin", false));

    configGrp = config->group("X-:*-Greeter");
    QString presstr = configGrp.readEntry("PreselectUser", "None");
    if (presstr == "Previous")
        ppRadio->setChecked(true);
    else if (presstr == "Default")
        spRadio->setChecked(true);
    else
        npRadio->setChecked(true);
    preselUser = configGrp.readEntry("DefaultUser");
    cbjumppw->setChecked(configGrp.readEntry("FocusPasswd", false));

    slotPresChanged();
}


void KDMConvenienceWidget::defaults()
{
    alGroup->setChecked(false);
    autoLockCheck->setChecked(false);
    npRadio->setChecked(true);
    npGroup->setChecked(false);
    cbarlen->setChecked(false);
    cbjumppw->setChecked(false);
    autoUser = "";
    preselUser = "";
    noPassUsers.clear();

    slotPresChanged();
}


void KDMConvenienceWidget::slotSetAutoUser(const QString &user)
{
    autoUser = user;
}

void KDMConvenienceWidget::slotSetPreselUser(const QString &user)
{
    preselUser = user;
}

void KDMConvenienceWidget::slotUpdateNoPassUser(QListWidgetItem *item)
{
    if (!item)
        return;
    int ind = noPassUsers.indexOf(item->text());
    if (item->checkState() == Qt::Checked) {
        if (ind < 0) {
            noPassUsers.append(item->text());
            emit changed();
        }
    } else {
        if (ind >= 0) {
            noPassUsers.removeAt(ind);
            emit changed();
        }
    }
}

void KDMConvenienceWidget::slotClearUsers()
{
    userlb->clear();
    puserlb->clear();
    npuserlv->clear();
    if (!autoUser.isEmpty())
        userlb->addItem(autoUser);
    if (!preselUser.isEmpty())
        puserlb->addItem(preselUser);
}

void KDMConvenienceWidget::slotAddUsers(const QMap<QString, int> &users)
{
    QMap<QString, int>::const_iterator it;
    for (it = users.begin(); it != users.end(); ++it) {
        if (it.value() > 0) {
            if (it.key() != autoUser)
                userlb->addItem(it.key());
            if (it.key() != preselUser)
                puserlb->addItem(it.key());
        }
        if (it.value() != 0) {
            QListWidgetItem *item = new QListWidgetItem(it.key(), npuserlv);
            item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled |
                           Qt::ItemIsUserCheckable);
            item->setCheckState(noPassUsers.contains(it.key()) ?
                                Qt::Checked : Qt::Unchecked);
        }
    }

    if (userlb->model())
        userlb->model()->sort(0);

    if (puserlb->model())
        puserlb->model()->sort(0);

    npuserlv->sortItems();

    userlb->setCurrentItem(autoUser);
    puserlb->setCurrentItem(preselUser);
}

void KDMConvenienceWidget::slotDelUsers(const QMap<QString, int> &users)
{
    QMap<QString, int>::const_iterator it;
    for (it = users.begin(); it != users.end(); ++it) {
        if (it.value() > 0) {
            int idx = userlb->findText(it.key());
            if (it.key() != autoUser && idx != -1)
                userlb->removeItem(idx);
            idx = puserlb->findText(it.key());
            if (it.key() != preselUser && idx != -1)
                puserlb->removeItem(idx);
        }
        if (it.value() != 0)
            delete npuserlv->findItems(it.key(), Qt::MatchExactly).first();
    }
}

#include "kdm-conv.moc"
