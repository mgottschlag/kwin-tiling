/*
 *  Copyright (c) 2000 Matthias Elter <elter@kde.org>
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
 */

#include <qwhatsthis.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qstring.h>
#include <qpushbutton.h>
#include <qpixmap.h>
#include <qimage.h>
#include <qslider.h>

#include <kconfig.h>
#include <kcombobox.h>
#include <kglobal.h>
#include <klocale.h>
#include <klineedit.h>
#include <kfiledialog.h>
#include <kmessagebox.h>
#include <kimageio.h>

#include "main.h"
#include "lookandfeeltab_impl.h"
#include "lookandfeeltab_impl.moc"


extern int kickerconfig_screen_number;


LookAndFeelTab::LookAndFeelTab( KickerConfig *parent, const char* name )
  : LookAndFeelTabBase (parent, name)
{
    kconf = parent;
    // connections
    connect(m_manualHideAnimation, SIGNAL(clicked()), SIGNAL(changed()));
    connect(m_manualHideSlider, SIGNAL(valueChanged(int)), SIGNAL(changed()));
    connect(m_autoHideAnimation, SIGNAL(clicked()), SIGNAL(changed()));
    connect(m_autoHideSlider, SIGNAL(valueChanged(int)), SIGNAL(changed()));
    connect(m_hideButtons, SIGNAL(activated(int)), SIGNAL(changed()));
    connect(m_hideButtonSlider, SIGNAL(valueChanged(int)), SIGNAL(changed()));
    connect(m_backgroundImage, SIGNAL(clicked()), SIGNAL(changed()));
    connect(m_backgroundButton, SIGNAL(clicked()), SLOT(browse_theme()));

    // whats this help
    QWhatsThis::add(m_manualHideAnimation, i18n("If hide buttons are enabled, check this option to make the "
                                                "panel softly slide away when you click on the hide buttons. "
                                                "Else it will just disappear."));

    QWhatsThis::add(m_manualHideSlider, i18n("Determines the speed of the hide animation, i.e. the "
                                             "animation shown when you click on the panel's hide buttons."));

    QWhatsThis::add(m_autoHideAnimation, i18n("If auto-hide panel is enabled, check this option to make "
                                              "the panel softly slide down after a certain amount of time. "
                                              "Else it will just disappear."));

    QWhatsThis::add(m_autoHideSlider, i18n("Determines the speed of the auto-hide animation, "
                                           "i.e. the animation shown when the panel disappears after "
                                           "a certain amount of time."));

    QWhatsThis::add(m_hideButtons, i18n("If this option is enabled, the panel"
                                        " will have buttons on both ends that can be used to hide it. The"
                                        " panel will slide away, leaving more room for applications. There"
                                        " only remains a small button which can be used to show the panel "
                                        "again."));

    QWhatsThis::add(m_hideButtonSlider, i18n("Here you can change the size of the hide buttons."));

    QWhatsThis::add(m_backgroundImage, i18n("If this option is selected, you "
                                            "can choose a background image that will be used to display the "
                                            "panel. If it is not selected, the default colors will be used, "
                                            "see the 'Colors' control module."));

    QWhatsThis::add(m_backgroundLabel, i18n("This is a preview for the selected background image."));

    QString wtstr = i18n("Here you can choose a theme to be displayed by the panel. "
                         "Press the 'Browse' button to choose a theme using the file dialog.<p> "
                         "This option is only active if 'Use background theme' is selected.");
    QWhatsThis::add(m_backgroundButton, wtstr );
    QWhatsThis::add(m_backgroundInput, wtstr );

    m_backgroundInput->setReadOnly(true);

    load();
}

void LookAndFeelTab::browse_theme()
{
    QString newtheme = KFileDialog::getOpenFileName(QString::null
                                                    , KImageIO::pattern(KImageIO::Reading)
                                                    , 0, i18n("Select an image file"));
    if (theme == newtheme) return;
    if (newtheme.isEmpty()) return;

    QImage tmpImg(newtheme);
    if( !tmpImg.isNull() ) {
        tmpImg = tmpImg.smoothScale(m_backgroundLabel->contentsRect().width(),
                                    m_backgroundLabel->contentsRect().height());
        theme_preview.convertFromImage(tmpImg);
        if( !theme_preview.isNull() ) {
            theme = newtheme;
            m_backgroundInput->setText(theme);
            m_backgroundLabel->setPixmap(theme_preview);
            emit changed();
            return;
        }
    }

    KMessageBox::error(this, i18n("Failed to load image file."), i18n("Failed to load image file."));
}

