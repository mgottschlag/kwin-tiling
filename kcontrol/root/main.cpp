/*
  main.cpp - root cursor control applet

  written 1999 by Joel Dillon
  
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
  
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  
  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
   
  */


#include "main.h"
#include "main.moc"
#include "kconfig.h"
#include <qbitmap.h>
#include <qpaintdevice.h>
#include <qcursor.h>
#include <qbuttongroup.h>
#include <kfiledialog.h>

static bool dogui;

KCursorConfig::KCursorConfig(QWidget * parent,const char * name,bool init)
  : KConfigWidget(parent,name)
{
  qbg=0;
  if(init)
    dogui=false;
  else
    dogui=true;
  if(dogui) {
    browse=new QPushButton("Browse",this);
    browse->show();
  }
  config=kapp->getConfig();
  GetSettings();
  if(init)
    saveParams();
}

bool squangle=false;
bool squingle=false;

void KCursorConfig::wastoggled(int n)
{
  mysel=n;
}

void KCursorConfig::initCheck()
{
  if(!squingle) {
    cursors[0]=&Qt::arrowCursor;
    cnames[0]=new QRadioButton("Arrow",this);
    cursors[1]=&Qt::upArrowCursor;
    cnames[1]=new QRadioButton("Up arrow",this);
    cursors[2]=&Qt::crossCursor;
    cnames[2]=new QRadioButton("Cross",this);
    cursors[3]=&Qt::waitCursor;
    cnames[3]=new QRadioButton("Wait",this);
    cursors[4]=&Qt::ibeamCursor;
    cnames[4]=new QRadioButton("Ibeam",this);
    cursors[5]=&Qt::sizeVerCursor;
    cnames[5]=new QRadioButton("Vertical size",this);
    cursors[6]=&Qt::sizeHorCursor;
    cnames[6]=new QRadioButton("Horizontal size",this);
    cursors[7]=&Qt::sizeBDiagCursor;
    cnames[7]=new QRadioButton("Diagonal 1",this);
    cursors[8]=&Qt::sizeFDiagCursor;
    cnames[8]=new QRadioButton("Diagonal 2",this);
    cursors[9]=&Qt::sizeAllCursor;
    cnames[9]=new QRadioButton("All size",this);
    cursors[10]=&Qt::splitVCursor;
    cnames[10]=new QRadioButton("Split V",this);
    cursors[11]=&Qt::splitHCursor;
    cnames[11]=new QRadioButton("Split H",this);
    cursors[12]=&Qt::pointingHandCursor;
    cnames[12]=new QRadioButton("Pointing hand",this);
    cursors[13]=new QCursor();      // Default cursor if no custom
    cnames[13]=new QRadioButton("Custom",this);
    squingle=true;
  }
}

void KCursorConfig::resizeEvent(QResizeEvent * qre)
{
  int loopc;
  if(!squangle) {
    initCheck();
    qbg=new QButtonGroup();
    connect(qbg,SIGNAL(clicked(int)),this,SLOT(wastoggled(int)));
    connect(browse,SIGNAL(clicked()),this,SLOT(browseSelected()));
    for(loopc=0;loopc<numcursors;loopc++) {
      qbg->insert(cnames[loopc],loopc);
      if(loopc==mysel) {
        cnames[loopc]->toggle();
      }
      cnames[loopc]->show();
      cnames[loopc]->setCursor(*cursors[loopc]);
    }
    squangle=true;
  }

  int ww=qre->size().width();
  int hh=(qre->size().height())-40;
  int xx=20;
  int yy=20;
  int midpos=ww/2;
  int sizeit=(hh-20)/((numcursors+1)/2);
  for(loopc=0;loopc<numcursors;loopc+=2) {
    cnames[loopc]->setGeometry(xx,yy+(loopc/2*sizeit),midpos-40,sizeit-5);
    if(loopc+1<numcursors) {
      cnames[loopc+1]->setGeometry(xx+(midpos+20),yy+(loopc/2*sizeit),
                                   ww-40,sizeit-5); 
    }
  }
  browse->move(20,qre->size().height()-40);
}

KCursorConfig::~KCursorConfig()
{
  if(qbg)
    delete qbg;
}

void KCursorConfig::setCustom(QString s)
{
  printf("Loading %s\n",s.ascii());
  custom=s;
  QPixmap wuggle(s.ascii());
  if(!wuggle.isNull()) {
     // Colour cursors appear to be impossible, grr
     QBitmap a;
     QBitmap b;
     a=wuggle;
     if(wuggle.mask()) {
       b=*wuggle.mask();
     } else {
       b=wuggle.createHeuristicMask();
     }
     delete cursors[13];
     cursors[13]=new QCursor(a,b);
     cnames[13]->setCursor(*cursors[13]);
  }
}

void KCursorConfig::GetSettings()
{
  initCheck();
  config->setGroup("Root cursor");
  mysel=config->readNumEntry("Cursor number",-1);
  config->setGroup("Custom");
  QString wibble;
  wibble=config->readEntry("Bitmap","");
  custom=wibble;
  if(wibble!="") {
    setCustom(wibble);
  }
  if(mysel==-1) {
    mysel=5;
  }
  if(qbg)
    qbg->setButton(mysel);
  QApplication::desktop()->setCursor(*cursors[mysel]);
}

void KCursorConfig::browseSelected()
{
  QString ret;
  ret=KFileDialog::getOpenFileName();
  if(ret) {
    setCustom(ret);
  }  
}

void KCursorConfig::saveParams()
{
  config->setGroup("Root cursor");
  config->writeEntry("Cursor number",mysel);
  config->setGroup("Custom");
  config->writeEntry("Bitmap",custom);
  config->sync();
  QApplication::desktop()->setCursor(*cursors[mysel]);
}

void KCursorConfig::loadSettings()
{
  GetSettings();
}

void KCursorConfig::applySettings()
{
  saveParams();
}

KCursorApplication::KCursorApplication(int &argc, char **argv, const char 
                                       *name)
  : KControlApplication(argc, argv, name)
{
  CursorCfg=0;
  if (runGUI()) {
    if(!pages || pages->contains("Cursor"))
      addPage(CursorCfg=new KCursorConfig(dialog,"Cursor",FALSE),
               "&Cursor","Cursor-1.html");
    if(CursorCfg) {
      dialog->show();
    } else {
      justInit=TRUE;
    }
  }
}

KCursorApplication::~KCursorApplication()
{
}

void KCursorApplication::init()
{
  KCursorConfig * cursconfig=new KCursorConfig(0,0,TRUE);
  delete cursconfig;
}

void KCursorApplication::apply()
{
  if(CursorCfg)
   CursorCfg->applySettings();
}

int main(int argc,char **argv)
{
  KCursorApplication app(argc, argv, "kcmsample");
  app.setTitle("Cursor");
  
  if (app.runGUI())
    return app.exec();
  else
    {
      app.init();
      return 0;
    }
}

