/* This file is part of the KDE Display Manager Configuration package
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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include <unistd.h>
#include <sys/types.h>


#include <qpushbutton.h>
#include <qapplication.h>
#include <qcombobox.h>
#include <qlayout.h>
#include <qgroupbox.h>
#include <qhgroupbox.h>
#include <qvgroupbox.h>
#include <qlabel.h>
#include <qwhatsthis.h>

#include <ksimpleconfig.h>
#include <klocale.h>
#include <kstandarddirs.h>

#include "kdm-font.h"


extern KSimpleConfig *config;

KDMFontWidget::KDMFontWidget(QWidget *parent, const char *name)
  : QWidget(parent, name)
{
  QGroupBox *tGroup = new QHGroupBox(i18n("Select fonts"), this);

  fontcombo = new QComboBox( FALSE, tGroup );
  fontcombo->insertItem(i18n("Greeting"), 0);
  fontcombo->insertItem(i18n("Fail"), 1);
  fontcombo->insertItem(i18n("Standard"), 2);
  fontcombo->setFixedSize(fontcombo->sizeHint());
  connect(fontcombo, SIGNAL(highlighted(int)), SLOT(slotSetFont(int)));

  QWhatsThis::add( fontcombo, i18n("Here you can select the font you want to change."
    " KDM knows three fonts: <ul><li><em>Greeting:</em> used to display KDM's greeting"
    " string (see \"Appearance\" tab)</li><li><em>Fail:</em> used to display a message"
    " when a person fails to login</li><li><em>Standard:</em> used for the rest of the text</li></ul>") );

  QPushButton *fontbtn = new QPushButton(i18n("C&hange Font..."), tGroup);
  fontbtn->setFixedSize(fontbtn->sizeHint());
  connect(fontbtn, SIGNAL(clicked()), SLOT(slotGetFont()));

  QWhatsThis::add( fontbtn, i18n("Click here to change the selected font.") );

  tGroup->addSpace(0);

  QGroupBox *bGroup = new QVGroupBox(i18n("Example"), this);
  QWhatsThis::add( bGroup, i18n("Shows a preview of the selected font.") );
  fontlabel = new QLabel( bGroup );
  fontlabel->setFrameStyle(QFrame::WinPanel|QFrame::Sunken);
  fontlabel->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));


  QGroupBox *mGroup = new QVGroupBox(i18n("Miscellaneous"), this);
  aacb = new QCheckBox (i18n("Use Anti-Aliasing for fonts"), mGroup);
  QWhatsThis::add( aacb, i18n("If you check this box and your X-Server has the Xft extension, "
	"fonts will be antialiased (smoothed) in the login dialog.") );
  connect(aacb, SIGNAL(toggled ( bool )),this,SLOT(configChanged()));

  QBoxLayout *ml = new QVBoxLayout(this, 10);
  ml->addWidget(tGroup);
  ml->addWidget(bGroup, 1);
  ml->addWidget(mGroup);
  ml->addStretch(1);

  load();
  slotSetFont(0);

  if (getuid() != 0)
    {
      fontbtn->setEnabled(false);
      fontcombo->setEnabled(false);
      aacb->setEnabled(false);
    }
}

void KDMFontWidget::configChanged()
{
    emit changed(true);
}


void KDMFontWidget::save()
{
  config->setGroup("X-*-Greeter");

  // write font
  config->writeEntry("StdFont", stdfont);
  config->writeEntry("GreetFont", greetfont);
  config->writeEntry("FailFont", failfont);
  config->writeEntry("AntiAliasing", aacb->isChecked());
}


void KDMFontWidget::set_def()
{
  stdfont = QFont("helvetica", 12);
  failfont = QFont("helvetica", 12, QFont::Bold);
  greetfont = QFont("charter", 24);
}


void KDMFontWidget::load()
{
  set_def();

  config->setGroup("X-*-Greeter");

  // Read the fonts
  stdfont = config->readFontEntry("StdFont", &stdfont);
  failfont = config->readFontEntry("FailFont", &failfont);
  greetfont = config->readFontEntry("GreetFont", &greetfont);

  slotSetFont(fontcombo->currentItem());

  aacb->setChecked(config->readBoolEntry("AntiAliasing"));
}


void KDMFontWidget::defaults()
{
  set_def();
  slotSetFont(fontcombo->currentItem());
  aacb->setChecked(false);
}


void KDMFontWidget::slotGetFont()
{
  QApplication::setOverrideCursor( waitCursor );

  KFontDialog *fontdlg = new KFontDialog(0, 0, TRUE, 0L);
  QApplication::restoreOverrideCursor( );
  QFont tmpfont;
  fontdlg->getFont(tmpfont);
  switch (fontcombo->currentItem())
  {
    case 0:
      greetfont = tmpfont;
      break;
    case 1:
      failfont = tmpfont;
      break;
    case 2:
      stdfont = tmpfont;
      break;
  }
  fontlabel->setFont(tmpfont);
  //fontlabel->setFixedSize(fontlabel->sizeHint());
  delete fontdlg;

  emit changed(true);
}


void KDMFontWidget::slotSetFont(int id)
{
  QApplication::setOverrideCursor( waitCursor );
  QFont *tmpfont;
  switch (id)
  {
    case 0:
      tmpfont = &greetfont;
      fontlabel->setText(i18n("Greeting font"));
      break;
    case 1:
      tmpfont = &failfont;
      fontlabel->setText(i18n("Fail font"));
      break;
    default:
      tmpfont = &stdfont;
      fontlabel->setText(i18n("Standard font"));
      break;
  }
  fontlabel->setFont(*tmpfont);
  //fontlabel->adjustSize();
  QApplication::restoreOverrideCursor( );
}

#include "kdm-font.moc"
