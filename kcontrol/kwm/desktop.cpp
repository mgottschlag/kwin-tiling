/*
 * $Id$
 *
 * Copyright (c) 1997 Bernd Johannes Wuebben wuebben@math.cornell.edu
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

#include <qlayout.h>
#include <qbuttongroup.h>
#include <qcheckbox.h>
#include <qslider.h>

#include <kapp.h>
#include <kglobal.h>
#include <klocale.h>
#include <kconfig.h>
#include <knuminput.h>

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "desktop.h"
#include "geom.h"


// kwm config keywords
#define KWM_ELECTRIC_BORDER                  "ElectricBorder"
#define KWM_ELECTRIC_BORDER_DELAY            "ElectricBorderNumberOfPushes"
#define KWM_ELECTRIC_BORDER_MOVE_POINTER     "ElectricBorderPointerWarp"

//CT 15mar 98 - magics
#define KWM_BRDR_SNAP_ZONE                   "BorderSnapZone"
#define KWM_BRDR_SNAP_ZONE_DEFAULT           10
#define KWM_WNDW_SNAP_ZONE                   "WindowSnapZone"
#define KWM_WNDW_SNAP_ZONE_DEFAULT           10

#define MAX_BRDR_SNAP                          50
#define MAX_WNDW_SNAP                          50
#define MAX_EDGE_RES                         1000


//CT 21Oct1998 - emptied
KDesktopConfig::~KDesktopConfig ()
{
}

extern "C" {
    KCModule *create_kwindesktop ( QWidget *parent, const char *name )
    {
      KGlobal::locale()->insertCatalogue("kcmkwm");
      return new KDesktopConfig( parent, name );
    }
} 

//CT 21Oct1998 - rewritten for using layouts
KDesktopConfig::KDesktopConfig (QWidget * parent, const char *name)
  : KCModule (parent, name)
{

  QBoxLayout *lay = new QVBoxLayout(this, 5);

  ElectricBox = new QButtonGroup(i18n("Active desktop borders"),
				 this);

  QGridLayout *eLay = new QGridLayout(ElectricBox,5,3,10,5);
  eLay->addRowSpacing(0,10);
  eLay->setColStretch(0,0);
  eLay->setColStretch(1,1);

  enable= new
    QCheckBox(i18n("Enable active desktop borders"),
	      ElectricBox);
  eLay->addMultiCellWidget(enable,1,1,0,1);

  movepointer = new QCheckBox(i18n("Move pointer towards center after switch"),
                              ElectricBox);
  eLay->addMultiCellWidget(movepointer,2,2,0,1);

  delays = new KIntNumInput(i18n("Desktop switch delay:"), 0,MAX_EDGE_RES/10,10,0,
                            QString::null, 10, true, ElectricBox);
  eLay->addMultiCellWidget(delays,4,4,1,2);

  connect( enable, SIGNAL(clicked()), this, SLOT(setEBorders()));

  eLay->activate();

  lay->addWidget(ElectricBox,5);

  // Electric borders are not in kwin yet => disable controls
  enable->setEnabled(false);
  movepointer->setEnabled(false);
  delays->setEnabled(false);


  //CT 15mar98 - add EdgeResistance, BorderAttractor, WindowsAttractor config
  MagicBox = new QButtonGroup(i18n("Magic Borders"), this);

  eLay = new QGridLayout(MagicBox,4,3,10,5);
  eLay->addRowSpacing(0,10);
  eLay->addRowSpacing(2,10);
  eLay->setColStretch(0,0);
  eLay->setColStretch(1,0);
  eLay->setColStretch(2,1);

  BrdrSnap = new KIntNumInput(i18n("Border Snap Zone:"), 0, MAX_BRDR_SNAP, 1, 0,
                                    i18n("pixels"), 10, true, MagicBox);
  BrdrSnap->setSteps(1,1);
  eLay->addWidget(BrdrSnap,1,2);
  eLay->addRowSpacing(0,5);

  WndwSnap = new KIntNumInput(i18n("Window Snap Zone:"), 0, MAX_WNDW_SNAP, 1, 0,
                              i18n("pixels"), 10, true, MagicBox);
  WndwSnap->setSteps(1,1);
  eLay->addWidget(WndwSnap,3,2);
  lay->addWidget(MagicBox,5);

  load();

  connect( BrdrSnap, SIGNAL(valueChanged(int)), this, SLOT(slotBrdrChanged(int)));
  connect( WndwSnap, SIGNAL(valueChanged(int)), this, SLOT(slotWndwChanged(int)));
}

void KDesktopConfig::slotBrdrChanged(int /* value */)
{
  emit changed(true);
}

