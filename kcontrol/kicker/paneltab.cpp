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
#include <qvbox.h>
#include <qlabel.h>
#include <qpainter.h>

#include <kconfig.h>
#include <kdialog.h>
#include <kglobal.h>
#include <klocale.h>
#include <knuminput.h>
#include <klineedit.h>
#include <kstddirs.h>
#include <kiconloader.h>

#include "paneltab.h"
#include "paneltab.moc"

const int hb_arrow = 8;

PanelTab::PanelTab( QWidget *parent, const char* name )
  : QWidget (parent, name)
  , position(Bottom)
  , size(Normal)
{
  int i;

  layout = new QGridLayout(this, 3, 2,
			   KDialog::marginHint(),
			   KDialog::spacingHint());

  // position button group
  pos_group = new QButtonGroup(i18n("&Location"), this);

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
  layout->addWidget(pos_group, 0, 0);

  // size button group
  size_group = new QButtonGroup(i18n("&Size"), this);

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

  // theme background group
  theme_group = new QGroupBox(i18n("Background Theme"), this);

  vbox = new QVBoxLayout(theme_group,KDialog::marginHint(),
                                       KDialog::spacingHint());
  vbox->addSpacing(fontMetrics().lineSpacing());

  QHBox *hbox1 = new QHBox(theme_group);
  use_theme_cb = new QCheckBox(i18n("Use background theme"), hbox1);
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
  misc_group = new QButtonGroup(i18n("Mis&cellaneous"), this);

  vbox = new QVBoxLayout(misc_group, KDialog::marginHint(),
                         KDialog::spacingHint());
  vbox->addSpacing(fontMetrics().lineSpacing());

  QHBox *hbox = new QHBox(misc_group);
  ta_label = new QLabel(i18n("Terminal application:"), hbox);
  ta_input = new KLineEdit(hbox);
  connect (ta_input, SIGNAL(textChanged(const QString&)), SLOT(ta_input_changed(const QString&)));
  hbox->setSpacing(KDialog::spacingHint());
  hbox->setStretchFactor(ta_input, 2);
  vbox->addWidget(hbox);

  layout->addMultiCellWidget(misc_group, 2, 2, 0, 1);

  layout->setRowStretch(0, 6);
  layout->setRowStretch(1, 5);
  layout->setRowStretch(2, 1);
  layout->setColStretch(0, 1);
  layout->setColStretch(1, 3);

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
  bool showHBs = show_hbs->isChecked();

  hb_preview->setEnabled(show_hbs->isChecked());

  highlight_hbs->setEnabled(showHBs);
  hb_input->setEnabled(showHBs);

  emit changed();
}

void PanelTab::hbs_input_changed(int)
{
  hb_preview->setWidth(hb_input->value());
  emit changed();
}

void PanelTab::ah_clicked()
{
  ah_input->setEnabled(ah_cb->isChecked());
  emit changed();
}

void PanelTab::ah_input_changed(int)
{
  emit changed();
}

void PanelTab::ta_input_changed(const QString&)
{
  emit changed();
}

void PanelTab::load()
{
  KConfig *c = new KConfig("kickerrc", false, false);

  c->setGroup("General");

  size = static_cast<Size>(c->readNumEntry("Size", Normal));
  size_buttons[size]->setChecked(true);

  position = static_cast<Position>(c->readNumEntry("Position", Bottom));
  pos_buttons[position]->setChecked(true);

  bool showHBs = c->readBoolEntry("ShowHideButtons", true);
  show_hbs->setChecked(showHBs);

  highlight_hbs->setChecked(c->readBoolEntry("HighlightHideButtons", true));
  hb_input->setValue(c->readNumEntry("HideButtonSize", 10));

  highlight_hbs->setEnabled(showHBs);
  hb_input->setEnabled(showHBs);

  hb_preview->setEnabled(show_hbs->isChecked());
  hb_preview->setWidth(hb_input->value());
  hb_preview->setHighlight(highlight_hbs->isChecked());

  bool ah = c->readBoolEntry("AutoHidePanel", false);
  ah_cb->setChecked(ah);

  ah_input->setValue(c->readNumEntry("AutoHideDelay", 3));
  ah_input->setEnabled(ah);

  c->setGroup("misc");

  ta_input->setText(c->readEntry("Terminal", "konsole"));

  delete c;
}

