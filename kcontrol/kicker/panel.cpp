/*
 *  panel.cpp
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
#include <qbuttongroup.h>
#include <qradiobutton.h>
#include <qcheckbox.h>
#include <qslider.h>
#include <qlabel.h>
#include <qhbox.h>
#include <qstring.h>
#include <qpushbutton.h>
#include <qpixmap.h>
#include <qimage.h>

#include <kconfig.h>
#include <kglobal.h>
#include <klocale.h>
#include <klineedit.h>
#include <kfiledialog.h>
#include <kmessagebox.h>
#include <kimgio.h>

#include "panel.h"

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

  layout = new QGridLayout(this, 2, 2, 10);
  
  // position button group
  pos_group = new QButtonGroup(i18n("Location"), this);
  
  QVBoxLayout *vbox = new QVBoxLayout(pos_group,10);
  vbox->addSpacing(pos_group->fontMetrics().height());
  
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
  
  vbox = new QVBoxLayout(size_group,10);
  vbox->addSpacing(size_group->fontMetrics().height());
  
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

  vbox = new QVBoxLayout(hb_group,10);
  vbox->addSpacing(hb_group->fontMetrics().height());

  show_hbs = new QCheckBox(i18n("Show hide buttons."), hb_group);
  connect(show_hbs, SIGNAL(clicked()), SLOT(show_hbs_clicked()));
  show_hbs->setChecked(showHBs);

  highlight_hbs = new QCheckBox(i18n("Highlight hide buttons on mouse over."), hb_group);
  connect(highlight_hbs, SIGNAL(clicked()), SLOT(highlight_hbs_clicked()));
  highlight_hbs->setChecked(highlightHBs);

  QHBox *hbox = new QHBox(hb_group);
  hb_size_label = new QLabel(i18n("Size:"), hbox);
  hb_size = new QSlider( 3, 24, 1, HBwidth, Horizontal, hbox); 
  connect(hb_size, SIGNAL(valueChanged(int)), SLOT(hbs_size_changed(int)));
  hb_size->setTickmarks(QSlider::Below);
  hb_size->setTracking(true);
  hbox->setSpacing(10); 
  hbox->setStretchFactor(hb_size, 2); 

  vbox->addWidget(show_hbs);
  vbox->addWidget(highlight_hbs);
  vbox->addWidget(hbox);
  layout->addWidget(hb_group, 0, 1);

  // theme background group
  theme_group = new QGroupBox(i18n("Background Theme"), this);

  vbox = new QVBoxLayout(theme_group,10);
  vbox->addSpacing(theme_group->fontMetrics().height());

  QHBox *hbox1 = new QHBox(theme_group);
  use_theme_cb = new QCheckBox(i18n("Use background theme."), hbox1);
  connect(use_theme_cb, SIGNAL(clicked()), SLOT(use_theme_clicked()));
  theme_label = new QLabel(hbox1);
  theme_label->setFixedSize(50,50);
  theme_label->setFrameStyle(QFrame::WinPanel | QFrame::Sunken);
  theme_label->setAlignment(AlignCenter);
  hbox1->setSpacing(10); 
  hbox1->setStretchFactor(use_theme_cb, 2); 


  QHBox *hbox2 = new QHBox(theme_group);
  theme_input = new KLineEdit(hbox2);
  theme_input->setReadOnly(true);
  browse_button = new QPushButton(i18n("Browse"), hbox2);
  connect(browse_button, SIGNAL(clicked()), SLOT(browse_theme()));

  hbox2->setSpacing(10); 
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

#include "panel.moc"