void LookAndFeelTab::load()
{
    QCString configname;
    if (kickerconfig_screen_number == 0)
	configname = "kickerrc";
    else
	configname.sprintf("kicker-screen-%drc", kickerconfig_screen_number);
    KConfig *c = new KConfig(configname, false, false);

    c->setGroup("General");

    bool use_theme = c->readBoolEntry("UseBackgroundTheme", false);
    theme = c->readEntry("BackgroundTheme", QString::null);

    m_backgroundImage->setChecked(use_theme);
    m_backgroundInput->setEnabled(use_theme);
    m_backgroundLabel->setEnabled(use_theme);
    m_backgroundButton->setEnabled(use_theme);

    if (theme != QString::null) {
        QImage tmpImg(theme);
        if(!tmpImg.isNull()) {
            tmpImg = tmpImg.smoothScale(m_backgroundLabel->contentsRect().width(),
                                        m_backgroundLabel->contentsRect().height());
            theme_preview.convertFromImage(tmpImg);
            if(!theme_preview.isNull()) {
                m_backgroundInput->setText(theme);
                m_backgroundLabel->setPixmap(theme_preview);
            }
            else
                m_backgroundInput->setText(i18n("Error loading theme image file."));
        }
        else
            m_backgroundInput->setText(i18n("Error loading theme image file."));
    }

    bool hideanim = c->readBoolEntry("HideAnimation", true);
    bool autohideanim = c->readBoolEntry("AutoHideAnimation", true);

    m_manualHideSlider->setValue(c->readNumEntry("HideAnimationSpeed", 100));
    m_autoHideSlider->setValue(c->readNumEntry("AutoHideAnimationSpeed", 25));

    m_manualHideSlider->setEnabled(hideanim);
    m_autoHideSlider->setEnabled(autohideanim);

    m_manualHideAnimation->setChecked(hideanim);
    m_autoHideAnimation->setChecked(autohideanim);

    bool showLHB = c->readBoolEntry("ShowLeftHideButton", true);    
    bool showRHB = c->readBoolEntry("ShowRightHideButton", true);
    
    if (showLHB)
        if (showRHB)
            m_hideButtons->setCurrentItem(0);
        else
            m_hideButtons->setCurrentItem(1);
    else if (showRHB)
        m_hideButtons->setCurrentItem(2);
    else
        m_hideButtons->setCurrentItem(3);

    m_hideButtonSlider->setValue(c->readNumEntry("HideButtonSize", 14));
    m_hideButtonSlider->setEnabled(showLHB || showRHB);

    delete c;
}

void LookAndFeelTab::save()
{
    QCString configname;
    if (kickerconfig_screen_number == 0)
	configname = "kickerrc";
    else
	configname.sprintf("kicker-screen-%drc", kickerconfig_screen_number);
    KConfig *c = new KConfig(configname, false, false);

    c->setGroup("General");

    c->writeEntry("UseBackgroundTheme", m_backgroundImage->isChecked());
    c->writeEntry("BackgroundTheme", theme);
    c->writeEntry("HideAnimation", m_manualHideAnimation->isChecked());
    c->writeEntry("AutoHideAnimation", m_autoHideAnimation->isChecked());
    c->writeEntry("HideAnimationSpeed", m_manualHideSlider->value());
    c->writeEntry("AutoHideAnimationSpeed", m_autoHideSlider->value());
    
    if (m_hideButtons->currentItem() == 0 || m_hideButtons->currentItem() == 1)
        c->writeEntry("ShowLeftHideButton", "true");
    else
        c->writeEntry("ShowLeftHideButton", "false");
    
    if (m_hideButtons->currentItem() == 0 || m_hideButtons->currentItem() == 2)
        c->writeEntry("ShowRightHideButton", "true");
    else
        c->writeEntry("ShowRightHideButton", "false");
    
    c->writeEntry("HideButtonSize", m_hideButtonSlider->value());
    c->sync();

    delete c;
}

void LookAndFeelTab::defaults()
{
    theme = QString::null;

    m_backgroundImage->setChecked(false);
    m_backgroundInput->setText(theme);
    m_backgroundLabel->clear();

    m_backgroundInput->setEnabled(false);
    m_backgroundLabel->setEnabled(false);
    m_backgroundButton->setEnabled(false);

    m_manualHideAnimation->setChecked(true);
    m_autoHideAnimation->setChecked(true);

    m_manualHideSlider->setEnabled(true);
    m_autoHideSlider->setEnabled(true);

    m_manualHideSlider->setValue(100);
    m_autoHideSlider->setValue(25);

    m_hideButtons->setCurrentItem(2);
    m_hideButtonSlider->setValue(14);
}

void LookAndFeelTab::hideButtonsSet(int index)
{
    if (index < 3)
    {
        m_hideButtonSlider->setEnabled(true);
    }
    else
    {
        m_hideButtonSlider->setEnabled(false);
    }
}

void LookAndFeelTab::show()
{
    if (kconf->horizontal())
    {
        m_hideButtons->changeItem(i18n("Enable Left Hide Button Only"), 1);
        m_hideButtons->changeItem(i18n("Enable Right Hide Button Only"), 2);
    }
    else
    {
        qDebug("vertical");
            m_hideButtons->changeItem(i18n("Enable Top Hide Button Only"), 1);
            m_hideButtons->changeItem(i18n("Enable Bottom Hide Button Only"), 2);
    }
    
    QWidget::show();
}
