/* This file is part of the KDE Display Manager Configuration package
    Copyright (C) 1997-1998 Thomas Tanghus (tanghus@earthling.net)

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


#include <qmessagebox.h>

#include "utils.h"
#include <kio_job.h>
#include <klocale.h>
#include <kglobal.h>
#include <klined.h>
#include <kstddirs.h>

#include "kdm-appear.moc"


// Destructor
KDMAppearanceWidget::~KDMAppearanceWidget()
{
  if(gui)
  {
    delete logobutton;
    delete logopixdrop;
    delete logo_lined;
    delete greetstr_lined;
    delete guicombo;
    delete langcombo;
  }
}

/*
void KDMConfigWidget::resizeEvent(QResizeEvent *re)
{
  QSize s = re->size();

  if(w && tabbar)
  {
    tabbar->adjustSize();
    w->setGeometry(0, tabbar->height(),
            s.width(), s.height()-tabbar->height());
  }
  else
    debug("Resize: NO WIDGET!!!");
  KConfigWidget::resizeEvent(re);
}
*/

KDMAppearanceWidget::KDMAppearanceWidget(QWidget *parent, const char *name, bool init)
  : KConfigWidget(parent, name)
{
      gui = !init;
      loadSettings();
      if(gui)
        setupPage(parent);
}

void KDMAppearanceWidget::setupPage(QWidget *pw)
{
      QGroupBox *group = new QGroupBox( 
            i18n("Appearance"), this );
      CHECK_PTR(group);
      group->setGeometry(5, 10, pw->width()-30, pw->height()-20);
      QLabel *label = new QLabel(i18n("Greeting string:"), group);
      label->move( 10, 20 );

      greetstr_lined = new KLineEdit(group);
      greetstr_lined->setText(greetstr);
      greetstr_lined->setGeometry(label->width()+10, 20,
                       pw->width()-(label->width()+50), label->height());

      label = new QLabel(i18n("KDM logo:"), group);
      label->move(10, greetstr_lined->height()+30);
      logo_lined = new KLineEdit( group);
      logo_lined->setText(logopath);
      logo_lined->setGeometry(greetstr_lined->x(), greetstr_lined->height()+30,
                       greetstr_lined->width(), greetstr_lined->height());
      connect(logo_lined, SIGNAL(returnPressed()),
              SLOT(slotLogoPixTextChanged()));

      logobutton = new KIconLoaderButton(iconloader, group);
      logobutton->setMaximumSize(80, 80);
      QPixmap p;
      if(!p.load(logopath))
      {
        logobutton->setIcon("kdelogo.xpm");
        //debug("Error loading %s", logopath.ascii());
      }
      else
      {
        logo_lined->setText(logopath);
        logobutton->setPixmap(p);
      }
      logobutton->move(logo_lined->x(), logo_lined->y()+logo_lined->height()+10);
      logobutton->adjustSize();
      connect(logobutton, SIGNAL(iconChanged(const QString&)),
              SLOT(slotLogoPixChanged(const QString&)));

      QToolTip::add(logobutton, i18n("Click or drop an image here"));
      logopixdrop = new KDNDDropZone(logobutton, DndURL);
      connect(logopixdrop, SIGNAL(dropAction(KDNDDropZone*)),
              SLOT(slotPixDropped(KDNDDropZone*)));

      label = new QLabel(i18n("GUI Style:"), group);
      label->move(10, logobutton->y()+100);

      guicombo = new QComboBox( FALSE, group );
      connect(guicombo, SIGNAL(highlighted(int)), SLOT(slotSetGUI(int)));
      guicombo->move(logobutton->x(), label->y());
      guicombo->insertItem(i18n("Motif"), 0);
      guicombo->insertItem(i18n("Windows"), 1);
      guicombo->adjustSize();
      if(guistr == "Windows")
        guicombo->setCurrentItem(1);
      else
        guicombo->setCurrentItem(0);

      group->adjustSize();
      group->setMinimumSize(group->size());

      QGroupBox *group2 = new QGroupBox( 
            i18n("Language"), this );
      CHECK_PTR(group2);
      group2->setGeometry(5, group->height()+5, group->width(), 50);
      label = new QLabel(i18n("Language:"), group2);
      label->move( 10, 20 );
      langcombo = new KLanguageCombo(group2);
      langcombo->adjustSize();
      langcombo->move(logo_lined->x(), 20);

      KConfig simple("kdmrc");
      simple.setGroup("Locale");
      QString lang = simple.readEntry("Language", "C");
      int index = lang.find(':');
      if (index>0)
        lang = lang.left(index);
      langcombo->setLanguage(lang);

      group2->adjustSize();
      group2->setMinimumSize(group2->size());
     
      QBoxLayout *main = new QVBoxLayout(this, 10);
      main->addWidget(group);
      main->addWidget(group2);
      main->activate();
}



