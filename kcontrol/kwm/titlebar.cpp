/*
 * titlebar.cpp
 *
 * Copyright (c) 1997 Patrick Dowler dowler@morgul.fsh.uvic.ca
 *
 * Requires the Qt widget libraries, available at no cost at
 * http://www.troll.no/
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
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <iostream.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>

#include <qdir.h>
#include <qlayout.h>
#include <qmessagebox.h>

#include <kapp.h>
#include <kiconloaderdialog.h>
#include <kiconloader.h>

#include "titlebar.h"

#include "geom.h"
#include <klocale.h>
#include <kconfig.h>
#include <kstddirs.h>
#include <kglobal.h>

#include <kpixmapeffect.h> //CT for the previews
extern KConfig *config;

// config file keywords used by kwm
#define KWM_TITLEBARLOOK   "TitlebarLook"
#define KWM_TITLEANIMATION "TitleAnimation"
#define KWM_TITLEALIGN     "TitleAlignment"

//CT 02Dec1998 - weird hacks
#define KWM_TITLEFRAME     "TitleFrameShaded"
#define KWM_PIXMAP_TEXT    "PixmapUnderTitleText"
//CT

//  buttons 1 2 3 are on left, 4 5 6 on right
#define KWM_B1 "ButtonA"
#define KWM_B2 "ButtonB"
#define KWM_B3 "ButtonC"
#define KWM_B4 "ButtonF"
#define KWM_B5 "ButtonE"
#define KWM_B6 "ButtonD"

//CT 11feb98
#define KWM_DCTBACTION "TitlebarDoubleClickCommand"

KTitlebarButtons::~KTitlebarButtons ()
{
  delete minB;
  delete maxB;
  delete stickyB;
  delete closeB;
  delete menuB;

  delete minP;
  delete maxP;
  delete stickyP;
  delete closeP;
  delete menuP;

  delete left;
  delete right;
  delete off;

  for (int i=0; i<3; i++)
    {
      delete minRB[i];
      delete maxRB[i];
      delete stickyRB[i];
      delete closeRB[i];
      delete menuRB[i];
    }

  delete minBox;
  delete maxBox;
  delete stickyBox;
  delete closeBox;
  delete menuBox;

  delete blankTitlebar;
  delete titlebarFrame;

}

KTitlebarButtons::KTitlebarButtons (QWidget * parent, const char *name)
  : KConfigWidget (parent, name)
{
  int i;

  //CT 08Apr1999 - layout - finally
  QGridLayout *lay = new QGridLayout( this, 11, 15, 10);
  lay->addRowSpacing( 0, 20);
  lay->addRowSpacing( 2, 20);
  lay->addRowSpacing( 4, 10);
  lay->addRowSpacing( 6, 10);
  lay->addRowSpacing( 6, 10);
  lay->addRowSpacing( 8, 10);
  lay->addRowSpacing(10, 10);
  lay->addRowSpacing(12, 10);
  lay->addRowSpacing(14, 20);

  lay->addColSpacing( 0, 20);
  lay->addColSpacing( 2, 20);
  lay->addColSpacing( 4, 10);
  lay->addColSpacing( 6, 10);
  lay->addColSpacing( 6, 10);
  lay->addColSpacing( 8, 10);
  lay->addColSpacing( 9, 10);
  lay->addColSpacing(10, 20);
  

  lay->setRowStretch( 0,  0);
  lay->setRowStretch( 1,  0);
  lay->setRowStretch( 2,  1);
  lay->setRowStretch( 3,  0);
  lay->setRowStretch( 4,  1);
  lay->setRowStretch( 5,  0);
  lay->setRowStretch( 6,  1);
  lay->setRowStretch( 7,  0);
  lay->setRowStretch( 8,  1);
  lay->setRowStretch( 9,  0);
  lay->setRowStretch(10,  1);
  lay->setRowStretch(11,  0);
  lay->setRowStretch(12,  1);
  lay->setRowStretch(13,  0);
  lay->setRowStretch(14,  0);

  lay->setColStretch( 0,  0);
  lay->setColStretch( 1,  0);
  lay->setColStretch( 2,  1);
  lay->setColStretch( 3,  0);
  lay->setColStretch( 4,  1);
  lay->setColStretch( 5,  0);
  lay->setColStretch( 6,  1);
  lay->setColStretch( 7,  0);
  lay->setColStretch( 8,  1);
  lay->setColStretch( 9,  0);
  lay->setColStretch(10,  0);
  //CT

  titlebarFrame = new QFrame(this, "testframe");
  titlebarFrame ->setFrameStyle(QFrame::WinPanel | QFrame::Raised );
  blankTitlebar = new TitlebarPreview(titlebarFrame, "blanktbar");

  lay->addMultiCellWidget (titlebarFrame, 1, 1, 1, 7, 10);

  // button name labels
  minB = new QLabel(i18n("Minimize"), this);

  lay->addWidget( minB, 5, 1, 10);

  maxB = new QLabel(i18n("Maximize"), this);

  lay->addWidget( maxB, 7, 1, 10);

  stickyB = new QLabel(i18n("Sticky"), this);

  lay->addWidget( stickyB, 9, 1, 10);

  closeB = new QLabel(i18n("Close"), this);

  lay->addWidget( closeB, 11, 1, 10);

  menuB = new QLabel(i18n("Menu"), this);

  lay->addWidget( menuB, 13, 1, 10);

  KGlobal::dirs()->addResourceType("kwm_pics", KStandardDirs::kde_default("data") + "/kwm/pics/");

  // pixmap labels to show which button is which
  minP = new QLabel("", this);
  minP->setPixmap( Icon(locate("kwm_pics", "iconify.xpm")));

  lay->addWidget( minP, 5, 3, 10);

  maxP = new QLabel("", this);
  maxP->setPixmap( Icon(locate("kwm_pics", "maximize.xpm")));

  lay->addWidget( maxP, 7, 3, 10);

  stickyP = new QLabel("", this);
  stickyP->setPixmap( Icon(locate("kwm_pics", "pinup.xpm")));

  lay->addWidget( stickyP, 9, 3, 10);

  closeP = new QLabel("", this);
  closeP->setPixmap( Icon(locate("kwm_pics", "close.xpm")));

  lay->addWidget( closeP, 11, 3, 10);

  menuP = new QLabel("", this);
  menuP->setPixmap( Icon(locate("kwm_pics", "menu.xpm")));

  lay->addWidget( menuP, 13, 3, 10);

  // left/right/off column labels
  left = new QLabel(i18n("Left"), this);

  lay->addWidget( left, 3, 5, 10);

  right = new QLabel(i18n("Right"), this);

  lay->addWidget( right, 3, 7, 10);

  off = new QLabel(i18n("Off"), this);

  lay->addWidget( off, 3, 9, 10);

  // left/right/off radio buttons and groups
  QBoxLayout *hlay;

  minBox = new QButtonGroup("", this, NULL);
  minBox->setFrameStyle( QFrame::NoFrame );

  lay->addMultiCellWidget(minBox, 5, 5, 5, 9, 10);

  hlay = new QHBoxLayout( minBox, 10);
  for (i=0; i<3; i++)
    {
      minRB[i] = new QRadioButton(" ", minBox, NULL);
      minRB[i]->adjustSize();
      hlay->addWidget(minRB[i], 10);
      connect(minRB[i], SIGNAL(clicked()), this, SLOT(updatePreview()));
    }
  hlay->addStretch(0);

  maxBox = new QButtonGroup(" ", this, NULL);
  maxBox->setFrameStyle( QFrame::NoFrame );

  lay->addMultiCellWidget(maxBox, 7, 7, 5, 9, 10);

  hlay = new QHBoxLayout( maxBox, 10);
  for (i=0; i<3; i++)
    {
      maxRB[i] = new QRadioButton(" ", maxBox, NULL);
      hlay->addWidget(maxRB[i], 10);
      connect(maxRB[i], SIGNAL(clicked()), this, SLOT(updatePreview()));
    }

  stickyBox = new QButtonGroup("", this, NULL);
  stickyBox->setFrameStyle( QFrame::NoFrame );

  lay->addMultiCellWidget(stickyBox, 9, 9, 5, 9, 10);

  hlay = new QHBoxLayout( stickyBox, 10);
  for (i=0; i<3; i++)
    {
      stickyRB[i] = new QRadioButton(" ", stickyBox, NULL);
      hlay->addWidget(stickyRB[i], 10);
      connect(stickyRB[i], SIGNAL(clicked()), this, SLOT(updatePreview()));
    }

  closeBox = new QButtonGroup("", this, NULL);
  closeBox->setFrameStyle( QFrame::NoFrame );

  lay->addMultiCellWidget(closeBox, 11, 11, 5, 9, 10);

  hlay = new QHBoxLayout( closeBox, 10);
  for (i=0; i<3; i++)
    {
      closeRB[i] = new QRadioButton(" ", closeBox, NULL);
      hlay->addWidget(closeRB[i], 10);
      connect(closeRB[i], SIGNAL(clicked()), this, SLOT(updatePreview()));
    }

  menuBox = new QButtonGroup("", this, NULL);
  menuBox->setFrameStyle( QFrame::NoFrame );

  lay->addMultiCellWidget(menuBox, 13, 13, 5, 9, 10);

  hlay = new QHBoxLayout( menuBox, 10);
  for (i=0; i<3; i++)
    {
      menuRB[i] = new QRadioButton(" ", menuBox, NULL);
      hlay->addWidget(menuRB[i], 10);
      connect(menuRB[i], SIGNAL(clicked()), this, SLOT(updatePreview()));
    }

  GetSettings();
}

void KTitlebarButtons::resizeEvent(QResizeEvent *)
{
  int h = SPACE_YO;
  int column_h = h + 2*SPACE_YO;   // vertical position of column headings

  h = column_h + left->height() + SPACE_YO;

  // use the previews own setGeometry to do what we mean
  titlebarFrame->setGeometry( SPACE_XO, SPACE_YO,
			      width() - 2*SPACE_XO, minP->height() + 8 );
  blankTitlebar->setGeometry( 4, 4,
			      width() - 2*SPACE_XO - 8, minP->height() );
  blankTitlebar->setPixmapSize( minP->width(), minP->height() );


  drawPreview(TRUE);
}

void KTitlebarButtons::updatePreview()
{
  drawPreview(TRUE);
}

void KTitlebarButtons::drawPreview(bool draw)
{
  int left = 0, right = 0;
  QPixmap *p = closeP->pixmap();

  for (int i=0; i<NUM_BUTTONS; i++)
    selectedFunc[i] = NOFUNC;

  blankTitlebar->removeAll();

  // place the highest priority pixmaps first

  // menu can only go at the edge: A or F
  if (menuRB[0]->isChecked())
    {
      if (draw)
	{
	  p = menuP->pixmap();
	  blankTitlebar->setA( p );
	}
      selectedFunc[0] = MENU;
      left++;
    }
  else if (menuRB[1]->isChecked())
    {
      if (draw)
	{
	  p = menuP->pixmap();
	  blankTitlebar->setF( p );
	}
      selectedFunc[5] = MENU;
      right++;
    }
  else
    {
      menuRB[2]->setChecked(TRUE);
    }

  // close can go in A, B, E, or F
  if (closeRB[0]->isChecked())
    {
      if (draw) p = closeP->pixmap();
      if (left == 0)
	{
	  if (draw) blankTitlebar->setA( p );
	  selectedFunc[0] = CLOSE;
	}
      else
	{
	  if (draw) blankTitlebar->setB( p );
	  selectedFunc[1] = CLOSE;
	}
      left++;
    }
  else if (closeRB[1]->isChecked())
    {
      if (draw) p = closeP->pixmap();
      if (right == 0)
	{
	  if (draw) blankTitlebar->setF( p );
	  selectedFunc[5] = CLOSE;
	}
      else
	{
	  if (draw) blankTitlebar->setE( p );
	  selectedFunc[4] = CLOSE;
	}
      right++;
    }
  else
    {
      // make sure it is OFF
      closeRB[2]->setChecked(TRUE);
    }


  // sticky can go anywhere but always fits
  if (stickyRB[0]->isChecked())
    {
      if (draw) p = stickyP->pixmap();
      if (left == 0)
	{
	  if (draw) blankTitlebar->setA( p );
	  selectedFunc[0] = STICKY;
	}
      else if (left == 1)
	{
	  if (draw) blankTitlebar->setB( p );
	  selectedFunc[1] = STICKY;
	}
      else
	{
	  if (draw) blankTitlebar->setC( p );
	  selectedFunc[2] = STICKY;
	}
      left++;
    }
  else if (stickyRB[1]->isChecked())
    {
      if (draw) p = stickyP->pixmap();
      if (right == 0)
	{
	  if (draw) blankTitlebar->setF( p );
	  selectedFunc[5] = STICKY;
	}
      else if (right == 1)
	{
	  if (draw) blankTitlebar->setE( p );
	  selectedFunc[4] = STICKY;
	}
      else
	{
	  if (draw) blankTitlebar->setD( p );
	  selectedFunc[3] = STICKY;
	}
      right++;
    }
  else
    {
      // make sure this func is OFF
      stickyRB[2]->setChecked(TRUE);
    }

  // max may not fit is the selected side is full already
  if (maxRB[0]->isChecked())
    {
      if (draw) p = maxP->pixmap();
      if (left == 0)
	{
	  if (draw) blankTitlebar->setA( p );
	  selectedFunc[0] = MAXIMIZE;
	}
      else if (left == 1)
	{
	  if (draw) blankTitlebar->setB( p );
	  selectedFunc[1] = MAXIMIZE;
	}
      else if (left == 2)
	{
	  if (draw) blankTitlebar->setC( p );
	  selectedFunc[2] = MAXIMIZE;
	}
      else
	{

	  // can't place max on left
	  QMessageBox::warning(this,i18n("Window Manager Setup - Warning"),
			       i18n("The left side of the titlebar "
				    "is full... disabling the 'maximize' "
				    "button\n"),
			       i18n("&Ok"));
	  maxRB[0]->setChecked(FALSE);
	  maxRB[2]->setChecked(TRUE);
	  left--;
	}
      left++;
    }
  else if (maxRB[1]->isChecked())
    {
      if (draw) p = maxP->pixmap();
      if (right == 0)
	{
	 if (draw)  blankTitlebar->setF( p );
	  selectedFunc[5] = MAXIMIZE;
	}
      else if (right == 1)
	{
	  if (draw) blankTitlebar->setE( p );
	  selectedFunc[4] = MAXIMIZE;
	}
      else if (right == 2)
	{
	  if (draw) blankTitlebar->setD( p );
	  selectedFunc[3] = MAXIMIZE;
	}
      else
	{
	  // can't place max on right
	  QMessageBox::warning(this,i18n("Window Manager Setup - Warning"),
			       i18n("The right side of the titlebar "
				    "is full... disabling the 'maximise' "
				    "button\n"),
			       i18n("&Ok"));
	  maxRB[1]->setChecked(FALSE);
	  maxRB[2]->setChecked(TRUE);
	  right--;
	}
      right++;
    }
  else
    {
      // make sure this func is OFF
      maxRB[2]->setChecked(TRUE);
    }

  // min may not fit is the selected side is full already
  if (minRB[0]->isChecked())
    {
      if (draw) p = minP->pixmap();
      if (left == 0)
	{
	  if (draw) blankTitlebar->setA( p );
	  selectedFunc[0] = ICONIFY;
	}
      else if (left == 1)
	{
	  if (draw) blankTitlebar->setB( p );
	  selectedFunc[1] = ICONIFY;
	}
      else if (left == 2)
	{
	  if (draw) blankTitlebar->setC( p );
	  selectedFunc[2] = ICONIFY;
	}
      else
	{
	  // left side is full
	  QMessageBox::warning(this,i18n("Window Manager Setup - Warning"),
			       i18n("The left side of the titlebar "
				    "is full... disabling the 'minimize' "
				    "button\n"),
			       i18n("&Ok"));
	  minRB[0]->setChecked(FALSE);
	  minRB[2]->setChecked(TRUE);
	  left--;
	}
      left++;
    }
  else if (minRB[1]->isChecked())
    {
      if (draw) p = minP->pixmap();
      if (right == 0)
	{
	  if (draw) blankTitlebar->setF( p );
	  selectedFunc[5] = ICONIFY;
	}
      else if (right == 1)
	{
	  if (draw) blankTitlebar->setE( p );
	  selectedFunc[4] = ICONIFY;
	}
      else if (right == 2)
	{
	  if (draw) blankTitlebar->setD( p );
	  selectedFunc[3] = ICONIFY;
	}
      else
	{
	  // can't place min on right
	  QMessageBox::warning(this,i18n("Window Manager Setup - Warning"),
			   i18n("The right side of the titlebar "
				"is full... disabling the 'minimize' "
				"button\n"),
			   i18n("&Ok"));
	  minRB[1]->setChecked(FALSE);
	  minRB[2]->setChecked(TRUE);
	  right--;
	}
      right++;
    }
  else
    {
      // make sure it is OFF
      minRB[2]->setChecked(TRUE);
    }
}

int KTitlebarButtons::getFunc(int button)
{
  return selectedFunc[button];
}

void KTitlebarButtons::setButton(int button, int func)
{
  // if button < 3, the func button goes on the left side
  // otherwise, the func button goes on the right side

  switch (func)
    {
    case ICONIFY:
      if (button < 3)
      {
	minRB[0]->setChecked(TRUE);
        minRB[1]->setChecked(FALSE);
      }
      else
      {
	minRB[1]->setChecked(TRUE);
        minRB[0]->setChecked(FALSE);
      }
      break;
    case MAXIMIZE:
      if (button < 3)
      {
	maxRB[0]->setChecked(TRUE);
        maxRB[1]->setChecked(FALSE);
      }
      else
      {
	maxRB[1]->setChecked(TRUE);
        maxRB[0]->setChecked(FALSE);
      }
      break;
    case STICKY:
      if (button < 3)
      {
	stickyRB[0]->setChecked(TRUE);
        stickyRB[1]->setChecked(FALSE);
      }
      else
      {
	stickyRB[1]->setChecked(TRUE);
        stickyRB[0]->setChecked(FALSE);
      }
      break;
    case CLOSE:
      if (button < 3)
      {
	closeRB[0]->setChecked(TRUE);
        closeRB[1]->setChecked(FALSE);
      }
      else
      {
	closeRB[1]->setChecked(TRUE);
        closeRB[0]->setChecked(FALSE);
      }
      break;
    case MENU:
      if (button < 3)
      {
	menuRB[0]->setChecked(TRUE);
        menuRB[1]->setChecked(FALSE);
      }
      else
      {
	menuRB[1]->setChecked(TRUE);
        menuRB[0]->setChecked(FALSE);
      }
      break;
    }
}

void KTitlebarButtons::setState()
{
  drawPreview(FALSE);
}

void KTitlebarButtons::getStringValue(int b, QString *str)
{
  switch (b)
    {
    case MENU:
      *str = "Menu";
      break;
    case STICKY:
      *str = "Sticky";
      break;
    case CLOSE:
      *str = "Close";
      break;
    case MAXIMIZE:
      *str = "Maximize";
      break;
    case ICONIFY:
      *str = "Iconify";
      break;
    case NOFUNC:
      *str = "Off";
      break;
    }
}

void KTitlebarButtons::SaveSettings( void )
{
  config->setGroup( "Buttons");

  QString str;
  int b;

  b = getFunc(0);
  getStringValue(b, &str);
  config->writeEntry(KWM_B1, str);

  b = getFunc(1);
  getStringValue(b, &str);
  config->writeEntry(KWM_B2, str);

  b = getFunc(2);
  getStringValue(b, &str);
  config->writeEntry(KWM_B3, str);

  b = getFunc(3);
  getStringValue(b, &str);
  config->writeEntry(KWM_B4, str);

  b = getFunc(4);
  getStringValue(b, &str);
  config->writeEntry(KWM_B5, str);

  b = getFunc(5);
  getStringValue(b, &str);
  config->writeEntry(KWM_B6, str);

  config->sync();

}

int KTitlebarButtons::buttonFunc(QString *key)
{
  int ret = NOFUNC;

  if( *key == "Off" )
    ret = NOFUNC;
  else if( *key == "Maximize" )
    ret = MAXIMIZE;
  else if( *key == "Iconify" )
    ret = ICONIFY;
  else if( *key == "Close" )
    ret = CLOSE;
  else if( *key == "Sticky" )
    ret = STICKY;
  else if (*key == "Menu" )
    ret = MENU;

  return ret;
}

void KTitlebarButtons::GetSettings( void )
{
  QString key;

  config->setGroup( "Buttons");
  int ABUTTON=0, BBUTTON=0, CBUTTON=0, DBUTTON=0, EBUTTON=0, FBUTTON=0;

  key = config->readEntry(KWM_B1);
  ABUTTON = buttonFunc(&key);

  key = config->readEntry(KWM_B2);
  BBUTTON = buttonFunc(&key);

  key = config->readEntry(KWM_B3);
  CBUTTON = buttonFunc(&key);

  key = config->readEntry(KWM_B4);
  DBUTTON = buttonFunc(&key);

  key = config->readEntry(KWM_B5);
  EBUTTON = buttonFunc(&key);

  key = config->readEntry(KWM_B6);
  FBUTTON = buttonFunc(&key);

  // clear all buttons (for reloading!)
  minRB[0]->setChecked(FALSE);
  minRB[1]->setChecked(FALSE);
  maxRB[0]->setChecked(FALSE);
  maxRB[1]->setChecked(FALSE);
  stickyRB[0]->setChecked(FALSE);
  stickyRB[1]->setChecked(FALSE);
  closeRB[0]->setChecked(FALSE);
  closeRB[1]->setChecked(FALSE);
  menuRB[0]->setChecked(FALSE);
  menuRB[1]->setChecked(FALSE);

  setButton(0, ABUTTON);
  setButton(1, BBUTTON);
  setButton(2, CBUTTON);
  setButton(3, DBUTTON);
  setButton(4, EBUTTON);
  setButton(5, FBUTTON);
  setState();
}

void KTitlebarButtons::loadSettings()
{
  GetSettings();
  drawPreview(TRUE);
}

void KTitlebarButtons::applySettings()
{
  SaveSettings();
}


// titlebar preview code
TitlebarPreview::~TitlebarPreview( )
{
  delete a;
  delete b;
  delete c;
  delete d;
  delete e;
  delete f;
}

TitlebarPreview::TitlebarPreview( QWidget *parent, const char *name )
        : QFrame( parent, name )
{
  a = new QLabel("", this, "a", 0);
  a->hide();
  b = new QLabel("", this, "b", 0);
  b->hide();
  c = new QLabel("", this, "c", 0);
  c->hide();
  d = new QLabel("", this, "d", 0);
  d->hide();
  e = new QLabel("", this, "e", 0);
  e->hide();
  f = new QLabel("", this, "f", 0);
  f->hide();
  setBackgroundColor( QColor( 0, 10, 160 ) );
}

void TitlebarPreview::setPixmapSize(int w, int /*unused*/ )
{
  xa = 0;
  xb = w;
  xc = 2*w;
  xd = width() - 3*w;
  xe = width() - 2*w;
  xf = width() - w;
}

