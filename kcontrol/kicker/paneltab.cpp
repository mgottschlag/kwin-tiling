/*
 *  paneltab.cpp
 *
 *  Copyright (c) 2000 Matthias Elter <elter@kde.org>
 *  Copyright (c) 2000 Preston Brown <pbrown@kde.org>
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
#include <qbuttongroup.h>
#include <qradiobutton.h>
#include <qcheckbox.h>
#include <qwhatsthis.h>
#include <qslider.h>
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

#include "paneltab.h"
#include "paneltab.moc"

PanelTab::PanelTab( QWidget *parent, const char* name )
  : QWidget (parent, name)
  , position(Bottom)
  , size(Normal)
  , showHBs(true)
  , highlightHBs(true)
  , HBwidth(10)
  , use_theme(false)
  , theme(QString::null)
{
  int i;

  layout = new QGridLayout(this, 2, 2, 
			   KDialog::marginHint(),
			   KDialog::spacingHint());
  
  // position button group
  pos_group = new QButtonGroup(i18n("Location"), this);

  QWhatsThis::add( pos_group, i18n("This sets the position of the panel"
    " i.e. the screen border it is attached to. You can also change this"
    " position by left-clicking on some free space on the panel and"
    " dragging it to a screen border.") );

  QVBoxLayout *vbox = new QVBoxLayout(pos_group, KDialog::marginHint(),
				      KDialog::spacingHint());
  vbox->addSpacing(fontMetrics().lineSpacing());

  pos_buttons[0] = new QRadioButton( i18n("&Left"), pos_group );
  pos_buttons[1] = new QRadioButton( i18n("&Right"), pos_group );
  pos_buttons[2] = new QRadioButton( i18n("&Top"), pos_group);
  pos_buttons[3] = new QRadioButton( i18n("&Bottom"), pos_group);
  connect(pos_group, SIGNAL(clicked(int)), SLOT(position_clicked(int)));
  pos_buttons[position]->setChecked(true);

  for (i = 0; i < 4; i++)
	vbox->addWidget(pos_buttons[i]);
  layout->addWidget(pos_group,0,0);

  // size button group
  size_group = new QButtonGroup(i18n("Size"), this);

  QWhatsThis::add( size_group, i18n("This sets the size of the panel."
    " You can also access this option via the panel context menu, i.e."
    " by right-clicking on some free space on the panel.") );

  vbox = new QVBoxLayout(size_group, KDialog::marginHint(),
			 KDialog::spacingHint());
  vbox->addSpacing(fontMetrics().lineSpacing());

  size_buttons[0] = new QRadioButton( i18n("T&iny"), size_group );
  size_buttons[1] = new QRadioButton( i18n("&Normal"), size_group );
  size_buttons[2] = new QRadioButton( i18n("L&arge"), size_group);
  connect(size_group, SIGNAL(clicked(int)), SLOT(size_clicked(int)));
  size_buttons[size]->setChecked(true);

  for (i = 0; i < 3; i++)
	vbox->addWidget(size_buttons[i]);
  layout->addWidget(size_group, 1, 0);
  
  // hidebutton group
  hb_group = new QGroupBox(i18n("Hide Buttons"), this);

  vbox = new QVBoxLayout(hb_group,KDialog::marginHint(),
			 KDialog::spacingHint());
  vbox->addSpacing(fontMetrics().lineSpacing());

  show_hbs = new QCheckBox(i18n("Show hide buttons."), hb_group);
  connect(show_hbs, SIGNAL(clicked()), SLOT(show_hbs_clicked()));
  show_hbs->setChecked(showHBs);
  QWhatsThis::add( show_hbs, i18n("If this option is selected, the panel"
    " will have buttons on both ends that can be used to hide it. The"
    " panel will slide away, leaving more room for applications. There"
    " only remains a small button which can be used to show the panel again.") );

  highlight_hbs = new QCheckBox(i18n("Highlight hide buttons on mouse over."), hb_group);
  connect(highlight_hbs, SIGNAL(clicked()), SLOT(highlight_hbs_clicked()));
  highlight_hbs->setChecked(highlightHBs);
  QWhatsThis::add( highlight_hbs, i18n("If this option is selected, the"
    " hide buttons are highlighted when the mouse cursor is moved over them.") );

  QHBox *hbox = new QHBox(hb_group);
  hb_size_label = new QLabel(i18n("Size:"), hbox);
  hb_size = new QSlider( 3, 24, 1, HBwidth, Horizontal, hbox);
  connect(hb_size, SIGNAL(valueChanged(int)), SLOT(hbs_size_changed(int)));
  hb_size->setTickmarks(QSlider::Below);
  hb_size->setTracking(true);
  hbox->setSpacing(KDialog::spacingHint());
  hbox->setStretchFactor(hb_size, 2);
  QString wtstr = i18n("Here you can change the size of the hide buttons.");
  QWhatsThis::add( hb_size_label, wtstr );
  QWhatsThis::add( hb_size, wtstr );

  vbox->addWidget(show_hbs);
  vbox->addWidget(highlight_hbs);
  vbox->addWidget(hbox);
  layout->addWidget(hb_group, 0, 1);

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
  theme_label->setFrameStyle(QFrame::WinPanel | QFrame::Sunken);
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
  wtstr = i18n("Here you can choose a theme to be displayed by the panel."
    " Press the 'Browse' button to choose a theme using the file dialog.<p>"
    " This option is only active if 'Use background theme' is selected.");
  QWhatsThis::add( theme_input, wtstr );
  QWhatsThis::add( browse_button, wtstr );

  hbox2->setSpacing(KDialog::spacingHint());
  hbox2->setStretchFactor(theme_input, 2);

  vbox->addWidget(hbox1);
  vbox->addWidget(hbox2);
  layout->addWidget(theme_group, 1, 1);

  load();
}

void PanelTab::position_clicked(int i)
{
  position = static_cast<Position>(i);
  emit changed();
}

void PanelTab::size_clicked(int i)
{
  size = static_cast<Size>(i);
  emit changed();
}

void PanelTab::show_hbs_clicked()
{
  showHBs = show_hbs->isChecked();
  
  highlight_hbs->setEnabled(showHBs);
  hb_size->setEnabled(showHBs);

  emit changed();
}

void PanelTab::highlight_hbs_clicked()
{
  highlightHBs = highlight_hbs->isChecked();
  emit changed();
}

void PanelTab::hbs_size_changed(int i)
{
  HBwidth = i;
  emit changed();
}

void PanelTab::use_theme_clicked()
{
  use_theme = use_theme_cb->isChecked();
  
  theme_input->setEnabled(use_theme);
  theme_label->setEnabled(use_theme);
  theme_input->setEnabled(use_theme);
  browse_button->setEnabled(use_theme);

  emit changed();
}

void PanelTab::browse_theme()
{
  QString newtheme = KFileDialog::getOpenFileName(QString::null,
                                                  KImageIO::pattern(KImageIO::Reading),
                                                  0,i18n("Select a image file"));
  if (theme == newtheme) return;

  QImage tmpImg(newtheme);
  tmpImg = tmpImg.smoothScale(theme_label->contentsRect().width(),theme_label->contentsRect().height());
  theme_preview.convertFromImage(tmpImg);
  if(theme_preview.isNull()) {
    KMessageBox::error(this, i18n("Failed to load image file."), i18n("Failed to load image file."));
    return;
  }
  
  theme = newtheme;
  theme_input->setText(theme);
  theme_label->setPixmap(theme_preview);
  emit changed();
}

PanelTab::~PanelTab( ) {}

void PanelTab::load()
{
  KConfig *c = new KConfig("kickerrc", false, false);
  
  c->setGroup("panel");

  size = static_cast<Size>(c->readNumEntry("Size", Normal));
  size_buttons[size]->setChecked(true);

  position = static_cast<Position>(c->readNumEntry("Position", Bottom));
  pos_buttons[position]->setChecked(true);

  showHBs = c->readBoolEntry("ShowHideButtons", true);
  show_hbs->setChecked(showHBs);

  highlightHBs = c->readBoolEntry("HighlightHideButtons", true);
  highlight_hbs->setChecked(highlightHBs);


  HBwidth = c->readNumEntry("HideButtonSize", 10);
  hb_size->setValue(HBwidth);

  highlight_hbs->setEnabled(showHBs);
  hb_size->setEnabled(showHBs);

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

  delete c;
}

void PanelTab::save()
{
  KConfig *c = new KConfig("kickerrc", false, false);
  
  c->setGroup("panel");

  c->writeEntry("Size", static_cast<int>(size));
  c->writeEntry("Position", static_cast<int>(position));
  c->writeEntry("ShowHideButtons", showHBs);
  c->writeEntry("HighlightHideButtons", highlightHBs);
  c->writeEntry("HideButtonSize", HBwidth);
  c->writeEntry("UseBackgroundTheme", use_theme);
  c->writeEntry("BackgroundTheme", theme);

  c->sync();

  delete c;
}

void PanelTab::defaults()
{
  position = Bottom;
  size = Normal;
  showHBs = true;
  highlightHBs = true;
  HBwidth = 10;
  use_theme = false;
  theme = QString::null;

  pos_buttons[position]->setChecked(true);
  size_buttons[size]->setChecked(true);
  show_hbs->setChecked(showHBs);
  highlight_hbs->setChecked(highlightHBs);
  hb_size->setValue(HBwidth);
  use_theme_cb->setChecked(use_theme);
  theme_input->setText(theme);
  theme_label->clear();

  highlight_hbs->setEnabled(showHBs);
  hb_size->setEnabled(showHBs);

  theme_input->setEnabled(use_theme);
  theme_label->setEnabled(use_theme);
  theme_input->setEnabled(use_theme);
  browse_button->setEnabled(use_theme);
}

QString PanelTab::quickHelp()
{
  return i18n("<h1>Panel</h1> Here you can configure the KDE panel (also"
    " referred to as 'kicker'). This includes options like the position and"
    " size of the panel as well as its hiding behaviour and its looks.<p>"
    " Note that you can access some of these options also by directly clicking"
    " on the panel, e.g. dragging it with the left mouse button or using the"
    " context menu on right button click. This context menu also offers you"
    " manipulation of the panel's buttons and applets.");
}