void KDesktopConfig::slotWndwChanged(int /* value */)
{
  emit changed(true);
}

void KDesktopConfig::setEBorders()
{
    delays->setEnabled(enable->isChecked());
    movepointer->setEnabled(enable->isChecked());
}

bool KDesktopConfig::getElectricBorders()
{
    return  enable->isChecked();
}

int KDesktopConfig::getElectricBordersDelay()
{
    return delays->value();
}

bool KDesktopConfig::getElectricBordersMovePointer()
{
    return movepointer->isChecked();
}

void KDesktopConfig::setElectricBordersMovePointer(bool move){

  if(move){
    movepointer->setEnabled(true);
    movepointer->setChecked(true);
  }
  else{
    movepointer->setEnabled(false);
    movepointer->setChecked(false);
  }

  movepointer->setEnabled(enable->isChecked());

}

void KDesktopConfig::setElectricBorders(bool b){
    enable->setChecked(b);
    setEBorders();
}

void KDesktopConfig::setElectricBordersDelay(int delay)
{
    delays->setValue(delay);
}


int KDesktopConfig::getBorderSnapZone() {
  return BrdrSnap->value();
}

void KDesktopConfig::setBorderSnapZone(int pxls) {
  BrdrSnap->setValue(pxls);
}

int KDesktopConfig::getWindowSnapZone() {
  return WndwSnap->value();
}

void KDesktopConfig::setWindowSnapZone(int pxls) {
  WndwSnap->setValue(pxls);
}

void KDesktopConfig::load( void )
{
  int v;
  QString key;

  KConfig *config = new KConfig("kwinrc");
  config->setGroup( "Windows" );

/* Electric borders are not in kwin yet (?)
  v = config->readNumEntry(KWM_ELECTRIC_BORDER);
  setElectricBorders(v != -1);

  v = config->readNumEntry(KWM_ELECTRIC_BORDER_DELAY);
  setElectricBordersDelay(v);

  //CT 17mar98 re-allign this reading with the one in kwm  ("on"/"off")
  // matthias: this is obsolete now. Should be fixed in 1.1 with NoWarp, MiddleWarp, FullWarp
  key = config->readEntry(KWM_ELECTRIC_BORDER_MOVE_POINTER);
  if (key == "MiddleWarp")
    setElectricBordersMovePointer(TRUE);
*/
  //CT 15mar98 - magics
  v = config->readNumEntry(KWM_BRDR_SNAP_ZONE, KWM_BRDR_SNAP_ZONE_DEFAULT);
  if (v > MAX_BRDR_SNAP) setBorderSnapZone(MAX_BRDR_SNAP);
  else if (v < 0) setBorderSnapZone (0);
  else setBorderSnapZone(v);

  v = config->readNumEntry(KWM_WNDW_SNAP_ZONE, KWM_WNDW_SNAP_ZONE_DEFAULT);
  if (v > MAX_WNDW_SNAP) setWindowSnapZone(MAX_WNDW_SNAP);
  else if (v < 0) setWindowSnapZone (0);
  else setWindowSnapZone(v);
  //CT ---
  
  emit changed(false);
  delete config;
}

void KDesktopConfig::save( void )
{
  int v;
//  bool bv;

  KConfig *config = new KConfig("kwinrc", false, false);
  config->setGroup( "Windows" );

/* Electric borders are not in kwin yet
  v = getElectricBordersDelay()>10?80*getElectricBordersDelay():800;
  if (getElectricBorders())
    config->writeEntry(KWM_ELECTRIC_BORDER,v);
  else
    config->writeEntry(KWM_ELECTRIC_BORDER,-1);


  config->writeEntry(KWM_ELECTRIC_BORDER_DELAY,getElectricBordersDelay());

  bv = getElectricBordersMovePointer();
  config->writeEntry(KWM_ELECTRIC_BORDER_MOVE_POINTER,bv?"MiddleWarp":"NoWarp");
*/

  //CT 15mar98 - magics
  v = getBorderSnapZone();
  config->writeEntry(KWM_BRDR_SNAP_ZONE,v);

  v = getWindowSnapZone();
  config->writeEntry(KWM_WNDW_SNAP_ZONE,v);

  config->sync();
  delete config;
}

void KDesktopConfig::defaults( void )
{
  setWindowSnapZone(KWM_WNDW_SNAP_ZONE_DEFAULT);
  setBorderSnapZone(KWM_BRDR_SNAP_ZONE_DEFAULT);
}

#include "desktop.moc"
