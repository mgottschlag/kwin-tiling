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

#include <kconfig.h>
#include <kdialog.h>
#include <kglobal.h>
#include <klocale.h>
#include <knuminput.h>
#include <klineedit.h>

#include "paneltab.h"
#include "paneltab.moc"

PanelTab::PanelTab( QWidget *parent, const char* name )
  : QWidget (parent, name)
  , position(Bottom)
  , size(Normal)
{
  int i;

  layout = new QGridLayout(this, 4, 2, 
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
  layout->addMultiCellWidget(pos_group, 0, 1, 0, 0);

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
  layout->addMultiCellWidget(size_group, 2, 3, 0, 0);

  // move mode group
  move_group = new QButtonGroup(i18n("&Move Mode"), this);

  vbox = new QVBoxLayout(move_group, KDialog::marginHint(),
                         KDialog::spacingHint());
  vbox->addSpacing(fontMetrics().lineSpacing());

  move_buttons[0] = new QRadioButton( i18n("&Position switching."), move_group );
  move_buttons[1] = new QRadioButton( i18n("Other applets are &fixed."), move_group );
  connect(move_group, SIGNAL(clicked(int)), SLOT(move_clicked(int)));

  for (i = 0; i < 2; i++)
	vbox->addWidget(move_buttons[i]);

  layout->addWidget(move_group, 0, 1);

  // auto-hide group
  ah_group = new QButtonGroup(i18n("&Auto-Hide Panel"), this);

  vbox = new QVBoxLayout(ah_group, KDialog::marginHint(),
                         KDialog::spacingHint());
  vbox->addSpacing(fontMetrics().lineSpacing());

  ah_cb = new QCheckBox(i18n("Enabled"), ah_group);
  connect(ah_cb, SIGNAL(clicked()), SLOT(ah_clicked()));
  vbox->addWidget(ah_cb);
  QWhatsThis::add( ah_cb, i18n("If this option is enabled the panel will automatically hide"
    " after some time and reappear when you move the mouse to the screen edge the panel is attached to."
    " This is particularly useful for small screen resolutions, for example, on laptops.") );

  ah_input = new KIntNumInput(3, ah_group);
  ah_input->setRange(1, 100, 1, true);
  ah_input->setLabel(i18n("&Delay in seconds:"), AlignLeft | AlignVCenter);
  connect(ah_input, SIGNAL(valueChanged(int)), SLOT(ah_input_changed(int)));
  vbox->addWidget(ah_input);
  QWhatsThis::add( ah_input, i18n("Here you can change the delay after which the panel will disappear"
    " if not used."));

  layout->addWidget(ah_group, 1, 1);

  // hidebutton group
  hb_group = new QGroupBox(i18n("&Hide Buttons"), this);

  vbox = new QVBoxLayout(hb_group,KDialog::marginHint(),
			 KDialog::spacingHint());
  vbox->addSpacing(fontMetrics().lineSpacing());

  show_hbs = new QCheckBox(i18n("&Enabled"), hb_group);
  connect(show_hbs, SIGNAL(clicked()), SLOT(show_hbs_clicked()));
  QWhatsThis::add( show_hbs, i18n("If this option is enabled, the panel"
    " will have buttons on both ends that can be used to hide it. The"
    " panel will slide away, leaving more room for applications. There"
    " only remains a small button which can be used to show the panel again.") );

  highlight_hbs = new QCheckBox(i18n("Highlight on mouse &over."), hb_group);
  connect(highlight_hbs, SIGNAL(clicked()), SIGNAL(changed()));
  QWhatsThis::add( highlight_hbs, i18n("If this option is enabled, the"
    " hide buttons are highlighted when the mouse cursor is moved over them.") );

  hb_input = new KIntNumInput(10, hb_group);
  hb_input->setRange(3, 24, 1, true);
  hb_input->setLabel(i18n("Size:"), AlignLeft | AlignVCenter);
  connect(hb_input, SIGNAL(valueChanged(int)), SLOT(hbs_input_changed(int)));
  QString wtstr = i18n("Here you can change the size of the hide buttons.");
  QWhatsThis::add( hb_input, wtstr );

  vbox->addWidget(show_hbs);
  vbox->addWidget(highlight_hbs);
  vbox->addWidget(hb_input);
  layout->addWidget(hb_group, 2, 1);

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

  layout->addWidget(misc_group, 3, 1);

  // TODO : Enable as soon as kicker supports move modes.
  move_group->setEnabled(false);

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

void PanelTab::move_clicked(int i)
{
  movemode = static_cast<MoveMode>(i);
  emit changed();
}

void PanelTab::show_hbs_clicked()
{
  bool showHBs = show_hbs->isChecked();
  
  highlight_hbs->setEnabled(showHBs);
  hb_input->setEnabled(showHBs);

  emit changed();
}

void PanelTab::hbs_input_changed(int)
{
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
  
  c->setGroup("panel");

  size = static_cast<Size>(c->readNumEntry("Size", Normal));
  size_buttons[size]->setChecked(true);

  position = static_cast<Position>(c->readNumEntry("Position", Bottom));
  pos_buttons[position]->setChecked(true);

  movemode = static_cast<MoveMode>(c->readNumEntry("MoveMode", Free));
  move_buttons[movemode]->setChecked(true);

  bool showHBs = c->readBoolEntry("ShowHideButtons", true);
  show_hbs->setChecked(showHBs);

  highlight_hbs->setChecked(c->readBoolEntry("HighlightHideButtons", true));
  hb_input->setValue(c->readNumEntry("HideButtonSize", 10));

  highlight_hbs->setEnabled(showHBs);
  hb_input->setEnabled(showHBs);

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
  
  c->setGroup("panel");

  c->writeEntry("Size", static_cast<int>(size));
  c->writeEntry("Position", static_cast<int>(position));
  c->writeEntry("MoveMode", static_cast<int>(movemode));
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
  movemode = Free;
   
  pos_buttons[position]->setChecked(true);
  size_buttons[size]->setChecked(true);
  move_buttons[movemode]->setChecked(true);
  show_hbs->setChecked(true);
  highlight_hbs->setChecked(true);
  highlight_hbs->setEnabled(true);
  hb_input->setValue(10);
  hb_input->setEnabled(true);

  ah_cb->setChecked(false);
  ah_input->setValue(3);
  ah_input->setEnabled(false);
}