void KDMAppearanceWidget::slotLogoPixTextChanged()
{
  QString msg, pix = logo_lined->text();
  QPixmap p(pix);
  if(!p.isNull())
  {
    logobutton->setPixmap(p);
    logobutton->adjustSize();
  }
  else
  {
    msg  = i18n("There was an error loading the image:\n>");
    msg += pix;
    msg += i18n("<");
    QMessageBox::warning(this, i18n("KDM Setup - Error"), msg,
			 i18n("&Ok"));
  }
}

void KDMAppearanceWidget::slotLogoPixChanged(const QString &iconstr)
{
  // Because KIconLoaderButton only returns a relative filename
  // we gotta save the image in PIXDIR.
  // To make it easy we save it as an XPM
  QString msg;
  QString pix = /*PIXDIR + */ iconstr.left(iconstr.findRev('.')) + ".xpm";
  const QPixmap *p = logobutton->pixmap();
  if(!p)
    return;
  if(!p->save(pix, "XPM"))
  {
    msg  = i18n("There was an error saving the image:\n>");
    msg += pix;
    msg += i18n("<");
    QMessageBox::warning(this, i18n("KDM Setup - Error"), msg, i18n("&Ok"));
  }
  else
    logo_lined->setText(pix);
  logobutton->adjustSize();
}

void KDMAppearanceWidget::slotPixDropped(KDNDDropZone *zone)
{
  // Find the widget on which the object was dropped
  QWidget *w = zone->getWidget();
  // Get the url
  KURL url(zone->getData());
  QString filename = url.filename();
  QString msg;
  QString pixurl("file:"+kapp->kde_datadir() + "/kdm/pics/"); 
  int last_dot_idx = filename.findRev('.');
  bool istmp = false;

  // CC: Now check for the extension
  QString ext(".xpm .xbm");
//#ifdef HAVE_LIBGIF
  ext += " .gif";
//#endif
#ifdef HAVE_LIBJPEG
  ext += " .jpg";
#endif

  if( !ext.contains(filename.right(filename.length()-last_dot_idx), false) )
  {
    msg =  i18n("Sorry, but %1\n"
                "does not seem to be an image file\n"
                "Please use files with these extensions:\n"
                "%2")
                .arg(filename)
                .arg(ext);
    QMessageBox::warning( this, i18n("KDM Setup - Improper File Extension"), msg,
			  i18n("&Ok"));
  }
  else
  {
    // we gotta check if it is a non-local file and make a tmp copy at the hd.
    if(url.protocol() != "file")
    {
      pixurl += url.filename();
      KIOJob *iojob = new KIOJob(); // will autodelete itself
      iojob->setGUImode( KIOJob::NONE );
      iojob->copy(url.url().ascii(), pixurl.ascii());
      url = pixurl;
      istmp = true;
    }
    // By now url should be "file:/..."
    if(w == logobutton) // Image dropped on logo button
    {
      QPixmap p(url.path());
      if(!p.isNull())
      {
        logobutton->setPixmap(p);
        logobutton->adjustSize();
        logopath = url.path();
        logo_lined->setText(logopath);
      }
      else
      {
        msg  = i18n("There was an error loading the image:\n>");
        msg += url.path();
        msg += i18n("<\nIt will not be saved...");
        QMessageBox::warning(this, i18n("KDM Setup - Error"), msg,
			     i18n("&Ok"));
      }
    }
  }
}

void KDMAppearanceWidget::slotSetGUI( int g )
{
  if(g == 0)
    guistr = "Motif";
  else
    guistr = "Windows";
}

void KDMAppearanceWidget::applySettings()
{
  //debug("KDMAppearanceWidget::applySettings()");
  KSimpleConfig *c = new KSimpleConfig(locate("config", "kdmrc"));

  c->setGroup("KDM");

  // write greeting string
  c->writeEntry("GreetString", greetstr_lined->text(), true);

  // write logo path
  if(!logo_lined->text().isEmpty())
    logopath = logo_lined->text();
  QFileInfo fi(logopath);
  if(fi.exists())
    c->writeEntry("LogoPixmap", logopath, true);
  else
    c->deleteEntry("LogoPixmap", false);

  // write GUI style
  c->writeEntry("GUIStyle", guistr, true);

  // write language
  c->setGroup("Locale");
  c->writeEntry("Language", langcombo->getLanguage());

  delete c;
}

void KDMAppearanceWidget::loadSettings()
{
    iconloader = KGlobal::iconLoader();
    QString str;
  
    // Get config object
    KConfig *c = new KConfig("kdmrc");
    c->setGroup("KDM");

  // Read the greeting string
  greetstr = "KDE System at HOSTNAME";
  greetstr = c->readEntry("GreetString", greetstr);

  // See if we use alternate logo
  logopath = c->readEntry("LogoPixmap");
  if(!logopath.isEmpty())
  {
    QFileInfo fi(logopath);
    if(fi.exists())
    {
      //logofile = fi.fileName();
      QString lpath = fi.dirPath(true);
      KGlobal::dirs()->addResourceDir("icon", lpath);
    }
  }

  // Check the GUI type
  guistr = c->readEntry("GUIStyle", "Motif");

  delete c;
}