void TitlebarPreview::setA( QPixmap *pm )
{
  a->setPixmap( *pm );
  a->adjustSize();
  a->show();
}
void TitlebarPreview::setB( QPixmap *pm )
{
  b->setPixmap( *pm );
  b->adjustSize();
  b->show();
}
void TitlebarPreview::setC( QPixmap *pm )
{
  c->setPixmap( *pm );
  c->adjustSize();
  c->show();
}
void TitlebarPreview::setD( QPixmap *pm )
{
  d->setPixmap( *pm );
  d->adjustSize();
  d->show();
}
void TitlebarPreview::setE( QPixmap *pm )
{
  e->setPixmap( *pm );
  e->adjustSize();
  e->show();
}
void TitlebarPreview::setF( QPixmap *pm )
{
  f->setPixmap( *pm );
  f->adjustSize();
  f->show();
}
void TitlebarPreview::removeAll( void )
{
  a->hide();
  b->hide();
  c->hide();
  d->hide();
  e->hide();
  f->hide();
}

void TitlebarPreview::paintEvent( QPaintEvent * )
{
  a->move(xa, 0);
  b->move(xb, 0);
  c->move(xc, 0);
  d->move(xd, 0);
  e->move(xe, 0);
  f->move(xf, 0);
}

// appearance dialog
//CT 21Oct1998 - voided
KTitlebarAppearance::~KTitlebarAppearance ()
{
}