void PanelTab::save()
{
  KConfig *c = new KConfig("kickerrc", false, false);

  c->setGroup("General");

  c->writeEntry("Size", static_cast<int>(size));
  c->writeEntry("Position", static_cast<int>(position));
  c->writeEntry("ShowHideButtons", show_hbs->isChecked());
  c->writeEntry("HighlightHideButtons", highlight_hbs->isChecked());
  c->writeEntry("HideButtonSize", hb_input->value());
  c->writeEntry("AutoHidePanel", ah_cb->isChecked());
  c->writeEntry("AutoHideDelay", ah_input->value());

  c->setGroup("misc");

  c->writeEntry("Terminal", ta_input->text());

  c->sync();

  delete c;
}

void PanelTab::defaults()
{
  position = Bottom;
  size = Normal;

  pos_buttons[position]->setChecked(true);
  size_buttons[size]->setChecked(true);
  show_hbs->setChecked(true);
  highlight_hbs->setChecked(true);
  highlight_hbs->setEnabled(true);
  hb_input->setValue(10);
  hb_input->setEnabled(true);

  ah_cb->setChecked(false);
  ah_input->setValue(3);
  ah_input->setEnabled(false);
}

HBPreview::HBPreview(QWidget *parent, const char* name)
  : QWidget(parent, name)
  , _enabled(true)
  , _highlight(true)
  , _width(10)
{
  _icon = KGlobal::iconLoader()->loadIcon("go", KIcon::Desktop, KIcon::SizeMedium, KIcon::DefaultState, 0L, true);
}


void HBPreview::paintEvent(QPaintEvent*)
{
  QRect brect, prect;

  if (_enabled)
	{
	  brect = QRect(0, 0, _width, height());
	  prect = QRect(brect.right()+1, 0, width() - brect.width(), height());
	}
  else
	prect = rect();

  bool hl = false;

  QPainter p(this);
  p.setFont(font());

  // fill panel rect
  p.fillRect(prect, colorGroup().brush(QColorGroup::Background));

  // draw panel
  p.setPen(colorGroup().light());
  p.drawLine(prect.left(), 0, prect.right()-1, 0);
  p.drawLine(prect.left(), 0, prect.left(), prect.bottom()-1);
  p.setPen(Qt::black);
  p.drawLine(prect.left(), prect.bottom()-1, prect.right()-1, prect.bottom()-1);
  p.drawLine(prect.right()-1, 0, prect.right()-1, prect.bottom()-1);

  // draw icon
  if(!_icon.isNull())
	p.drawPixmap(brect.width()+7, 7, _icon);


  if (!_enabled) return;

  // fill button rect
  if(!hl)
	p.fillRect(brect, colorGroup().brush(QColorGroup::Background));
  else
	p.fillRect(brect, colorGroup().brush(QColorGroup::Light));

  // draw button
  p.setPen(colorGroup().light());
  p.drawLine(0, 0, brect.right()-1, 0);
  p.drawLine(0, 0, 0, brect.bottom()-1);
  p.setPen(Qt::black);
  p.drawLine(0, brect.bottom()-1, brect.right()-1, brect.bottom()-1);
  p.drawLine(brect.right()-1, 0, brect.right()-1, brect.bottom()-1);

  if(_width < 10) return; // don't draw arrows if we are to small

  QApplication::style().drawArrow(&p, Qt::LeftArrow, false,
								  (_width-hb_arrow)/2, (height()-hb_arrow)/2,
								  hb_arrow, hb_arrow, colorGroup(), true);
}
