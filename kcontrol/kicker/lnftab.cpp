/*
 *  lnftab.cpp
 *
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

#include <qlayout.h>
#include <qgroupbox.h>
#include <qwhatsthis.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qhbox.h>
#include <qstring.h>
#include <qpushbutton.h>
#include <qpixmap.h>
#include <qimage.h>

#include <kconfig.h>
#include <kdialog.h>
#include <kglobal.h>
#include <klocale.h>
#include <klineedit.h>
#include <kfiledialog.h>
#include <kmessagebox.h>
#include <kimageio.h>
#include <knuminput.h>

#include "lnftab.h"
#include "lnftab.moc"


LnFTab::LnFTab( QWidget *parent, const char* name )
  : QWidget (parent, name)
  , use_theme(false)
  , theme(QString::null)
{
  layout = new QGridLayout(this, 4, 1,
                           KDialog::marginHint(),
                           KDialog::spacingHint());


  // hide animation group
  hide_group = new QGroupBox(i18n("&Hide Animation"), this);

  QVBoxLayout *vbox = new QVBoxLayout(hide_group, KDialog::marginHint(),
                                      KDialog::spacingHint());
  vbox->addSpacing(fontMetrics().lineSpacing());

  hide_cb = new QCheckBox(i18n("Enabled"), hide_group);
  connect(hide_cb, SIGNAL(clicked()), SLOT(hide_clicked()));
  vbox->addWidget(hide_cb);
  QWhatsThis::add( hide_cb, i18n("If hide buttons are enabled, check this option to make the"
    " panel softly slide away when you click on the hide buttons. Else it will just disappear."));

  hide_input = new KIntNumInput(50, hide_group);
  hide_input->setRange(1, 200, 1, true);
  hide_input->setLabel(i18n("Speed:"), AlignLeft | AlignVCenter);
  connect(hide_input, SIGNAL(valueChanged(int)), SLOT(hide_changed(int)));
  vbox->addWidget(hide_input);
  QWhatsThis::add( hide_input, i18n("Determines the speed of the hide animation, i.e. the"
    " animation shown when you click on the panel's hide buttons.") );

  layout->addWidget(hide_group, 0, 0);

  // auto-hide animation group
  autohide_group = new QGroupBox(i18n("&Auto-Hide Animation"), this);

  vbox = new QVBoxLayout(autohide_group, KDialog::marginHint(),
                                      KDialog::spacingHint());
  vbox->addSpacing(fontMetrics().lineSpacing());

  autohide_cb = new QCheckBox(i18n("Enabled"), autohide_group);
  connect(autohide_cb, SIGNAL(clicked()), SLOT(autohide_clicked()));
  vbox->addWidget(autohide_cb);
  QWhatsThis::add( autohide_cb, i18n("If auto-hide panel is enabled, check this option to make"
    " the panel softly slide down after a certain amount of time. Else it will just disappear."));

  autohide_input = new KIntNumInput(5, autohide_group);
  autohide_input->setRange(1, 50, 1, true);
  autohide_input->setLabel(i18n("Speed:"), AlignLeft | AlignVCenter);
  connect(autohide_input, SIGNAL(valueChanged(int)), SLOT(autohide_changed(int)));
  vbox->addWidget(autohide_input);
  QWhatsThis::add( autohide_input, i18n("Determines the speed of the auto-hide animation,"
    " i.e. the animation shown when the panel disappears after a certain amount of time."));

  layout->addWidget(autohide_group, 1, 0);

  // theme background group
  theme_group = new QGroupBox(i18n("Background Theme"), this);

  vbox = new QVBoxLayout(theme_group,KDialog::marginHint(),
                                       KDialog::spacingHint());
  vbox->addSpacing(fontMetrics().lineSpacing());

  QHBox *hbox1 = new QHBox(theme_group);
  use_theme_cb = new QCheckBox(i18n("Use background theme."), hbox1);
  connect(use_theme_cb, SIGNAL(clicked()), SLOT(use_theme_clicked()));
  theme_label = new QLabel(hbox1);
  theme_label->setFixedSize(50,50);
  theme_label->setFrameStyle(QFrame::Panel | QFrame::Sunken);
  theme_label->setAlignment(AlignCenter);
  hbox1->setSpacing(KDialog::spacingHint());
  hbox1->setStretchFactor(use_theme_cb, 2);
  QWhatsThis::add( use_theme_cb, i18n("If this option is selected, you"
    " can choose an individual 'theme' that will be used to display the"
    " panel. If it is not selected, the default colors will be used,"
    " see the 'Colors' control module.") );
  QWhatsThis::add( theme_label, i18n("This is a preview for the selected"
    " panel theme.") );

  QHBox *hbox2 = new QHBox(theme_group);
  theme_input = new KLineEdit(hbox2);
  theme_input->setReadOnly(true);
  browse_button = new QPushButton(i18n("Browse"), hbox2);
  connect(browse_button, SIGNAL(clicked()), SLOT(browse_theme()));
  QString wtstr = i18n("Here you can choose a theme to be displayed by the panel."
    " Press the 'Browse' button to choose a theme using the file dialog.<p>"
    " This option is only active if 'Use background theme' is selected.");
  QWhatsThis::add( theme_input, wtstr );
  QWhatsThis::add( browse_button, wtstr );

  hbox2->setSpacing(KDialog::spacingHint());
  hbox2->setStretchFactor(theme_input, 2);

  vbox->addWidget(hbox1);
  vbox->addWidget(hbox2);

  layout->addWidget(theme_group, 2, 0);

  // misc group
  misc_group = new QGroupBox(i18n("Miscellaneous"), this);

  vbox = new QVBoxLayout(misc_group, KDialog::marginHint(),
                         KDialog::spacingHint());
  vbox->addSpacing(fontMetrics().lineSpacing());


  fade_out_cb = new QCheckBox(i18n("Fade out applet handels."), misc_group);
  connect(fade_out_cb, SIGNAL(clicked()), SIGNAL(changed()));
  vbox->addWidget(fade_out_cb);
  QWhatsThis::add( fade_out_cb, i18n("If this option is enabled, the handle on the left"
    " of a panel applet will only be shown when the mouse cursor is moved over it."));

  layout->addWidget(misc_group, 3, 0);

  load();
}

void LnFTab::use_theme_clicked()
{
  use_theme = use_theme_cb->isChecked();
  
  theme_input->setEnabled(use_theme);
  theme_label->setEnabled(use_theme);
  theme_input->setEnabled(use_theme);
  browse_button->setEnabled(use_theme);

  emit changed();
}

void LnFTab::browse_theme()
{
  QString newtheme = KFileDialog::getOpenFileName(QString::null,
                                                  KImageIO::pattern(KImageIO::Reading),
                                                  0,i18n("Select a image file"));
  if (theme == newtheme) return;
  if (newtheme.isEmpty()) return;

  QImage tmpImg(newtheme);
  if( !tmpImg.isNull() ) {
    tmpImg = tmpImg.smoothScale(theme_label->contentsRect().width(),theme_label->contentsRect().height());
    theme_preview.convertFromImage(tmpImg);
    if( !theme_preview.isNull() ) {
      theme = newtheme;
      theme_input->setText(theme);
      theme_label->setPixmap(theme_preview);
      emit changed();
      return;
    }
  }

  KMessageBox::error(this, i18n("Failed to load image file."), i18n("Failed to load image file."));
}

void LnFTab::hide_clicked()
{
  hide_input->setEnabled(hide_cb->isChecked());
  emit changed();
}

void LnFTab::autohide_clicked()
{
  autohide_input->setEnabled(autohide_cb->isChecked());
  emit changed();
}

void LnFTab::hide_changed(int)
{
  emit changed();
}
void LnFTab::autohide_changed(int)
{
  emit changed();
}

void LnFTab::load()
{
  KConfig *c = new KConfig("kickerrc", false, false);
  
  c->setGroup("panel");

  use_theme = c->readBoolEntry("UseBackgroundTheme", false);
  theme = c->readEntry("BackgroundTheme", QString::null);

  use_theme_cb->setChecked(use_theme);

  theme_input->setEnabled(use_theme);
  theme_label->setEnabled(use_theme);
  theme_input->setEnabled(use_theme);
  browse_button->setEnabled(use_theme);

  if (theme != QString::null) {
    QImage tmpImg(theme);
    if(!tmpImg.isNull()) {
      tmpImg = tmpImg.smoothScale(theme_label->contentsRect().width(),
                                  theme_label->contentsRect().height());
      theme_preview.convertFromImage(tmpImg);
      if(!theme_preview.isNull()) {
        theme_input->setText(theme);
        theme_label->setPixmap(theme_preview);
      }
      else
        theme_input->setText(i18n("Error loading theme image file."));
    }
    else
      theme_input->setText(i18n("Error loading theme image file."));
  }

  bool hideanim = c->readBoolEntry("HideAnimation", true);
  bool autohideanim = c->readBoolEntry("AutoHideAnimation", true);
  
  hide_input->setValue(c->readNumEntry("HideAnimationSpeed", 50));
  autohide_input->setValue(c->readNumEntry("AutoHideAnimationSpeed", 50));
  
  hide_input->setEnabled(hideanim);
  autohide_input->setEnabled(autohideanim);

  hide_cb->setChecked(hideanim);
  autohide_cb->setChecked(autohideanim);

  fade_out_cb->setChecked(c->readBoolEntry("FadeOutAppletHandles", false));

  delete c;
}

void LnFTab::save()
{
  KConfig *c = new KConfig("kickerrc", false, false);
  
  c->setGroup("panel");

  c->writeEntry("UseBackgroundTheme", use_theme);
  c->writeEntry("BackgroundTheme", theme);
  c->writeEntry("HideAnimation", hide_cb->isChecked());
  c->writeEntry("AutoHideAnimation", autohide_cb->isChecked());
  c->writeEntry("HideAnimationSpeed", hide_input->value());
  c->writeEntry("AutoHideAnimationSpeed", autohide_input->value());
  c->writeEntry("FadeOutAppletHandles", fade_out_cb->isChecked());
  c->sync();

  delete c;
}

void LnFTab::defaults()
{
  use_theme = false;
  theme = QString::null;

  use_theme_cb->setChecked(use_theme);
  theme_input->setText(theme);
  theme_label->clear();

  theme_input->setEnabled(use_theme);
  theme_label->setEnabled(use_theme);
  theme_input->setEnabled(use_theme);
  browse_button->setEnabled(use_theme);

  hide_cb->setChecked(true);
  autohide_cb->setChecked(true);

  hide_input->setEnabled(true);
  autohide_input->setEnabled(true);

  hide_input->setValue(50);
  autohide_input->setValue(5);

  fade_out_cb->setChecked(false);
}

QString LnFTab::quickHelp()
{
  return i18n("");
}