//CT 21Oct1998 - rewritten
KTitlebarAppearance::KTitlebarAppearance (QWidget * parent, const char *name)
  : KConfigWidget (parent, name)
{
  // titlebar shading style

  QGridLayout *lay = new QGridLayout(this,4,2,5);
  lay->setRowStretch(0,0);
  lay->setRowStretch(1,1);
  lay->setRowStretch(2,0);
  lay->setRowStretch(3,0);

  lay->setColStretch(0,1);
  lay->setColStretch(1,1);

  //CT 06Nov1998 - title alignment GUI config  
  alignBox = new QButtonGroup (i18n("Title Alignment"), this);

  QGridLayout *pixLay = new QGridLayout(alignBox,2,3,15,5);
  
  leftAlign = new QRadioButton(i18n("Left"),alignBox);
  pixLay->addWidget(leftAlign,1,0);

  midAlign = new QRadioButton(i18n("Middle"),alignBox);
  pixLay->addWidget(midAlign,1,1);

  rightAlign = new QRadioButton(i18n("Right"),alignBox);
  pixLay->addWidget(rightAlign,1,2);

  pixLay->activate();

  lay->addMultiCellWidget(alignBox,0,0,0,1);
  //CT

  //CT 02Dec1998 - foul changes for some weird options
  appearBox = new QGroupBox(i18n("Appearance"), 
				 this);

  QBoxLayout *appearLay = new QVBoxLayout (appearBox,10,5);
  appearLay->addSpacing(10);

  titlebarBox = new QButtonGroup(appearBox);
  titlebarBox->setFrameStyle(QFrame::NoFrame);

  QBoxLayout *pushLay = new QVBoxLayout (titlebarBox,10,5);
  //CT

  bShaded = new QRadioButton(i18n("Gradient"), titlebarBox);
  pushLay->addWidget(bShaded);

  connect(bShaded, SIGNAL(clicked()), this, SLOT(titlebarChanged()));

  plain = new QRadioButton(i18n("Plain"), 
			   titlebarBox);
  pushLay->addWidget(plain);

  connect(plain, SIGNAL(clicked()), this, SLOT(titlebarChanged()));

  pixmap = new QRadioButton(i18n("Pixmap"), titlebarBox);
  pushLay->addWidget(pixmap);

  connect(pixmap, SIGNAL(clicked()), this, SLOT(titlebarChanged()));


  appearLay->addWidget(titlebarBox);

  cbFrame = new QCheckBox(i18n("Active title has shaded frame"),
	                   appearBox);
  cbFrame->adjustSize();
  cbFrame->setMinimumSize(cbFrame->size());
  appearLay->addWidget(cbFrame);


  lay->addWidget(appearBox,1,0);

  optOpts =  new QWidgetStack( this, "optOpts");

  lay->addWidget(optOpts, 1, 1);

  // the first page - options for pixmap titlebars  
  pixmapBox    = new QGroupBox(i18n("Pixmap"), optOpts); 
 
  pixLay = new QGridLayout(pixmapBox,7,2,10,5);
  pixLay->addRowSpacing(0,10);
  pixLay->addRowSpacing(3,10);
  pixLay->addColSpacing(0,20);
  pixLay->setRowStretch(0,1);
  pixLay->setRowStretch(1,0);
  pixLay->setRowStretch(2,0);
  pixLay->setRowStretch(3,1);
  pixLay->setRowStretch(4,0);
  pixLay->setRowStretch(5,0);
  pixLay->setRowStretch(6,1);
  pixLay->setColStretch(0,0);

  pbPixmapActive = new QPushButton(pixmapBox);
  pbPixmapActive->resize(96,32);
  pixLay->addWidget(pbPixmapActive,2,1);

  connect(pbPixmapActive, SIGNAL(clicked()), this, SLOT(activePressed()));
  
  pbPixmapInactive = new QPushButton(pixmapBox);
  pbPixmapInactive->resize(96,32);
  pixLay->addWidget(pbPixmapInactive,5,1);

  connect(pbPixmapInactive, SIGNAL(clicked()), this, SLOT(inactivePressed()));

  lPixmapActive = new QLabel(pbPixmapActive, i18n("Active pixmap:"), pixmapBox);
  pixLay->addMultiCellWidget(lPixmapActive,1,1,0,1);

  lPixmapInactive = new QLabel(pbPixmapInactive, i18n("Inactive pixmap:"), pixmapBox);
  pixLay->addMultiCellWidget(lPixmapInactive,4,4,0,1);

  cbPixedText = new QCheckBox(i18n("No pixmap under text"),pixmapBox);
  pixLay->addMultiCellWidget(cbPixedText,6,6,0,1);

  // second page - options for gradients

  gradBox = new QGroupBox(i18n("Gradient"), optOpts);
  
  QBoxLayout *gradLay = new QVBoxLayout(optOpts, 10);
  gradLay->addSpacing(10);

  gradientTypes = new QListBox(gradBox);

  gradientTypes->insertItem(i18n("Vertical"));
  gradientTypes->insertItem(i18n("Horizontal"));
  gradientTypes->insertItem(i18n("Diagonal"));
  gradientTypes->insertItem(i18n("CrossDiagonal"));
  gradientTypes->insertItem(i18n("Pyramid"));
  gradientTypes->insertItem(i18n("Rectangle"));
  gradientTypes->insertItem(i18n("PipeCross"));
  gradientTypes->insertItem(i18n("Elliptic"));
  
  gradientTypes->setMultiSelection(false);

  connect(gradientTypes, SIGNAL(highlighted(const QString &)), 
	  this, SLOT(setGradient(const QString &)));

  gradLay->addWidget(gradientTypes);

  gradLay->addSpacing(10);

  gradPreview = new QFrame ( gradBox);
  gradPreview->setFrameStyle(QFrame::Panel | QFrame::Raised);
  gradPreview->setFixedHeight(24);
  gradPreview->setFixedWidth(150);

  setGradient(i18n("Horizontal")); // build the gradient
  gradPreview->setBackgroundPixmap(gradPix);

  gradLay->addWidget(gradPreview);

  //CT 11feb98 - Title double click
  titlebarDblClickBox = new QGroupBox(i18n("Mouse action"),
				       this);

  pixLay = new QGridLayout(titlebarDblClickBox,2,2,10,5);
  pixLay->addRowSpacing(0,10);
  pixLay->setColStretch(0,0);
  pixLay->setColStretch(1,1);

  lDblClick = new QLabel(i18n("Left Button double click does:"),
			 titlebarDblClickBox);
  lDblClick->adjustSize();
  lDblClick->setMinimumSize(lDblClick->size());
  pixLay->addWidget(lDblClick,1,0);

  //CT 11feb98 - Title double click
  
  // I commented some stuff out, since it does not make sense (Matthias 23okt98)
  dblClickCombo = new QComboBox(FALSE, titlebarDblClickBox);
  dblClickCombo->insertItem(i18n("(Un)Maximize"),DCTB_MAXIMIZE);
  dblClickCombo->insertItem(i18n("(Un)Shade"),DCTB_SHADE);
  dblClickCombo->insertItem(i18n("Iconify"),DCTB_ICONIFY);
  dblClickCombo->insertItem(i18n("(Un)Sticky"),DCTB_STICKY);
//   dblClickCombo->insertItem(i18n("Move"),DCTB_MOVE);
//   dblClickCombo->insertItem(i18n("Resize"),DCTB_RESIZE);
//   dblClickCombo->insertItem(i18n("Restore"),DCTB_RESTORE);
//   dblClickCombo->insertItem(i18n("Operations Menu"),
// 			    DCTB_OPERATIONS);
  dblClickCombo->insertItem(i18n("Close"),DCTB_CLOSE);
  dblClickCombo->setCurrentItem( DCTB_MAXIMIZE );

  dblClickCombo->adjustSize();
  dblClickCombo->setMinimumSize(dblClickCombo->size());
  pixLay->addWidget(dblClickCombo,1,1);

  pixLay->activate();

  lay->addMultiCellWidget(titlebarDblClickBox,2,2,0,1);

  //CT ---

  // titlebar animation
  animBox = new QGroupBox(i18n("Title animation"),
				       this);

  pixLay = new QGridLayout(animBox,2,3,10,5);
  pixLay->addRowSpacing(0,10);
  pixLay->setColStretch(0,0);
  pixLay->setColStretch(1,0);
  pixLay->setColStretch(2,1);

  t = new QLCDNumber (2, animBox);
  t->setFrameStyle( QFrame::NoFrame );
  t->setFixedHeight(30);
  t->adjustSize();
  t->setMinimumSize(t->size());
  pixLay->addWidget(t,1,0);

  sec = new QLabel(i18n("ms"), animBox);
  sec->adjustSize();
  sec->setMinimumSize(sec->size());
  pixLay->addWidget(sec,1,1);

  titleAnim = new KSlider(0,100,10,0, KSlider::Horizontal, animBox);
  titleAnim->setSteps(10,10);
  titleAnim->adjustSize();
  titleAnim->setMinimumSize(titleAnim->size());
  pixLay->addWidget(titleAnim,1,2);

  pixLay->activate();

  lay->addMultiCellWidget(animBox,3,3,0,1);

  lay->activate();

  connect( titleAnim,   SIGNAL(valueChanged(int)), t, SLOT(display(int)) );

  GetSettings();

  gradientTypes->setCurrentItem((int) gradient);
}

