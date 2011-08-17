/*
    Copyright (C) 1997 Thomas Tanghus (tanghus@earthling.net)

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

#include "kdm-gen.h"

#include "kbackedcombobox.h"

#include <KDialog>
#include <KFontRequester>
#include <KLocale>
#include <KConfig>
#include <KConfigGroup>
#include <KStandardDirs>
#include <KLanguageButton>

#include <QCheckBox>
#include <QGroupBox>
#include <QPushButton>
#include <QFormLayout>
#include <QVBoxLayout>

extern KConfig *config;

KDMGeneralWidget::KDMGeneralWidget(QWidget *parent)
    : QWidget(parent)
{
    QString wtstr;

    QBoxLayout *ml = new QVBoxLayout(this);

    QGroupBox *box = new QGroupBox(i18nc("@title:group 'man locale' ...", "Locale"), this);
    ml->addWidget(box);
    QFormLayout *fl = new QFormLayout(box);

    // The Language group box
    langcombo = new KLanguageButton(box);
    langcombo->showLanguageCodes(true);
    langcombo->loadAllLanguages();
    connect(langcombo, SIGNAL(activated(QString)), SIGNAL(changed()));
    fl->addRow(i18n("&Language:"), langcombo);
    wtstr = i18n(
        "Here you can choose the language used by KDM. This setting does not "
        "affect a user's personal settings; that will take effect after login.");
    langcombo->setWhatsThis(wtstr);

    QBoxLayout *mlml = new QHBoxLayout();
    ml->addItem(mlml);

    box = new QGroupBox(i18nc("@title:group", "Appearance"), this);
    mlml->addWidget(box);

    fl = new QFormLayout(box);

    useThemeCheck = new QCheckBox(i18n("&Use themed greeter\n(Warning: poor accessibility)"), box);
    connect(useThemeCheck, SIGNAL(toggled(bool)), SLOT(slotUseThemeChanged()));
    useThemeCheck->setWhatsThis(i18n(
        "Enable this if you would like to use a themed Login Manager.<br>"
        "Note that the themed greeter is challenged accessibility-wise (keyboard usage), "
        "and themes may lack support for features like a user list or alternative "
        "authentication methods."));
    fl->addRow(useThemeCheck);

    guicombo = new KBackedComboBox(box);
    guicombo->insertItem("", i18n("<placeholder>default</placeholder>"));
    loadGuiStyles(guicombo);
    guicombo->model()->sort(0);

    connect(guicombo, SIGNAL(activated(int)), SIGNAL(changed()));
    fl->addRow(i18n("GUI s&tyle:"), guicombo);
    wtstr = i18n(
        "You can choose a basic GUI style here that will be used by KDM only.");
    guicombo->setWhatsThis(wtstr);

    colcombo = new KBackedComboBox(box);
    colcombo->insertItem("", i18n("<placeholder>default</placeholder>"));
    loadColorSchemes(colcombo);
    colcombo->model()->sort(0);
    connect(colcombo, SIGNAL(activated(int)), SIGNAL(changed()));
    fl->addRow(i18n("Color sche&me:"), colcombo);
    wtstr = i18n(
        "You can choose a basic Color Scheme here that will be used by KDM only.");
    colcombo->setWhatsThis(wtstr);

    box = new QGroupBox(i18nc("@title:group", "Fonts"), this);
    mlml->addSpacing(KDialog::spacingHint());
    mlml->addWidget(box);
    fl = new QFormLayout(box);

    stdFontChooser = new KFontRequester(box);
    stdFontChooser->setWhatsThis(i18n(
        "This changes the font which is used for all the text in the login manager "
        "except for the greeting and failure messages."));
    connect(stdFontChooser, SIGNAL(fontSelected(QFont)), SIGNAL(changed()));
    fl->addRow(i18nc("... font", "&General:"), stdFontChooser);

    failFontChooser = new KFontRequester(box);
    failFontChooser->setWhatsThis(i18n(
        "This changes the font which is used for failure messages in the login manager."));
    connect(failFontChooser, SIGNAL(fontSelected(QFont)), SIGNAL(changed()));
    fl->addRow(i18nc("font for ...", "&Failure:"), failFontChooser);

    greetingFontChooser = new KFontRequester(box);
    greetingFontChooser->setWhatsThis(i18n(
        "This changes the font which is used for the login manager's greeting."));
    connect(greetingFontChooser, SIGNAL(fontSelected(QFont)), SIGNAL(changed()));
    fl->addRow(i18nc("font for ...", "Gree&ting:"), greetingFontChooser);

    aacb = new QCheckBox(i18n("Use anti-aliasing for fonts"), box);
    aacb->setWhatsThis(i18n(
        "If you check this box and your X-Server has the Xft extension, "
        "fonts will be antialiased (smoothed) in the login dialog."));
    connect(aacb, SIGNAL(toggled(bool)), SIGNAL(changed()));
    fl->addRow(aacb);

    ml->addStretch(1);
}

void KDMGeneralWidget::loadColorSchemes(KBackedComboBox *combo)
{
    // XXX: Global + local schemes
    const QStringList list = KGlobal::dirs()->
        findAllResources("data", "color-schemes/*.colors", KStandardDirs::NoDuplicates);
    for (QStringList::ConstIterator it = list.constBegin(); it != list.constEnd(); ++it) {
        KConfig _config(*it, KConfig::SimpleConfig);
        KConfigGroup config(&_config, "General");

        QString str;
        if (!(str = config.readEntry("Name")).isEmpty()) {
            QString str2 = (*it).mid((*it).lastIndexOf('/') + 1); // strip off path
            str2.chop(7); // strip off ".colors"
            combo->insertItem(str2, str);
        }
    }
}

void KDMGeneralWidget::loadGuiStyles(KBackedComboBox *combo)
{
    // XXX: Global + local schemes
    const QStringList list = KGlobal::dirs()->
        findAllResources("data", "kstyle/themes/*.themerc", KStandardDirs::NoDuplicates);
    for (QStringList::ConstIterator it = list.constBegin(); it != list.constEnd(); ++it) {
        KConfig config(*it, KConfig::SimpleConfig);

        if (!(config.hasGroup("KDE") && config.hasGroup("Misc")))
            continue;

        if (config.group("Desktop Entry").readEntry("Hidden" , false))
            continue;

        QString str2 = config.group("KDE").readEntry("WidgetStyle");
        if (str2.isNull())
            continue;

        combo->insertItem(str2, config.group("Misc").readEntry("Name"));
    }
}

void KDMGeneralWidget::set_def()
{
    stdFontChooser->setFont(QFont("Sans Serif", 10));
    failFontChooser->setFont(QFont("Sans Serif", 10, QFont::Bold));
    greetingFontChooser->setFont(QFont("Serif", 20));
}

void KDMGeneralWidget::save()
{
    KConfigGroup configGrp = config->group("X-*-Greeter");

    configGrp.writeEntry("UseTheme", useThemeCheck->isChecked());
    configGrp.writeEntry("GUIStyle", guicombo->currentId());
    configGrp.writeEntry("ColorScheme", colcombo->currentId());
    configGrp.writeEntry("Language", langcombo->current());
    configGrp.writeEntry("StdFont", stdFontChooser->font());
    configGrp.writeEntry("GreetFont", greetingFontChooser->font());
    configGrp.writeEntry("FailFont", failFontChooser->font());
    configGrp.writeEntry("AntiAliasing", aacb->isChecked());
}


void KDMGeneralWidget::load()
{
    set_def();

    KConfigGroup configGrp = config->group("X-*-Greeter");

    useThemeCheck->setChecked(configGrp.readEntry("UseTheme", false));

    // Check the GUI type
    guicombo->setCurrentId(configGrp.readEntry("GUIStyle"));

    // Check the Color Scheme
    colcombo->setCurrentId(configGrp.readEntry("ColorScheme"));

    // get the language
    langcombo->setCurrentItem(configGrp.readEntry("Language", "C"));

    // Read the fonts
    QFont font = stdFontChooser->font();
    stdFontChooser->setFont(configGrp.readEntry("StdFont", font));
    font = failFontChooser->font();
    failFontChooser->setFont(configGrp.readEntry("FailFont", font));
    font = greetingFontChooser->font();
    greetingFontChooser->setFont(configGrp.readEntry("GreetFont", font));

    aacb->setChecked(configGrp.readEntry("AntiAliasing", false));
}


void KDMGeneralWidget::defaults()
{
    useThemeCheck->setChecked(true);
    guicombo->setCurrentId("");
    colcombo->setCurrentId("");
    langcombo->setCurrentItem("en_US");
    set_def();
    aacb->setChecked(false);
}

void KDMGeneralWidget::slotUseThemeChanged()
{
    bool en = useThemeCheck->isChecked();
    failFontChooser->setEnabled(!en);
    greetingFontChooser->setEnabled(!en);
    emit useThemeChanged(en);
    emit changed();
}

#include "kdm-gen.moc"