//CT 02Dec1998
bool KTitlebarAppearance::getFramedTitle() {
  return cbFrame->isChecked();
}

void KTitlebarAppearance::setFramedTitle(bool a) {
  cbFrame->setChecked(a);
}

bool KTitlebarAppearance::getPixedText() {
  return !cbPixedText->isChecked();
}

void KTitlebarAppearance::setPixedText(bool a) {
  cbPixedText->setChecked(!a);
}


//CT 06Nov1998
int KTitlebarAppearance::getAlign() {
  if (midAlign->isChecked()) return AT_MIDDLE;
  else if (rightAlign->isChecked()) return AT_RIGHT;
  else return AT_LEFT;
}

void KTitlebarAppearance::setAlign(int a) {
  if (a == AT_LEFT) 
    leftAlign->setChecked(TRUE);
  if (a == AT_MIDDLE)
    midAlign->setChecked(TRUE);
  if (a == AT_RIGHT)
    rightAlign->setChecked(TRUE);
}
//CT

int KTitlebarAppearance::getTitlebar()
{
  if (bShaded->isChecked()) {
    return TITLEBAR_SHADED;
  }
  else if (pixmap->isChecked())
      return TITLEBAR_PIXMAP;
  else
      return TITLEBAR_PLAIN;
}

void KTitlebarAppearance::setTitlebar(int tb)
{
  if (tb == TITLEBAR_PIXMAP)
    {
      bShaded->setChecked(FALSE);
      plain->setChecked(FALSE);
      pixmap->setChecked(TRUE);
      optOpts->raiseWidget(pixmapBox);
      pixmapBox->setEnabled(TRUE);
      lPixmapActive->setEnabled(TRUE);
      pbPixmapActive->setEnabled(TRUE);
      lPixmapInactive->setEnabled(TRUE);
      pbPixmapInactive->setEnabled(TRUE);
      cbPixedText->setEnabled(TRUE);
      return;
    }
  if (tb == TITLEBAR_SHADED)
    {
      bShaded->setChecked(TRUE);
      plain->setChecked(FALSE);
      pixmap->setChecked(FALSE);
      optOpts->raiseWidget(gradBox);
      pixmapBox->setEnabled(FALSE);
      lPixmapActive->setEnabled(FALSE);
      pbPixmapActive->setEnabled(FALSE);
      lPixmapInactive->setEnabled(FALSE);
      pbPixmapInactive->setEnabled(FALSE);
      cbPixedText->setEnabled(FALSE);
      return;
    }
  if (tb == TITLEBAR_PLAIN)
    {
      bShaded->setChecked(FALSE);
      plain->setChecked(TRUE);
      pixmap->setChecked(FALSE);
      optOpts->raiseWidget(pixmapBox);
      pixmapBox->setEnabled(FALSE);
      lPixmapActive->setEnabled(FALSE);
      pbPixmapActive->setEnabled(FALSE);
      lPixmapInactive->setEnabled(FALSE);
      pbPixmapInactive->setEnabled(FALSE);
      cbPixedText->setEnabled(FALSE);
      return;
    }
}

int KTitlebarAppearance::getTitleAnim()
{
  return t->intValue();
}
void KTitlebarAppearance::setTitleAnim(int tb)
{
  titleAnim->setValue(tb);
  t->display(tb);
}

//CT 11feb98 action on double click on titlebar
int KTitlebarAppearance::getDCTBAction()
{
  return dblClickCombo->currentItem();
}

void KTitlebarAppearance::setDCTBAction(int action)
{
  dblClickCombo->setCurrentItem(action);
}
//CT ---

void KTitlebarAppearance::SaveSettings( void )
{

  config->setGroup( "General" );

  //CT 06Nov1998
  int t = getAlign();
  if (t == AT_MIDDLE) config->writeEntry(KWM_TITLEALIGN, "middle");
  else if (t == AT_RIGHT) config->writeEntry(KWM_TITLEALIGN, "right");
  else config->writeEntry(KWM_TITLEALIGN, "left");
  //CT

  //CT 02Dec1998 - optional shaded frame on titlebar
  config->writeEntry(KWM_TITLEFRAME, getFramedTitle()?"yes":"no");

  //CT 02Dec1998 - optional pixmap under the title text
  config->writeEntry(KWM_PIXMAP_TEXT, getPixedText()?"yes":"no");
  //CT

  t = getTitlebar();
  if (t == TITLEBAR_SHADED)
    {
      if (gradient == VERT)
	config->writeEntry(KWM_TITLEBARLOOK, "shadedVertical");
      else if (gradient == HORIZ)
	config->writeEntry(KWM_TITLEBARLOOK, "shadedHorizontal");
      else if (gradient == DIAG)
	config->writeEntry(KWM_TITLEBARLOOK, "shadedDiagonal");
      else if (gradient == CROSSDIAG)
	config->writeEntry(KWM_TITLEBARLOOK, "shadedCrossDiagonal");
      else if (gradient == PYRAM)
	config->writeEntry(KWM_TITLEBARLOOK, "shadedPyramid");
      else if (gradient == RECT)
	config->writeEntry(KWM_TITLEBARLOOK, "shadedRectangle");
      else if (gradient == PIPE)
	config->writeEntry(KWM_TITLEBARLOOK, "shadedPipeCross");
      else if (gradient == ELLIP)
	config->writeEntry(KWM_TITLEBARLOOK, "shadedElliptic");
    }
  else if (t == TITLEBAR_PIXMAP)
    config->writeEntry(KWM_TITLEBARLOOK, "pixmap");
  else
    config->writeEntry(KWM_TITLEBARLOOK, "plain");

  /*CT 18Oct1998 - these are no more needed
  config->writeEntry("TitlebarPixmapActive", sPixmapActive);
  config->writeEntry("TitlebarPixmapInactive", sPixmapInactive);
  */

  //CT 18Oct1998 - save the pixmaps
  if (t == TITLEBAR_PIXMAP ) {
    QString kwmpicsdir = locateLocal("data", "kwm/");

    //first, a backup
    sPixmapActive   = "oldactivetitlebar.xpm";
    sPixmapInactive = "oldinactivetitlebar.xpm";   
 
    if (!pixmapActiveOld.isNull()) {
      QFile( sPixmapActive ).remove();
      pixmapActiveOld.save(kwmpicsdir+'/'+sPixmapActive,"XPM");
      iconLoader->flush( sPixmapActive );
    }

    if (!pixmapInactiveOld.isNull()) {
      QFile( sPixmapInactive ).remove();
      pixmapInactiveOld.save(kwmpicsdir+'/'+sPixmapInactive,"XPM");
      iconLoader->flush( sPixmapInactive );
    }

    //then, the save
    sPixmapActive   = "activetitlebar.xpm";
    sPixmapInactive = "inactivetitlebar.xpm";

    bool a_saved = true, i_saved = true;
    if (!pixmapActive.isNull()) {
      QFile( sPixmapActive ).remove();
      a_saved = pixmapActive.save(kwmpicsdir+'/'+sPixmapActive,"XPM");
      iconLoader->flush( sPixmapActive );
    }

    if (!pixmapInactive.isNull()) {
      QFile( sPixmapInactive ).remove();
      i_saved = pixmapInactive.save(kwmpicsdir+'/'+sPixmapInactive,"XPM");
      iconLoader->flush( sPixmapInactive );
    }

    //and a little check
    if ( !( a_saved && i_saved ) ) {
      QMessageBox::critical(this, i18n("Window manager setup - Error"),
			    i18n("There was an error while saving\n"
				 "the titlebar pixmaps! Please check permissions."),
			    i18n("&Ok"));
    }
  }
  //CT

  int a = getTitleAnim();
  config->writeEntry(KWM_TITLEANIMATION, a);

  //CT 11feb98 action on double click on titlebar
  a = getDCTBAction();
  switch (a) {
    //CT 23Oct1998 took out useless checks
    /*  case DCTB_MOVE:
    config->writeEntry(KWM_DCTBACTION, "winMove");
    break;
  case DCTB_RESIZE:
    config->writeEntry(KWM_DCTBACTION, "winResize");
    break;*/
  case DCTB_MAXIMIZE:
    config->writeEntry(KWM_DCTBACTION, "winMaximize");
    break;
    /*  case DCTB_RESTORE:
    config->writeEntry(KWM_DCTBACTION, "winRestore");
    break;*/
  case DCTB_ICONIFY:
    config->writeEntry(KWM_DCTBACTION, "winIconify");
    break;
  case DCTB_CLOSE:
    config->writeEntry(KWM_DCTBACTION, "winClose");
    break;
  case DCTB_STICKY:
    config->writeEntry(KWM_DCTBACTION, "winSticky");
    break;
  case DCTB_SHADE:
    config->writeEntry(KWM_DCTBACTION, "winShade");
    break;
    /*  case DCTB_OPERATIONS:
    config->writeEntry(KWM_DCTBACTION, "winOperations");
    break;*/
  //CT should never get here
  default:     config->writeEntry(KWM_DCTBACTION, "winMaximize");
  }
  //CT ---

  config->sync();

}

void KTitlebarAppearance::setGradient(const QString & grad_name)
{

  gradPix.resize(gradPreview->width(), gradPreview->height());

  if (grad_name == i18n("Vertical"))
  {
    gradient = VERT;
    KPixmapEffect::gradient(gradPix, Qt::black, Qt::blue, 
			    KPixmapEffect::VerticalGradient);
  }
  else if (grad_name == i18n("Horizontal"))
  {
    gradient = HORIZ;
    KPixmapEffect::gradient(gradPix, Qt::black, Qt::blue, 
			    KPixmapEffect::HorizontalGradient);
  }
  else if (grad_name == i18n("Diagonal"))
  {
    gradient = DIAG;
    KPixmapEffect::gradient(gradPix, Qt::black, Qt::blue, 
			    KPixmapEffect::DiagonalGradient);
  }
  else if (grad_name == i18n("CrossDiagonal"))
  {
    gradient = CROSSDIAG;
    KPixmapEffect::gradient(gradPix, Qt::black, Qt::blue, 
			    KPixmapEffect::CrossDiagonalGradient);
  }
  else if (grad_name == i18n("Pyramid"))
  {
    gradient = PYRAM;
    KPixmapEffect::gradient(gradPix, Qt::black, Qt::blue, 
			    KPixmapEffect::PyramidGradient);
  }
  else if (grad_name == i18n("Rectangle"))
  {
    gradient = RECT;
    KPixmapEffect::gradient(gradPix, Qt::black, Qt::blue, 
			    KPixmapEffect::RectangleGradient);
  }
  else if (grad_name == i18n("PipeCross"))
  {
    gradient = PIPE;
    KPixmapEffect::gradient(gradPix, Qt::black, Qt::blue, 
			    KPixmapEffect::PipeCrossGradient);
  }
  else if (grad_name == i18n("Elliptic"))
  {
    gradient = ELLIP;
    KPixmapEffect::gradient(gradPix, Qt::black, Qt::blue, 
			    KPixmapEffect::EllipticGradient);
  }

  gradPreview->setBackgroundPixmap(gradPix);
}

void KTitlebarAppearance::GetSettings( void )
{
  QString key;

  config->setGroup( "General" );

  //CT 06Nov1998
  key = config->readEntry(KWM_TITLEALIGN);
  if( key == "middle" ) setAlign(AT_MIDDLE);
  else if ( key == "right" ) setAlign(AT_RIGHT);
  else setAlign(AT_LEFT);
  //CT

  //CT 02Dec1998 - optional shaded frame on titlebar
  key = config->readEntry(KWM_TITLEFRAME);
  if (key == "no") setFramedTitle(false);
  else setFramedTitle(true);
  //CT

  //CT 02Dec1998 - optional pixmap under the title text
  key = config->readEntry(KWM_PIXMAP_TEXT);
  if (key == "no") setPixedText(false);
  else setPixedText (true);
  //CT

  key = config->readEntry(KWM_TITLEBARLOOK);
  if( key.find("shaded") != -1)
    {
      setTitlebar(TITLEBAR_SHADED);
      if (key== "shadedVertical")
	setGradient(i18n("Vertical"));
      else if (key== "shadedHorizontal")
	setGradient(i18n("Horizontal"));
      else if (key== "shadedDiagonal")
	setGradient(i18n("Diagonal"));
      else if (key== "shadedCrossDiagonal")
	setGradient(i18n("CrossDiagonal"));
      else if (key== "shadedPyramid")
	setGradient(i18n("Pyramid"));
      else if (key== "shadedRectangle")
	setGradient(i18n("Rectangle"));
      else if (key== "shadedPipeCross")
	setGradient(i18n("PipeCross"));
      else if (key== "shadedElliptic")
	setGradient(i18n("Elliptic"));
    }
  else if( key == "pixmap")
    setTitlebar(TITLEBAR_PIXMAP);
  else
    setTitlebar(TITLEBAR_PLAIN);

  sPixmapActive = "activetitlebar.xpm";
  sPixmapInactive = "inactivetitlebar.xpm";
  pbPixmapActive->setPixmap(pixmapActiveOld =
			      Icon( sPixmapActive ));
  pbPixmapInactive->setPixmap(pixmapInactiveOld =
			      Icon( sPixmapInactive ));


  int k = config->readNumEntry(KWM_TITLEANIMATION,0);
  setTitleAnim(k);

  key = config->readEntry(KWM_DCTBACTION);
  //CT 23Oct1998 continue what Matthias started 
  //   took out useless checks
  //  if (key == "winMove") setDCTBAction(DCTB_MOVE);
  //  else if (key == "winResize") setDCTBAction(DCTB_RESIZE);
  /*else*/ if (key == "winMaximize") setDCTBAction(DCTB_MAXIMIZE);
  //  else if (key == "winRestore") setDCTBAction(DCTB_RESTORE);
  else if (key == "winIconify") setDCTBAction(DCTB_ICONIFY);
  else if (key == "winClose") setDCTBAction(DCTB_CLOSE);
  else if (key == "winSticky") setDCTBAction(DCTB_STICKY);
  else if (key == "winShade") setDCTBAction(DCTB_SHADE);
  //  else if (key == "winOperations") setDCTBAction(DCTB_OPERATIONS);
  else setDCTBAction(DCTB_MAXIMIZE);

}

void KTitlebarAppearance::loadSettings()
{
  GetSettings();
}

void KTitlebarAppearance::applySettings()
{
  SaveSettings();
}


void KTitlebarAppearance::titlebarChanged()
{
  setTitlebar(getTitlebar());
}


void KTitlebarAppearance::activePressed()
{
  KIconLoaderDialog dlg(iconLoader, this);
  QString name ;//CT= sPixmapActive;
  //CT  QPixmap map;

  pixmapActive = dlg.selectIcon(name, "*");
  if (!name.isEmpty())
    {
      //CT      sPixmapActive = name;
      pbPixmapActive->setPixmap(pixmapActive);
    }
}


void KTitlebarAppearance::inactivePressed()
{
  KIconLoaderDialog dlg(iconLoader, this);
  QString name ;//CT= sPixmapInactive;
  //CT  QPixmap map;

  pixmapInactive = dlg.selectIcon(name, "*");
  if (!name.isEmpty())
    {
      //CT      sPixmapInactive = name;
      pbPixmapInactive->setPixmap(pixmapInactive);
    }
}


#include "titlebar.moc"


